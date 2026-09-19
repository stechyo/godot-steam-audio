#include "stream.hpp"
#include "config.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/os.hpp"
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
	playback->handle = handle;

	return playback;
}

void SteamAudioStream::set_stream(Ref<AudioStream> p_stream) { stream = p_stream; }
Ref<AudioStream> SteamAudioStream::get_stream() { return this->stream; }

// ----------------------------------------------------
// SteamAudioStreamPlayback

std::mutex SteamAudioStreamPlayback::registry_mux;
std::unordered_set<SteamAudioStreamPlayback *> SteamAudioStreamPlayback::registry;
std::atomic<bool> SteamAudioStreamPlayback::shutting_down{ false };
std::atomic<int> SteamAudioStreamPlayback::made{ 0 };
std::atomic<int> SteamAudioStreamPlayback::destroyed{ 0 };

SteamAudioStreamPlayback::SteamAudioStreamPlayback() {
	made++;
	std::lock_guard<std::mutex> g(registry_mux);
	registry.insert(this);
}
SteamAudioStreamPlayback::~SteamAudioStreamPlayback() {
	destroyed++;
	std::lock_guard<std::mutex> g(registry_mux);
	registry.erase(this);
}

int SteamAudioStreamPlayback::drain(int max_ms) {
	// Called as this library unloads. Godot destroys its AudioServer AFTER unloading extensions, and the server still
	// holds the last reference to playbacks it has retired but not yet freed (it frees them in its per-frame update,
	// which no longer runs). Its final unref would run our destructor from unloaded code -- the exit crash, named with
	// a symbol build of 4.7.2: Main::cleanup -> ~AudioServer -> _cleanup_lists -> Ref<AudioStreamPlayback>::unref.
	// So: stop mixing, then take one reference on every playback still alive. Their memory is reclaimed by the OS a
	// moment later with the process; nothing of ours runs after unload.
	shutting_down.store(true);
	OS::get_singleton()->delay_usec(uint32_t(max_ms) * 1000u); // let any mix in progress finish
	std::lock_guard<std::mutex> g(registry_mux);
	for (auto pb : registry) {
		pb->reference();
	}
	return int(registry.size());
}

int32_t SteamAudioStreamPlayback::_mix(AudioFrame *buffer, float rate_scale, int32_t frames) {
	// Hold the player alive for the whole mix, or do nothing (see SteamAudioPlayerHandle).
	if (shutting_down.load()) {
		return frames;
	}
	std::shared_ptr<SteamAudioPlayerHandle> h = handle;
	if (!h) {
		return frames;
	}
	std::shared_lock hl(h->mux);
	if (!h->alive.load() || h->player == nullptr) {
		return frames;
	}
	SteamAudioPlayer *parent = h->player;

	// Our own reference to the inner playback: play_stream may replace the member on the main thread mid-mix,
	// and dropping the last reference to the one being mixed would free it under us.
	Ref<AudioStreamPlayback> sp;
	{
		std::lock_guard sl(sp_mux);
		sp = stream_playback;
	}
	if (sp.is_null()) {
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

	LocalSteamAudioState *ls = parent->get_local_state(false);
	if (ls == nullptr) { // not built yet (main thread builds it next frame) or being destroyed
		return frames;
	}
	std::unique_lock lock(ls->mux);

	// Some extra checks because at this point parent may have been deleted
	if (parent == nullptr) {
		return frames;
	}
	ls = parent->get_local_state(false);
	if (ls == nullptr || !ls->src.player) {
		return frames;
	}

	PackedVector2Array mixed_frames = sp->mix_audio(rate_scale, frames);
	frames = int(mixed_frames.size());

	auto gs_local = SteamAudioServer::get_singleton()->get_global_state(false);
	if (gs_local != nullptr && frames > gs_local->audio_cfg.frameSize) {
		frames = gs_local->audio_cfg.frameSize;
	}

	for (int i = 0; i < frames; i++) {
		ls->bufs.in.data[0][i] = mixed_frames[i].x;
		ls->bufs.in.data[1][i] = mixed_frames[i].y;
	}

	if (ls->cfg.is_air_absorp_on) {
		ls->direct_outputs.flags = static_cast<IPLDirectEffectFlags>(
				ls->direct_outputs.flags |
				IPL_DIRECTEFFECTFLAGS_APPLYAIRABSORPTION);
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
		ls->direct_outputs.transmissionType = ls->cfg.transmission_type;
	}
	if (ls->cfg.is_directivity_on) {
		ls->direct_outputs.flags = static_cast<IPLDirectEffectFlags>(
				ls->direct_outputs.flags |
				IPL_DIRECTEFFECTFLAGS_APPLYDIRECTIVITY);
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

	if (ls->cfg.is_ambisonics_on && ls->fx.bin != nullptr) {
		// Direct sound through Steam Audio's binaural effect -- the HRTF applied to the point source, as the Steam
		// Audio guide prescribes for direct paths. The plugin used to encode it into order-1 ambisonics and decode,
		// which smears direction (measured: wide-band ILD +/-0.5 dB on speech vs a sharp HRTF). Ambisonics is kept
		// for the reflections below, where a diffuse field is the point.
		const IPLCoordinateSpace3 &lis = gs->listener_coords;
		IPLVector3 src = { lis.origin.x + ls->dir_to_listener.x, lis.origin.y + ls->dir_to_listener.y, lis.origin.z + ls->dir_to_listener.z };
		IPLBinauralEffectParams bin_params{};
		bin_params.direction = ls->dir_to_listener.length_squared() < 1e-6f
				? IPLVector3{ 0.0f, 0.0f, -1.0f } // on top of the listener: straight ahead, not a zero vector
				: iplCalculateRelativeDirection(gs->ctx, src, lis.origin, lis.ahead, lis.up);
		bin_params.interpolation = IPL_HRTFINTERPOLATION_BILINEAR;
		bin_params.spatialBlend = 1.0f;
		bin_params.hrtf = gs->hrtf;
		bin_params.peakDelays = nullptr;
		iplAudioBufferDownmix(gs->ctx, &ls->bufs.direct, &ls->bufs.mono);
		iplBinauralEffectApply(ls->fx.bin, &bin_params, &ls->bufs.mono, &ls->bufs.out);
		SteamAudio::log(SteamAudio::log_debug, "mixing: finished binaural");
	} else if (ls->cfg.is_ambisonics_on) {
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

	Ref<AudioStreamPlayback> fresh = p_stream->instantiate_playback();
	fresh->start(p_from_offset);
	std::lock_guard sl(sp_mux);
	stream = p_stream;
	stream_playback = fresh;

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

bool SteamAudioStreamPlayback::_is_playing() const { return is_active && !shutting_down.load(); }
void SteamAudioStreamPlayback::set_stream(Ref<AudioStream> p_stream) { stream = p_stream; }
Ref<AudioStreamPlayback> SteamAudioStreamPlayback::get_stream_playback() { return this->stream_playback; }
