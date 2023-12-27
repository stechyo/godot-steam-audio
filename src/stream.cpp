#include "stream.hpp"
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

	return playback;
}

// ----------------------------------------------------

SteamAudioStreamPlayback::SteamAudioStreamPlayback() {
	is_local_state_init.store(false);
}

SteamAudioStreamPlayback::~SteamAudioStreamPlayback() {
	if (!is_local_state_init.load()) {
		return;
	}
	SteamAudio::log(SteamAudio::log_debug, "destroying stream playback and local state");

	SteamAudioServer::get_singleton()->remove_local_state(&local_state);
	auto gs = SteamAudioServer::get_singleton()->get_global_state();

	iplSourceRemove(local_state.src.src, gs->sim);
	iplSourceRelease(&local_state.src.src);
	iplDirectEffectRelease(&local_state.fx.direct);

	iplAudioBufferFree(gs->ctx, &local_state.bufs.in);
	iplAudioBufferFree(gs->ctx, &local_state.bufs.direct);
	iplAudioBufferFree(gs->ctx, &local_state.bufs.ambi);
	iplAudioBufferFree(gs->ctx, &local_state.bufs.out);
}

int32_t SteamAudioStreamPlayback::_mix(AudioFrame *buffer, double rate_scale, int32_t frames) {
	if (stream_playback.is_null()) {
		return frames;
	}

	if (Engine::get_singleton()->is_editor_hint()) {
		return frames;
	}

	if (!is_local_state_init.load()) {
		return frames;
	}

	SteamAudio::log(SteamAudio::log_debug, "mixing");
	auto gs = SteamAudioServer::get_singleton()->get_global_state();

	PackedVector2Array mixed_frames = stream_playback->get_raw_audio(rate_scale, frames);
	frames = int(mixed_frames.size());

	for (int i = 0; i < frames; i++) {
		local_state.bufs.in.data[0][i] = mixed_frames[i].x;
		local_state.bufs.in.data[1][i] = mixed_frames[i].y;
	}

	local_state.direct_outputs.flags = static_cast<IPLDirectEffectFlags>(
			IPL_DIRECTEFFECTFLAGS_APPLYOCCLUSION |
			IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION |
			IPL_DIRECTEFFECTFLAGS_APPLYTRANSMISSION);
	iplDirectEffectApply(
			local_state.fx.direct, &local_state.direct_outputs,
			&local_state.bufs.in, &local_state.bufs.direct);

	IPLAmbisonicsEncodeEffectParams enc_params{};
	enc_params.direction = ipl_vec3_from(local_state.dir_to_listener);
	enc_params.order = 2;
	iplAmbisonicsEncodeEffectApply(
			gs->ambi_enc_effect, &enc_params,
			&local_state.bufs.direct, &local_state.bufs.ambi);

	IPLAmbisonicsDecodeEffectParams dec_params{};
	dec_params.orientation = gs->listener_coords;
	dec_params.order = 2;
	dec_params.hrtf = gs->hrtf;
	dec_params.binaural = IPL_TRUE;
	iplAmbisonicsDecodeEffectApply(
			gs->ambi_dec_effect, &dec_params,
			&local_state.bufs.ambi, &local_state.bufs.out);

	for (int i = 0; i < frames; i++) {
		buffer[i].left = local_state.bufs.direct.data[0][i];
		buffer[i].right = local_state.bufs.direct.data[1][i];
	}

	SteamAudio::log(SteamAudio::log_debug, "mix done");
	return frames;
}

void SteamAudioStreamPlayback::_bind_methods() {
	ClassDB::bind_method(D_METHOD("play_stream", "stream", "from_offset", "volume_db", "pitch_scale"), &SteamAudioStreamPlayback::play_stream, DEFVAL(0), DEFVAL(0), DEFVAL(1.0));
	ClassDB::bind_method(D_METHOD("set_parent", "node"), &SteamAudioStreamPlayback::set_parent);
}

void SteamAudioStreamPlayback::set_parent(Node3D *node) {
	this->local_state.src.player = node;
}

void SteamAudioStreamPlayback::set_cfg(SteamAudioSourceConfig cfg) {
	this->local_state.cfg = cfg;
}

int SteamAudioStreamPlayback::play_stream(const Ref<AudioStream> &p_stream, float p_from_offset, float p_volume_db, float p_pitch_scale) {
	if (Engine::get_singleton()->is_editor_hint()) {
		return 0;
	}

	stream = p_stream;
	stream_playback = stream->instantiate_playback();
	stream_playback->start(p_from_offset);

	this->init_local_state();

	SteamAudioServer::get_singleton()->add_local_state(&this->local_state);
	is_local_state_init.store(true);
	return 0;
}

void SteamAudioStreamPlayback::init_local_state() {
	SteamAudio::log(SteamAudio::log_debug, "init local state");
	auto gs = SteamAudioServer::get_singleton()->get_global_state();

	IPLSourceSettings src_cfg{};
	src_cfg.flags = IPL_SIMULATIONFLAGS_DIRECT;
	iplSourceCreate(gs->sim, &src_cfg, &local_state.src.src);
	iplSourceAdd(local_state.src.src, gs->sim);
	iplSimulatorCommit(gs->sim);

	IPLDirectEffectSettings dir_effect_cfg;
	dir_effect_cfg.numChannels = 2;
	iplDirectEffectCreate(gs->ctx, &gs->audio_cfg, &dir_effect_cfg, &local_state.fx.direct);

	iplAudioBufferAllocate(gs->ctx, 2, gs->audio_cfg.frameSize, &local_state.bufs.in);
	iplAudioBufferAllocate(gs->ctx, 2, gs->audio_cfg.frameSize, &local_state.bufs.direct);
	iplAudioBufferAllocate(gs->ctx, 9, gs->audio_cfg.frameSize, &local_state.bufs.ambi);
	iplAudioBufferAllocate(gs->ctx, 2, gs->audio_cfg.frameSize, &local_state.bufs.out);

	SteamAudio::log(SteamAudio::log_debug, "init local state done");
}

void SteamAudioStreamPlayback::_start(double from_pos) {
	if (stream_playback == nullptr || !stream_playback->is_playing()) {
		return;
	}
	stream_playback->start(from_pos);
}

void SteamAudioStreamPlayback::_stop() {
	if (stream_playback == nullptr || !stream_playback->is_playing()) {
		return;
	}
	stream_playback->stop();
}
