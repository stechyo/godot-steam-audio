#include "player.hpp"
#include "config.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "server.hpp"
#include "server_init.hpp"
#include "steam_audio.hpp"
#include "stream.hpp"

void SteamAudioPlayer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("play_stream", "stream", "from_offset", "volume_db", "pitch_scale"), &SteamAudioPlayer::play_stream, DEFVAL(0), DEFVAL(0), DEFVAL(1.0));
	ClassDB::bind_method(D_METHOD("get_inner_stream"), &SteamAudioPlayer::get_inner_stream);
	ClassDB::bind_method(D_METHOD("get_inner_stream_playback"), &SteamAudioPlayer::get_inner_stream_playback);

	ClassDB::bind_method(D_METHOD("is_dist_attn_on"), &SteamAudioPlayer::is_dist_attn_on);
	ClassDB::bind_method(D_METHOD("set_dist_attn_on", "p_dist_attn_on"), &SteamAudioPlayer::set_dist_attn_on);
	ClassDB::bind_method(D_METHOD("get_min_attenuation_distance"), &SteamAudioPlayer::get_min_attenuation_dist);
	ClassDB::bind_method(D_METHOD("set_min_attenuation_distance", "p_min_attenuation_distance"), &SteamAudioPlayer::set_min_attenuation_dist);
	ClassDB::bind_method(D_METHOD("set_max_reflection_distance", "p_max_reflection_distance"), &SteamAudioPlayer::set_max_reflection_dist);
	ClassDB::bind_method(D_METHOD("get_max_reflection_distance"), &SteamAudioPlayer::get_max_reflection_dist);
	ClassDB::bind_method(D_METHOD("is_air_absorp_on"), &SteamAudioPlayer::is_air_absorp_on);
	ClassDB::bind_method(D_METHOD("set_air_absorp_on", "p_air_absorp_on"), &SteamAudioPlayer::set_air_absorp_on);
	ClassDB::bind_method(D_METHOD("set_air_absorption_low", "p_air_absorption_low"), &SteamAudioPlayer::set_air_absorption_low);
	ClassDB::bind_method(D_METHOD("get_air_absorption_low"), &SteamAudioPlayer::get_air_absorption_low);
	ClassDB::bind_method(D_METHOD("set_air_absorption_mid", "p_air_absorption_mid"), &SteamAudioPlayer::set_air_absorption_mid);
	ClassDB::bind_method(D_METHOD("get_air_absorption_mid"), &SteamAudioPlayer::get_air_absorption_mid);
	ClassDB::bind_method(D_METHOD("set_air_absorption_high", "p_air_absorption_high"), &SteamAudioPlayer::set_air_absorption_high);
	ClassDB::bind_method(D_METHOD("get_air_absorption_high"), &SteamAudioPlayer::get_air_absorption_high);
	ClassDB::bind_method(D_METHOD("get_air_absorption_model_type"), &SteamAudioPlayer::get_air_absorption_model_type);
	ClassDB::bind_method(D_METHOD("set_air_absorption_model_type", "p_air_absorption_model_type"), &SteamAudioPlayer::set_air_absorption_model_type);
	ClassDB::bind_method(D_METHOD("is_occlusion_on"), &SteamAudioPlayer::is_occlusion_on);
	ClassDB::bind_method(D_METHOD("set_occlusion_on", "p_occlusion_on"), &SteamAudioPlayer::set_occlusion_on);
	ClassDB::bind_method(D_METHOD("is_reflection_on"), &SteamAudioPlayer::is_reflection_on);
	ClassDB::bind_method(D_METHOD("set_reflection_on", "p_reflection_on"), &SteamAudioPlayer::set_reflection_on);
	ClassDB::bind_method(D_METHOD("is_directivity_on"), &SteamAudioPlayer::is_directivity_on);
	ClassDB::bind_method(D_METHOD("set_directivity_on", "p_directivity_on"), &SteamAudioPlayer::set_directivity_on);
	ClassDB::bind_method(D_METHOD("get_dipole_weight"), &SteamAudioPlayer::get_dipole_weight);
	ClassDB::bind_method(D_METHOD("set_dipole_weight", "p_dipole_weight"), &SteamAudioPlayer::set_dipole_weight);
	ClassDB::bind_method(D_METHOD("get_dipole_power"), &SteamAudioPlayer::get_dipole_power);
	ClassDB::bind_method(D_METHOD("set_dipole_power", "p_dipole_power"), &SteamAudioPlayer::set_dipole_power);
	ClassDB::bind_method(D_METHOD("get_occlusion_radius"), &SteamAudioPlayer::get_occlusion_radius);
	ClassDB::bind_method(D_METHOD("set_occlusion_radius", "p_occlusion_radius"), &SteamAudioPlayer::set_occlusion_radius);
	ClassDB::bind_method(D_METHOD("get_occlusion_samples"), &SteamAudioPlayer::get_occlusion_samples);
	ClassDB::bind_method(D_METHOD("set_occlusion_samples", "p_occlusion_samples"), &SteamAudioPlayer::set_occlusion_samples);
	ClassDB::bind_method(D_METHOD("get_transmission_rays"), &SteamAudioPlayer::get_transmission_rays);
	ClassDB::bind_method(D_METHOD("set_transmission_rays", "p_transmission_rays"), &SteamAudioPlayer::set_transmission_rays);
	ClassDB::bind_method(D_METHOD("get_ambisonics_order"), &SteamAudioPlayer::get_ambisonics_order);
	ClassDB::bind_method(D_METHOD("set_ambisonics_order", "p_ambisonics_order"), &SteamAudioPlayer::set_ambisonics_order);
	ClassDB::bind_method(D_METHOD("is_ambisonics_on"), &SteamAudioPlayer::is_ambisonics_on);
	ClassDB::bind_method(D_METHOD("set_ambisonics_on", "p_ambisonics_on"), &SteamAudioPlayer::set_ambisonics_on);

	ADD_GROUP("Distance Attenuation", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "distance_attenuation"), "set_dist_attn_on", "is_dist_attn_on");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_attenuation_distance", PROPERTY_HINT_RANGE, "0.0,100.0,0.1"), "set_min_attenuation_distance", "get_min_attenuation_distance");

	ADD_GROUP("Air Absorption", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "air_absorption"), "set_air_absorp_on", "is_air_absorp_on");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "air_absorption_low", PROPERTY_HINT_RANGE, "0.0,1.0,0.1"), "set_air_absorption_low", "get_air_absorption_low");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "air_absorption_mid", PROPERTY_HINT_RANGE, "0.0,1.0,0.1"), "set_air_absorption_mid", "get_air_absorption_mid");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "air_absorption_high", PROPERTY_HINT_RANGE, "0.0,1.0,0.1"), "set_air_absorption_high", "get_air_absorption_high");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "air_absorption_model", PROPERTY_HINT_ENUM, "Default,Exponential"), "set_air_absorption_model_type", "get_air_absorption_model_type");

	ADD_GROUP("Occlusion and Transmission", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "occlusion"), "set_occlusion_on", "is_occlusion_on");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "occlusion_radius", PROPERTY_HINT_RANGE, "0.0,20.0,0.1"), "set_occlusion_radius", "get_occlusion_radius");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "occlusion_samples", PROPERTY_HINT_RANGE, "0,512,1"), "set_occlusion_samples", "get_occlusion_samples");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "transmission_rays", PROPERTY_HINT_RANGE, "0,512,1"), "set_transmission_rays", "get_transmission_rays");
	ClassDB::bind_method(D_METHOD("get_transmission_type"), &SteamAudioPlayer::get_transmission_type);
	ClassDB::bind_method(D_METHOD("set_transmission_type", "p_transmission_type"), &SteamAudioPlayer::set_transmission_type);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "transmission_type", PROPERTY_HINT_ENUM, "Frequency-Independent,Frequency-Dependent"), "set_transmission_type", "get_transmission_type");

	ADD_GROUP("Ambisonics", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "ambisonics"), "set_ambisonics_on", "is_ambisonics_on");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "ambisonics_order", PROPERTY_HINT_RANGE, "0,5,1"), "set_ambisonics_order", "get_ambisonics_order");

	ADD_GROUP("Reflection", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "reflection"), "set_reflection_on", "is_reflection_on");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_reflection_distance", PROPERTY_HINT_RANGE, "0.0,20000.0,0.1"), "set_max_reflection_distance", "get_max_reflection_distance");

	ADD_GROUP("Directivity", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "directivity"), "set_directivity_on", "is_directivity_on");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "dipole_weight", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_dipole_weight", "get_dipole_weight");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "dipole_power", PROPERTY_HINT_RANGE, "0.0,4.0,0.01"), "set_dipole_power", "get_dipole_power");
}

