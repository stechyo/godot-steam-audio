#include "stream.hpp"
#include "config.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/packed_vector2_array.hpp"
#include "server.hpp"
#include "steam_audio.hpp"
#include <phonon.h>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/property_info.hpp>

SteamAudioStream::SteamAudioStream() {}
SteamAudioStream::~SteamAudioStream() {}

void SteamAudioStream::_bind_methods() {}

Ref<AudioStreamPlayback> SteamAudioStream::_instantiate_playback() const {
	Ref<SteamAudioStreamPlayback> playback;
	playback.instantiate();
	playback->set_stream(stream);
	playback->parent = parent;

	return playback;
}

void SteamAudioStream::set_stream(Ref<AudioStream> p_stream) { stream = p_stream; }
Ref<AudioStream> SteamAudioStream::get_stream() { return this->stream; }

// ----------------------------------------------------
// SteamAudioStreamPlayback

SteamAudioStreamPlayback::SteamAudioStreamPlayback() {}
SteamAudioStreamPlayback::~SteamAudioStreamPlayback() {}

int32_t SteamAudioStreamPlayback::_mix(AudioFrame *buffer, float rate_scale, int32_t frames) {
	if (parent == nullptr) {
		return frames;
	}

	if (stream_playback.is_null()) {
		return frames;
	}

	if (Engine::get_singleton()->is_editor_hint()) {
		return frames;
	}

	auto gs = SteamAudioServer::get_singleton()->get_global_state(false);
	if (gs == nullptr) {
		return frames;
	}

	SteamAudio::log(SteamAudio::log_debug, "mixing");

	LocalSteamAudioState *ls = parent->get_local_state();
	if (ls == nullptr) { // probably being destroyed
		return frames;
	}
	std::unique_lock lock(ls->mux);

	// Some extra checks because at this point parent may have been deleted
	if (parent == nullptr) {
		return frames;
	}
	ls = parent->get_local_state();
	if (ls == nullptr || !ls->src.player) {
		return frames;
	}

	PackedVector2Array mixed_frames = stream_playback->mix_audio(rate_scale, frames);
	frames = int(mixed_frames.size());

	for (int i = 0; i < frames; i++) {
		ls->bufs.in.data[0][i] = mixed_frames[i].x;
		ls->bufs.in.data[1][i] = mixed_frames[i].y;
	}

	if (ls->cfg.is_dist_attn_on) {
		ls->direct_outputs.flags = static_cast<IPLDirectEffectFlags>(
				ls->direct_outputs.flags |
				IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION);
	}
	if (ls->cfg.is_occlusion_on) {
		ls->direct_outputs.flags = static_cast<IPLDirectEffectFlags>(
				ls->direct_outputs.flags |
				IPL_DIRECTEFFECTFLAGS_APPLYOCCLUSION |
				IPL_DIRECTEFFECTFLAGS_APPLYTRANSMISSION);
	}

	if (ls->direct_outputs.flags != 0) {
		iplDirectEffectApply(
				ls->fx.direct, &ls->direct_outputs,
				&ls->bufs.in, &ls->bufs.direct);
	} else {
		for (int i = 0; i < ls->bufs.direct.numChannels; i++) {
			for (int j = 0; j < ls->bufs.direct.numSamples; j++) {
				ls->bufs.direct.data[i][j] = 0.0f;
			}
		}

		iplAudioBufferMix(gs->ctx, &ls->bufs.in, &ls->bufs.direct);
	}

	IPLAmbisonicsDecodeEffectParams dec_params{};
	dec_params.orientation = gs->listener_coords;
	dec_params.order = ls->cfg.ambisonics_order;
	dec_params.hrtf = gs->hrtf;
	dec_params.binaural = IPL_TRUE;

	if (ls->cfg.is_ambisonics_on) {
		IPLAmbisonicsEncodeEffectParams enc_params{};
		enc_params.direction = ipl_vec3_from(ls->dir_to_listener);
		enc_params.order = ls->cfg.ambisonics_order;
		iplAmbisonicsEncodeEffectApply(
				ls->fx.enc, &enc_params,
				&ls->bufs.direct, &ls->bufs.ambi);

		iplAmbisonicsDecodeEffectApply(
				ls->fx.dec, &dec_params,
				&ls->bufs.ambi, &ls->bufs.out);
		SteamAudio::log(SteamAudio::log_debug, "mixing: finished ambisonics");
	} else {
		iplAudioBufferMix(gs->ctx, &ls->bufs.direct, &ls->bufs.out);
	}

	gs->refl_ir_lock.lock();
	if (ls->refl_outputs.ir != nullptr && ls->cfg.is_reflection_on) {
		iplAudioBufferDownmix(gs->ctx, &ls->bufs.in, &ls->bufs.mono);
		ls->refl_outputs.numChannels = ambisonic_channels_from(ls->cfg.ambisonics_order);
		ls->refl_outputs.type = IPL_REFLECTIONEFFECTTYPE_CONVOLUTION;
		ls->refl_outputs.irSize = int(SteamAudioConfig::max_refl_duration * float(gs->audio_cfg.samplingRate));
		iplReflectionEffectApply(ls->fx.refl, &ls->refl_outputs, &ls->bufs.mono, &ls->bufs.refl_ambi, nullptr);

		iplAmbisonicsDecodeEffectApply(
				ls->fx.refl_dec, &dec_params,
				&ls->bufs.refl_ambi, &ls->bufs.refl_out);

		SteamAudio::log(SteamAudio::log_debug, "mixing: mixing reflection and direct buffers");
		iplAudioBufferMix(gs->ctx, &ls->bufs.refl_out, &ls->bufs.out);
	}
	gs->refl_ir_lock.unlock();

	for (int i = 0; i < frames; i++) {
		buffer[i].left = ls->bufs.out.data[0][i];
		buffer[i].right = ls->bufs.out.data[1][i];
	}

	SteamAudio::log(SteamAudio::log_debug, "mixing: done");
	return frames;
}

void SteamAudioStreamPlayback::_bind_methods() {
	ClassDB::bind_method(D_METHOD("play_stream", "stream", "from_offset", "volume_db", "pitch_scale"), &SteamAudioStreamPlayback::play_stream, DEFVAL(0), DEFVAL(0), DEFVAL(1.0));
}

int SteamAudioStreamPlayback::play_stream(const Ref<AudioStream> &p_stream, float p_from_offset, float p_volume_db, float p_pitch_scale) {
	if (Engine::get_singleton()->is_editor_hint()) {
		return 0;
	}

	stream = p_stream;
	stream_playback = stream->instantiate_playback();
	stream_playback->start(p_from_offset);

	return 0;
}

void SteamAudioStreamPlayback::_start(double from_pos) {
	if (stream_playback == nullptr) {
		if (stream != nullptr) {
			is_active.store(true);
			play_stream(stream, float(from_pos), 0.0, 1.0); // FIXME: do not assume these params
		}
		return;
	} else if (stream_playback->is_playing()) {
		return;
	}
	stream_playback->start(from_pos);
	is_active.store(true);
}

void SteamAudioStreamPlayback::_stop() {
	is_active.store(false);
	if (stream_playback == nullptr || !stream_playback->is_playing()) {
		return;
	}
	stream_playback->stop();
}

bool SteamAudioStreamPlayback::_is_playing() const { return is_active; }
void SteamAudioStreamPlayback::set_stream(Ref<AudioStream> p_stream) { stream = p_stream; }
Ref<AudioStreamPlayback> SteamAudioStreamPlayback::get_stream_playback() { return this->stream_playback; }
