#include "server.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/project_settings.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "phonon.h"
#include "server_init.hpp"
#include "steam_audio.hpp"
#include <algorithm>
#include <godot_cpp/variant/utility_functions.hpp>

void SteamAudioServer::tick() {
	if (!self->is_global_state_init.load()) {
		return;
	}

	if (self->listener == nullptr) {
		return;
	}

	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	SteamAudio::log(SteamAudio::log_debug, "tick");

	iplSceneCommit(self->global_state.scene);
	iplSimulatorSetScene(self->global_state.sim, self->global_state.scene);
	iplSimulatorCommit(self->global_state.sim);

	SteamAudio::log(SteamAudio::log_debug, "tick: committed scene");

	self->global_state.listener_coords =
			ipl_coords_from(self->listener->get_global_transform());

	SteamAudio::log(SteamAudio::log_debug, "tick: committed scene");
	for (auto ls : self->local_states) {
		if (ls->src.player == nullptr) {
			UtilityFunctions::push_warning(
					"local state has empty player, not updating simulation state");
		}

		Vector3 src_pos = ls->src.player->get_global_position();
		ls->dir_to_listener = src_pos - self->listener->get_global_position();

		IPLDistanceAttenuationModel attn_model{};
		attn_model.type = IPL_DISTANCEATTENUATIONTYPE_INVERSEDISTANCE;
		attn_model.minDistance = ls->cfg.min_attn_dist;

		IPLCoordinateSpace3 src_coords;
		src_coords.ahead = IPLVector3{};
		src_coords.up = IPLVector3{};
		src_coords.right = IPLVector3{};
		src_coords.origin = ipl_vec3_from(src_pos);

		IPLSimulationInputs inputs{};
		inputs.flags = IPL_SIMULATIONFLAGS_DIRECT;
		inputs.directFlags = static_cast<IPLDirectSimulationFlags>(
				IPL_DIRECTSIMULATIONFLAGS_DISTANCEATTENUATION |
				IPL_DIRECTSIMULATIONFLAGS_OCCLUSION |
				IPL_DIRECTSIMULATIONFLAGS_TRANSMISSION);
		inputs.distanceAttenuationModel = attn_model;
		inputs.source = src_coords;
		inputs.occlusionType = IPL_OCCLUSIONTYPE_VOLUMETRIC;
		inputs.occlusionRadius = ls->cfg.occ_radius;
		inputs.numOcclusionSamples = ls->cfg.occ_samples;
		inputs.numTransmissionRays = ls->cfg.transm_rays;

		SteamAudio::log(SteamAudio::log_debug, "tick: setting inputs");
		iplSourceSetInputs(ls->src.src, IPL_SIMULATIONFLAGS_DIRECT, &inputs);
	}
	SteamAudio::log(SteamAudio::log_debug, "tick: direct inputs set");

	IPLSimulationSharedInputs sharedInputs{};
	sharedInputs.listener = self->global_state.listener_coords;
	iplSimulatorSetSharedInputs(self->global_state.sim,
			IPL_SIMULATIONFLAGS_DIRECT, &sharedInputs);
	iplSimulatorRunDirect(self->global_state.sim);

	SteamAudio::log(SteamAudio::log_debug, "tick: direct sim complete");

	for (auto ls : self->local_states) {
		IPLSimulationOutputs outputs{};
		iplSourceGetOutputs(ls->src.src, IPL_SIMULATIONFLAGS_DIRECT, &outputs);
		ls->direct_outputs = outputs.direct;
	}
	SteamAudio::log(SteamAudio::log_debug, "tick: done");
}

GlobalSteamAudioState *SteamAudioServer::get_global_state() {
	self->init_mux.lock();
	if (self->is_global_state_init.load()) {
		self->init_mux.unlock();
		return &self->global_state;
	}

	SteamAudio::log(SteamAudio::log_info, "Initializing SteamAudioServer global state");

	self->global_state.audio_cfg = create_audio_cfg();
	self->global_state.ctx = create_ctx();

	IPLSceneSettings scene_cfg = create_scene_cfg(self->global_state.ctx);

	IPLerror err = iplSceneCreate(self->global_state.ctx, &scene_cfg, &self->global_state.scene);
	handleErr(err);

	self->global_state.sim = create_simulator(
			self->global_state.ctx, self->global_state.audio_cfg, scene_cfg);
	self->global_state.hrtf = create_hrtf(self->global_state.ctx, self->global_state.audio_cfg);
	self->global_state.ambi_enc_effect = create_ambisonics_encode_effect(
			self->global_state.ctx, self->global_state.audio_cfg);
	self->global_state.ambi_dec_effect = create_ambisonics_decode_effect(
			self->global_state.ctx, self->global_state.audio_cfg, self->global_state.hrtf);

	SteamAudio::log(SteamAudio::log_debug, "setting scene");
	// HACK: this print makes iplSimulatorSetScene not crash. Don't ask.
	std::cout << self->global_state.scene << std::endl;
	iplSimulatorSetScene(self->global_state.sim, self->global_state.scene);
	iplSimulatorCommit(self->global_state.sim);

	SteamAudio::log(SteamAudio::log_debug, "init global state done");
	self->is_global_state_init.store(true);
	self->init_mux.unlock();
	return &self->global_state;
}

void SteamAudioServer::add_listener(SteamAudioListener *lis) {
	self->listener = lis;
}

void SteamAudioServer::add_local_state(LocalSteamAudioState *ls) {
	self->local_states.push_back(ls);
}

void SteamAudioServer::remove_local_state(LocalSteamAudioState *ls) {
	auto it = std::find(local_states.begin(), local_states.end(), ls);
	if (it == local_states.end()) {
		return;
	}
	local_states.erase(it);
}

SteamAudioServer::SteamAudioServer() {
	self = this;
	self->is_global_state_init.store(false);
}

SteamAudioServer::~SteamAudioServer() {
	if (!self->is_global_state_init.load()) {
		return;
	}
	SteamAudio::log(SteamAudio::log_debug, "destroying steam audio server");

	iplAmbisonicsDecodeEffectRelease(&self->global_state.ambi_dec_effect);
	iplAmbisonicsEncodeEffectRelease(&self->global_state.ambi_enc_effect);
	iplHRTFRelease(&self->global_state.hrtf);
	iplSimulatorRelease(&self->global_state.sim);
	iplSceneRelease(&self->global_state.scene);
	iplContextRelease(&self->global_state.ctx);
}

void SteamAudioServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("tick"), &SteamAudioServer::tick);
	ClassDB::bind_static_method("SteamAudioServer", D_METHOD("get_singleton"),
			&SteamAudioServer::get_singleton);
}

SteamAudioServer *SteamAudioServer::get_singleton() {
	return self;
}

SteamAudioServer *SteamAudioServer::self = nullptr;