SteamAudioPlayer::SteamAudioPlayer() {
	handle->player = this;
	is_local_state_init.store(false);
	can_load_local_state.store(true);

	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	auto str = dynamic_cast<SteamAudioStream *>(get_stream().ptr());
	if (str == nullptr) {
		Ref<SteamAudioStream> new_stream;
		new_stream.instantiate();
		new_stream->parent = this;
		new_stream->handle = handle;
		if (get_stream().ptr() != nullptr) {
			new_stream->set_stream(get_stream());
		}
		this->set_stream(new_stream);
	}
}
SteamAudioPlayer::~SteamAudioPlayer() {
	SteamAudio::log(SteamAudio::log_debug, "destroying player");
	{
		// FIRST: no mix may touch this player again, and none is inside it now.
		std::unique_lock hl(handle->mux);
		handle->alive.store(false);
		handle->player = nullptr;
	}
	if (!is_local_state_init.load()) {
		return;
	}

	std::unique_lock lock(local_state.mux);

	is_local_state_init.store(false);
	can_load_local_state.store(false);
	SteamAudioServer::get_singleton()->remove_local_state(&local_state);
	auto gs = SteamAudioServer::get_singleton()->get_global_state();

	SteamAudioServer::get_singleton()->queue_source_remove(local_state.src.src);
	iplSourceRelease(&local_state.src.src);
	iplDirectEffectRelease(&local_state.fx.direct);
	iplReflectionEffectRelease(&local_state.fx.refl);
	iplAmbisonicsDecodeEffectRelease(&local_state.fx.dec);
	iplAmbisonicsDecodeEffectRelease(&local_state.fx.refl_dec);
	iplAmbisonicsEncodeEffectRelease(&local_state.fx.enc);

	iplAudioBufferFree(gs->ctx, &local_state.bufs.in);
	iplAudioBufferFree(gs->ctx, &local_state.bufs.direct);
	iplAudioBufferFree(gs->ctx, &local_state.bufs.ambi);
	iplAudioBufferFree(gs->ctx, &local_state.bufs.out);
	iplAudioBufferFree(gs->ctx, &local_state.bufs.mono);
	iplAudioBufferFree(gs->ctx, &local_state.bufs.refl_ambi);
	iplAudioBufferFree(gs->ctx, &local_state.bufs.refl_out);

	// (the playback's raw `parent` used to be cleared here through a stored Ref; the handle above does that job,
	//  and releasing that Ref at shutdown freed a playback the AudioServer still held -- see player.hpp `pb`)
}

LocalSteamAudioState *SteamAudioPlayer::get_local_state(bool may_init) {
	if (!can_load_local_state.load()) {
		return nullptr;
	}
	if (!is_local_state_init.load()) {
		// Built on the MAIN thread only (process_internal / play_stream). The audio thread used to build it
		// mid-mix, which is where the simulator race started.
		if (!may_init) {
			return nullptr;
		}
		init_local_state();
	}
	return &local_state;
}

void SteamAudioPlayer::init_local_state() {
	SteamAudio::log(SteamAudio::log_debug, "init local state");
	auto gs = SteamAudioServer::get_singleton()->get_global_state();
	local_state.cfg = cfg;

	IPLSourceSettings src_cfg{};
	src_cfg.flags = static_cast<IPLSimulationFlags>(IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS);
	handleErr(iplSourceCreate(gs->sim, &src_cfg, &local_state.src.src));
	// queued, not added here: see SteamAudioServer::apply_pending_ops
	SteamAudioServer::get_singleton()->queue_source_add(local_state.src.src);

	// TODO: check if we can't create effects globally and use their Reset functions.
	// If we create these globally and use them for all sources, then strange things happen
	// (e.g. one source may start to play audio from all sources and positioning gets screwed)
	IPLDirectEffectSettings dir_effect_cfg;
	dir_effect_cfg.numChannels = 2;
	handleErr(iplDirectEffectCreate(gs->ctx, &gs->audio_cfg, &dir_effect_cfg, &local_state.fx.direct));

	IPLReflectionEffectSettings refl_effect_cfg{};
	refl_effect_cfg.type = IPL_REFLECTIONEFFECTTYPE_CONVOLUTION;
	refl_effect_cfg.irSize = int(SteamAudioConfig::max_refl_duration * float(gs->audio_cfg.samplingRate));
	refl_effect_cfg.numChannels = ambisonic_channels_from(local_state.cfg.ambisonics_order);
	handleErr(iplReflectionEffectCreate(gs->ctx, &gs->audio_cfg, &refl_effect_cfg, &local_state.fx.refl));

	local_state.fx.dec = create_ambisonics_decode_effect(
			gs->ctx, gs->audio_cfg, gs->hrtf);
	local_state.fx.refl_dec = create_ambisonics_decode_effect(
			gs->ctx, gs->audio_cfg, gs->hrtf);
	local_state.fx.enc = create_ambisonics_encode_effect(
			gs->ctx, gs->audio_cfg);

	handleErr(iplAudioBufferAllocate(gs->ctx, 2, gs->audio_cfg.frameSize, &local_state.bufs.in));
	handleErr(iplAudioBufferAllocate(gs->ctx, 2, gs->audio_cfg.frameSize, &local_state.bufs.direct));
	handleErr(iplAudioBufferAllocate(gs->ctx, ambisonic_channels_from(local_state.cfg.ambisonics_order), gs->audio_cfg.frameSize, &local_state.bufs.ambi));
	handleErr(iplAudioBufferAllocate(gs->ctx, 2, gs->audio_cfg.frameSize, &local_state.bufs.out));
	handleErr(iplAudioBufferAllocate(gs->ctx, 1, gs->audio_cfg.frameSize, &local_state.bufs.mono));
	handleErr(iplAudioBufferAllocate(gs->ctx, ambisonic_channels_from(local_state.cfg.ambisonics_order), gs->audio_cfg.frameSize, &local_state.bufs.refl_ambi));
	handleErr(iplAudioBufferAllocate(gs->ctx, 2, gs->audio_cfg.frameSize, &local_state.bufs.refl_out));
	local_state.src.player = this;

	SteamAudio::log(SteamAudio::log_debug, "init local state done");

	SteamAudioServer::get_singleton()->add_local_state(&this->local_state);
	is_local_state_init.store(true);
}

void SteamAudioPlayer::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
			ready_internal();
			break;
		case NOTIFICATION_EXIT_TREE:
			if (!Engine::get_singleton()->is_editor_hint() && is_local_state_init.load()) {
				SteamAudioServer::get_singleton()->remove_local_state(&local_state);
			}
			break;
		case NOTIFICATION_PROCESS:
			process_internal(get_process_delta_time());
			break;
	}
}

void SteamAudioPlayer::ready_internal() {
	set_process(true);

	set_panning_strength(0.0f);
	if (cfg.is_dist_attn_on) {
		set_attenuation_model(ATTENUATION_DISABLED);
	}

	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	auto str = dynamic_cast<SteamAudioStream *>(get_stream().ptr());
	if (str == nullptr) {
		if (is_autoplay_enabled()) {
			stop();
		}
		Ref<SteamAudioStream> new_stream;
		new_stream.instantiate();
		if (get_stream().ptr() != nullptr) {
			new_stream->set_stream(get_stream());
		}
		set_stream(new_stream);
		str = new_stream.ptr();
		str->parent = this;
		str->handle = handle;
		if (is_autoplay_enabled()) {
			play();
		}
	}

	if (cfg.ambisonics_order > SteamAudioConfig::max_ambisonics_order) {
		cfg.ambisonics_order = SteamAudioConfig::max_ambisonics_order;
	}
	if (cfg.occ_samples > SteamAudioConfig::max_num_occ_samples) {
		cfg.occ_samples = SteamAudioConfig::max_num_occ_samples;
	}
}

void SteamAudioPlayer::process_internal(double delta) {
	if (get_panning_strength() > 0.0f) {
		if (!has_warned_panning) {
			UtilityFunctions::push_warning("Panning strength is always zero on SteamAudioPlayer. You can control panning by enabling or disabling ambisonics.");
			has_warned_panning = true;
		}
		set_panning_strength(0.0f);
	}
	if (cfg.is_dist_attn_on && get_attenuation_model() != ATTENUATION_DISABLED) {
		if (!has_warned_attenuation) {
			UtilityFunctions::push_warning("You cannot enable Godot's and SteamAudio's distance attenuation features at the same time. Disable SteamAudio's attenuation before adjusting Godot's.");
			has_warned_attenuation = true;
		}
		set_attenuation_model(ATTENUATION_DISABLED);
	}

	if (is_playing() && !get_stream_playback().is_null()) {
		get_local_state(); // build it here, on the main thread, before the mixer ever needs it
	}

	// Sync cfg changes (e.g. runtime property toggles) into local_state.cfg.
	// local_state.cfg is a copy made at init time, so setters must be reflected here.
	if (is_local_state_init.load() && cfg_dirty.exchange(false)) {
		std::unique_lock lock(local_state.mux);
		local_state.cfg = cfg;
	}
}

void SteamAudioPlayer::play_stream(const Ref<AudioStream> &p_stream, float p_from_offset, float p_volume_db, float p_pitch_scale) {
	if (p_stream.is_null()) {
		SteamAudio::log(SteamAudio::log_warn, "Tried to play a null stream, won't play anything.");
		return;
	}

	if (this->is_playing()) {
		this->stop();
	}
	this->play();

	auto str = dynamic_cast<SteamAudioStream *>(get_stream().ptr());
	if (str == nullptr) {
		SteamAudio::log(SteamAudio::log_warn,
						"Tried to get an inner stream from a SteamAudioPlayer, but its outer stream is not a SteamAudioStream. Returning null.");
		return;
	}
	str->set_stream(p_stream);

	auto playback_ptr = dynamic_cast<SteamAudioStreamPlayback *>(get_stream_playback().ptr());
	if (playback_ptr == nullptr) {
		SteamAudio::log(SteamAudio::log_warn,
						"Tried to play a new stream on SteamAudioPlayer, but this player's outer stream was not a SteamAudioStream. Will not play anything.");
		this->stop();
		return;
	}

	playback_ptr->play_stream(p_stream, p_from_offset, p_volume_db, p_pitch_scale);
}

Ref<AudioStream> SteamAudioPlayer::get_inner_stream() {
	auto str = dynamic_cast<SteamAudioStream *>(get_stream().ptr());
	if (str == nullptr) {
		SteamAudio::log(SteamAudio::log_warn,
						"Tried to get an inner stream from a SteamAudioPlayer, but its outer stream is not a SteamAudioStream. Returning null.");
		Ref<AudioStream> null_str;
		return null_str;
	}

	return str->get_stream();
}

Ref<AudioStreamPlayback> SteamAudioPlayer::get_inner_stream_playback() {
	auto spb = dynamic_cast<SteamAudioStreamPlayback *>(get_stream_playback().ptr());
	if (spb == nullptr) {
		SteamAudio::log(SteamAudio::log_warn,
						"Tried to get an inner stream playback from a SteamAudioPlayer, but its outer stream playback is not a SteamAudioStreamPlayback (or the player may not be playing audio). Returning null.");
		Ref<AudioStreamPlayback> null_pb;
		return null_pb;
	}

	return spb->get_stream_playback();
}

float SteamAudioPlayer::get_occlusion_radius() { return cfg.occ_radius; }
void SteamAudioPlayer::set_occlusion_radius(float p_occlusion_radius) { cfg.occ_radius = p_occlusion_radius; cfg_dirty.store(true); }
int SteamAudioPlayer::get_occlusion_samples() { return cfg.occ_samples; }
void SteamAudioPlayer::set_occlusion_samples(int p_occlusion_samples) { cfg.occ_samples = p_occlusion_samples; cfg_dirty.store(true); }
int SteamAudioPlayer::get_transmission_rays() { return cfg.transm_rays; }
void SteamAudioPlayer::set_transmission_rays(int p_transmission_rays) { cfg.transm_rays = p_transmission_rays; cfg_dirty.store(true); }
float SteamAudioPlayer::get_min_attenuation_dist() { return cfg.min_attn_dist; }
void SteamAudioPlayer::set_min_attenuation_dist(float p_min_attenuation_dist) { cfg.min_attn_dist = p_min_attenuation_dist; cfg_dirty.store(true); }
int SteamAudioPlayer::get_ambisonics_order() { return cfg.ambisonics_order; }
void SteamAudioPlayer::set_ambisonics_order(int p_ambisonics_order) { cfg.ambisonics_order = p_ambisonics_order; cfg_dirty.store(true); }
float SteamAudioPlayer::get_max_reflection_dist() { return cfg.max_refl_dist; }
void SteamAudioPlayer::set_max_reflection_dist(float p_max_reflection_dist) { cfg.max_refl_dist = p_max_reflection_dist; cfg_dirty.store(true); }

bool SteamAudioPlayer::is_dist_attn_on() { return cfg.is_dist_attn_on; }
void SteamAudioPlayer::set_dist_attn_on(bool p_dist_attn_on) { cfg.is_dist_attn_on = p_dist_attn_on; cfg_dirty.store(true); }

bool SteamAudioPlayer::is_air_absorp_on() { return cfg.is_air_absorp_on; }
void SteamAudioPlayer::set_air_absorp_on(bool p_air_absorp_on) { cfg.is_air_absorp_on = p_air_absorp_on; cfg_dirty.store(true); }
float SteamAudioPlayer::get_air_absorption_low() { return cfg.air_absorption_low; }
void SteamAudioPlayer::set_air_absorption_low(float p_air_absorption_low) { cfg.air_absorption_low = p_air_absorption_low; cfg_dirty.store(true); }
float SteamAudioPlayer::get_air_absorption_mid() { return cfg.air_absorption_mid; }
void SteamAudioPlayer::set_air_absorption_mid(float p_air_absorption_mid) { cfg.air_absorption_mid = p_air_absorption_mid; cfg_dirty.store(true); }
float SteamAudioPlayer::get_air_absorption_high() { return cfg.air_absorption_high; }
void SteamAudioPlayer::set_air_absorption_high(float p_air_absorption_high) { cfg.air_absorption_high = p_air_absorption_high; cfg_dirty.store(true); }
IPLAirAbsorptionModelType SteamAudioPlayer::get_air_absorption_model_type() { return cfg.air_absorption_model_type; }
void SteamAudioPlayer::set_air_absorption_model_type(IPLAirAbsorptionModelType p_air_absorption_model_type) { cfg.air_absorption_model_type = p_air_absorption_model_type; cfg_dirty.store(true); }

bool SteamAudioPlayer::is_reflection_on() { return cfg.is_reflection_on; }
void SteamAudioPlayer::set_reflection_on(bool p_reflection_on) { cfg.is_reflection_on = p_reflection_on; cfg_dirty.store(true); }
bool SteamAudioPlayer::is_occlusion_on() { return cfg.is_occlusion_on; }
void SteamAudioPlayer::set_occlusion_on(bool p_occlusion_on) { cfg.is_occlusion_on = p_occlusion_on; cfg_dirty.store(true); }

IPLTransmissionType SteamAudioPlayer::get_transmission_type() { return cfg.transmission_type; }
void SteamAudioPlayer::set_transmission_type(IPLTransmissionType p_transmission_type) { cfg.transmission_type = p_transmission_type; cfg_dirty.store(true); }

bool SteamAudioPlayer::is_directivity_on() { return cfg.is_directivity_on; }
void SteamAudioPlayer::set_directivity_on(bool p_directivity_on) { cfg.is_directivity_on = p_directivity_on; cfg_dirty.store(true); }
float SteamAudioPlayer::get_dipole_weight() { return cfg.dipole_weight; }
void SteamAudioPlayer::set_dipole_weight(float p_dipole_weight) { cfg.dipole_weight = p_dipole_weight; cfg_dirty.store(true); }
float SteamAudioPlayer::get_dipole_power() { return cfg.dipole_power; }
void SteamAudioPlayer::set_dipole_power(float p_dipole_power) { cfg.dipole_power = p_dipole_power; cfg_dirty.store(true); }

bool SteamAudioPlayer::is_ambisonics_on() { return cfg.is_ambisonics_on; }
void SteamAudioPlayer::set_ambisonics_on(bool p_ambisonics_on) { cfg.is_ambisonics_on = p_ambisonics_on; cfg_dirty.store(true); }

PackedStringArray SteamAudioPlayer::_get_configuration_warnings() const {
	PackedStringArray res;
	return res;
}
