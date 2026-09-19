#include "server.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/project_settings.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/memory.hpp"
#include "godot_cpp/variant/callable_method_pointer.hpp"
#include "phonon.h"
#include "player.hpp"
#include "server_init.hpp"
#include "steam_audio.hpp"
#include <algorithm>
#include <godot_cpp/variant/utility_functions.hpp>

// Simulator race, caught under a debugger (project/race.gd): the AUDIO thread ran
// init_local_state -> iplSourceAdd + iplSimulatorCommit while the MAIN thread destroyed another player's source and
// the reflection thread ran iplSimulatorRunReflections -- SimulationManager::commit copied a list node already freed
// (0xfeeefeee). Steam Audio forbids changing the source list or the scene while a simulation runs. So: every such
// change is QUEUED (queue_op, which retains the handle) and APPLIED in tick() only when the simulator is idle
// (a try_lock on sim_mux: a reflection pass in flight just defers them a frame, it never stalls the main thread);
// runs hold sim_mux shared, so direct and reflection runs may still overlap as Steam Audio allows.
void SteamAudioServer::queue_source_add(IPLSource src) {
	std::lock_guard<std::mutex> q(queue_mux);
	pending_ops.push_back({ PendingOp::SOURCE_ADD, iplSourceRetain(src) });
}

void SteamAudioServer::queue_source_remove(IPLSource src) {
	std::lock_guard<std::mutex> q(queue_mux);
	pending_ops.push_back({ PendingOp::SOURCE_REMOVE, iplSourceRetain(src) });
}

void SteamAudioServer::apply_pending_ops() {
	std::vector<PendingOp> ops;
	{
		std::lock_guard<std::mutex> q(queue_mux);
		ops.swap(pending_ops);
	}
	bool scene_changed = scene_dirty.exchange(false); // a dynamic mesh moved
	if (ops.empty() && !scene_changed) {
		return;
	}
	for (auto &op : ops) {
		scene_changed = scene_changed || (op.kind != PendingOp::SOURCE_ADD && op.kind != PendingOp::SOURCE_REMOVE);
		switch (op.kind) {
			case PendingOp::SOURCE_ADD: {
				auto s = static_cast<IPLSource>(op.handle);
				iplSourceAdd(s, global_state.sim);
				iplSourceRelease(&s);
			} break;
			case PendingOp::SOURCE_REMOVE: {
				auto s = static_cast<IPLSource>(op.handle);
				iplSourceRemove(s, global_state.sim);
				iplSourceRelease(&s);
			} break;
			case PendingOp::STATIC_ADD: {
				auto m = static_cast<IPLStaticMesh>(op.handle);
				iplStaticMeshAdd(m, global_state.scene);
				iplStaticMeshRelease(&m);
			} break;
			case PendingOp::STATIC_REMOVE: {
				auto m = static_cast<IPLStaticMesh>(op.handle);
				iplStaticMeshRemove(m, global_state.scene);
				iplStaticMeshRelease(&m);
			} break;
			case PendingOp::INSTANCED_ADD: {
				auto m = static_cast<IPLInstancedMesh>(op.handle);
				iplInstancedMeshAdd(m, global_state.scene);
				iplInstancedMeshRelease(&m);
			} break;
			case PendingOp::INSTANCED_REMOVE: {
				auto m = static_cast<IPLInstancedMesh>(op.handle);
				iplInstancedMeshRemove(m, global_state.scene);
				iplInstancedMeshRelease(&m);
			} break;
		}
	}
	if (scene_changed) {
		iplSceneCommit(global_state.scene); // only for mesh changes: it is the expensive one
	}
	iplSimulatorCommit(global_state.sim);
}

void SteamAudioServer::tick() {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	if (!self->is_global_state_init.load()) {
		return;
	}
	std::lock_guard<std::mutex> tick_lock(self->tick_mux);
	if (self->listener == nullptr || !self->listener->is_inside_tree()) {
		return;
	}

	SteamAudio::log(SteamAudio::log_debug, "tick");

	{
		// Apply queued source/scene changes only while no simulation is running (never blocks).
		std::unique_lock<std::shared_mutex> exclusive(self->sim_mux, std::try_to_lock);
		// No unconditional iplSceneCommit here: it rebuilds the scene's acceleration structure every call, changed or
		// not (~50 ms per frame on the main thread with ~800k triangles). apply_pending_ops commits what changed.
		if (exclusive.owns_lock() && !is_refl_thread_processing.load()) {
			self->apply_pending_ops();
		}
	}
	std::shared_lock<std::shared_mutex> running(self->sim_mux);

	SteamAudio::log(SteamAudio::log_debug, "tick: committed scene");

	self->global_state.listener_coords =
			ipl_coords_from(self->listener->get_global_transform());

	for (auto ls : self->local_states) {
		if (ls->src.player == nullptr || !ls->src.player->is_inside_tree()) {
			continue;
		}
		if (!ls->src.player->is_playing()) {
			continue;
		}

		Vector3 src_pos = ls->src.player->get_global_position();
		ls->dir_to_listener = src_pos - self->listener->get_global_position();

		IPLDistanceAttenuationModel attn_model{};
		attn_model.type = IPL_DISTANCEATTENUATIONTYPE_INVERSEDISTANCE;
		attn_model.minDistance = ls->cfg.min_attn_dist;

		IPLAirAbsorptionModel absorp_model{};
		absorp_model.type = ls->cfg.air_absorption_model_type;
		absorp_model.coefficients[0] = ls->cfg.air_absorption_low;
		absorp_model.coefficients[1] = ls->cfg.air_absorption_mid;
		absorp_model.coefficients[2] = ls->cfg.air_absorption_high;

		IPLCoordinateSpace3 src_coords = ipl_coords_from(ls->src.player->get_global_transform());

		IPLSimulationInputs inputs{};
		inputs.flags = IPL_SIMULATIONFLAGS_DIRECT;
		inputs.distanceAttenuationModel = attn_model;
		inputs.airAbsorptionModel = absorp_model;
		inputs.source = src_coords;
		inputs.occlusionType = IPL_OCCLUSIONTYPE_VOLUMETRIC;
		inputs.occlusionRadius = ls->cfg.occ_radius;
		inputs.numOcclusionSamples = ls->cfg.occ_samples;
		inputs.numTransmissionRays = ls->cfg.transm_rays;

		if (ls->cfg.is_air_absorp_on) {
			inputs.directFlags = static_cast<IPLDirectSimulationFlags>(
					inputs.directFlags |
					IPL_DIRECTSIMULATIONFLAGS_AIRABSORPTION);
		}

		if (ls->cfg.is_dist_attn_on) {
			inputs.directFlags = static_cast<IPLDirectSimulationFlags>(
					inputs.directFlags |
					IPL_DIRECTSIMULATIONFLAGS_DISTANCEATTENUATION);
		}
		if (ls->cfg.is_occlusion_on) {
			inputs.directFlags = static_cast<IPLDirectSimulationFlags>(
					inputs.directFlags |
					IPL_DIRECTSIMULATIONFLAGS_OCCLUSION |
					IPL_DIRECTSIMULATIONFLAGS_TRANSMISSION);
		}
		if (ls->cfg.is_directivity_on) {
			inputs.directivity.dipoleWeight = ls->cfg.dipole_weight;
			inputs.directivity.dipolePower = ls->cfg.dipole_power;
			inputs.directFlags = static_cast<IPLDirectSimulationFlags>(
					inputs.directFlags |
					IPL_DIRECTSIMULATIONFLAGS_DIRECTIVITY);
		}

		SteamAudio::log(SteamAudio::log_debug, "tick: setting inputs");
		iplSourceSetInputs(ls->src.src, IPL_SIMULATIONFLAGS_DIRECT, &inputs);
	}
	SteamAudio::log(SteamAudio::log_debug, "tick: direct inputs set");

	IPLSimulationSharedInputs shared_inputs{};
	shared_inputs.listener = self->global_state.listener_coords;
	iplSimulatorSetSharedInputs(self->global_state.sim,
			IPL_SIMULATIONFLAGS_DIRECT, &shared_inputs);
	iplSimulatorRunDirect(self->global_state.sim);

	SteamAudio::log(SteamAudio::log_debug, "tick: direct sim complete");

	for (auto ls : self->local_states) {
		if (ls->src.player == nullptr || !ls->src.player->is_inside_tree()) {
			continue;
		}
		if (!ls->src.player->is_playing()) {
			continue;
		}

		IPLSimulationOutputs outputs{};
		iplSourceGetOutputs(ls->src.src, IPL_SIMULATIONFLAGS_DIRECT, &outputs);
		ls->direct_outputs = outputs.direct;
	}

	if (is_refl_thread_processing.load()) {
		SteamAudio::log(SteamAudio::log_debug, "tick: done, skipping reflections");
		return;
	}

	global_state.refl_ir_lock.lock();
	for (auto ls : local_states) {
		if (ls->src.player == nullptr || !ls->src.player->is_inside_tree()) {
			continue;
		}
		if (!ls->src.player->is_playing()) {
			continue;
		}
		if (listener == nullptr || !listener->is_inside_tree()) {
			continue;
		}

		if (ls->src.player->get_global_position().distance_to(listener->get_global_position()) > ls->cfg.max_refl_dist) {
			continue;
		}

		// Zero-initialised: iplSourceGetOutputs does not write every field (tanDevice among them), and
		// iplReflectionEffectApply dereferences a non-null tanDevice -- stack garbage there crashed the mixer.
		IPLSimulationOutputs outputs{};
		iplSourceGetOutputs(ls->src.src, IPL_SIMULATIONFLAGS_REFLECTIONS, &outputs);
		ls->refl_outputs = outputs.reflections;
		ls->refl_outputs.tanDevice = nullptr; // convolution only; TrueAudio Next is not used
	}
	global_state.refl_ir_lock.unlock();

	for (auto ls : self->local_states) {
		if (ls->src.player == nullptr || !ls->src.player->is_inside_tree()) {
			continue;
		}
		if (!ls->src.player->is_playing()) {
			continue;
		}
		if (listener == nullptr || !listener->is_inside_tree()) {
			continue;
		}
		if (ls->src.player->get_global_position().distance_to(listener->get_global_position()) > ls->cfg.max_refl_dist) {
			continue;
		}

		auto player = dynamic_cast<SteamAudioPlayer *>(ls->src.player);
		if (player == nullptr || !player->is_reflection_on()) {
			continue;
		}

		IPLCoordinateSpace3 src_coords = ipl_coords_from(ls->src.player->get_global_transform());

		IPLSimulationInputs inputs{};
		inputs.flags = IPL_SIMULATIONFLAGS_REFLECTIONS;
		inputs.source = src_coords;

		iplSourceSetInputs(ls->src.src, IPL_SIMULATIONFLAGS_REFLECTIONS, &inputs);
	}

	if (listener == nullptr || !listener->is_inside_tree()) {
		return;
	}
	shared_inputs = IPLSimulationSharedInputs{};
	shared_inputs.listener = global_state.listener_coords;
	shared_inputs.numRays = listener->get_num_refl_rays();
	shared_inputs.numBounces = listener->get_num_refl_bounces();
	shared_inputs.duration = listener->get_refl_duration();
	shared_inputs.order = listener->get_refl_ambisonics_order();
	shared_inputs.irradianceMinDistance = listener->get_irradiance_min_dist();
	iplSimulatorSetSharedInputs(global_state.sim, IPL_SIMULATIONFLAGS_REFLECTIONS, &shared_inputs);

	{
		// notify reflection thread and tell it it can start running again
		std::unique_lock<std::mutex> lock(refl_mux);
		is_refl_thread_processing.store(true);
		cv.notify_one();
	}

	SteamAudio::log(SteamAudio::log_debug, "tick: done");
}

GlobalSteamAudioState *SteamAudioServer::get_global_state(bool should_init) {
	self->init_mux.lock();
	if (self->is_global_state_init.load()) {
		self->init_mux.unlock();
		return &self->global_state;
	}

	if (!should_init) {
		self->init_mux.unlock();
		return nullptr;
	}

	SteamAudio::log(SteamAudio::log_info, "Initializing SteamAudioServer global state");

	global_state.audio_cfg = create_audio_cfg();
	global_state.ctx = create_ctx();

	IPLSceneSettings scene_cfg = create_scene_cfg(global_state.ctx);
	IPLerror err = iplSceneCreate(global_state.ctx, &scene_cfg, &global_state.scene);
	handleErr(err);
	for (auto m : static_meshes_to_add) {
		iplStaticMeshAdd(m, global_state.scene);
	}

	global_state.sim = create_simulator(
			global_state.ctx, global_state.audio_cfg, scene_cfg);
	global_state.hrtf = create_hrtf(global_state.ctx, global_state.audio_cfg);
	global_state.ambi_enc_effect = create_ambisonics_encode_effect(
			global_state.ctx, global_state.audio_cfg);
	global_state.ambi_dec_effect = create_ambisonics_decode_effect(
			global_state.ctx, global_state.audio_cfg, global_state.hrtf);

	iplSimulatorSetScene(global_state.sim, global_state.scene);
	iplSimulatorCommit(global_state.sim);

	is_global_state_init.store(true);
	init_mux.unlock();

	SteamAudio::log(SteamAudio::log_info, "Initialized SteamAudioServer global state");
	start_refl_sim();
	return &global_state;
}

void SteamAudioServer::start_refl_sim() {
	refl_thread.instantiate();
	refl_thread->start(callable_mp(this, &SteamAudioServer::run_refl_sim));
}

void SteamAudioServer::run_refl_sim() {
	while (this->is_running.load()) {
		{
			std::unique_lock<std::mutex> lock(this->refl_mux);
			cv.wait(lock, [&] { return is_refl_thread_processing.load() || !is_running.load(); });
		}
		// if someone removed a local state, then the reflection sim might crash, so
		// we need it to wait for another tick.
		// XXX: what happens if a local state is removed in the middle of a sim run...?
		if (local_states_have_changed.load()) {
			local_states_have_changed.store(false);
			is_refl_thread_processing.store(false);
			continue;
		}
		SteamAudio::log(SteamAudio::log_debug, "running reflection sim");
		{
			std::shared_lock<std::shared_mutex> running(sim_mux);
			iplSimulatorRunReflections(global_state.sim);
		}
		is_refl_thread_processing.store(false);
	}
}

void SteamAudioServer::add_listener(SteamAudioListener *lis) {
	std::lock_guard<std::mutex> lock(self->tick_mux);
	self->listener = lis;
}

void SteamAudioServer::add_local_state(LocalSteamAudioState *ls) {
	std::lock_guard<std::mutex> lock(self->tick_mux);
	self->local_states.push_back(ls);
}

void SteamAudioServer::remove_local_state(LocalSteamAudioState *ls) {
	std::lock_guard<std::mutex> lock(tick_mux);
	auto it = std::find(local_states.begin(), local_states.end(), ls);
	if (it == local_states.end()) {
		return;
	}
	local_states.erase(it);
	local_states_have_changed.store(true);
}

void SteamAudioServer::add_static_mesh(IPLStaticMesh mesh) {
	if (is_global_state_init.load()) {
		std::lock_guard<std::mutex> q(queue_mux);
		pending_ops.push_back({ PendingOp::STATIC_ADD, iplStaticMeshRetain(mesh) });
	} else {
		static_meshes_to_add.push_back(mesh);
	}
}

void SteamAudioServer::remove_static_mesh(IPLStaticMesh mesh) {
	if (is_global_state_init.load()) {
		std::lock_guard<std::mutex> q(queue_mux);
		pending_ops.push_back({ PendingOp::STATIC_REMOVE, iplStaticMeshRetain(mesh) });
	} else {
		// Probably won't happen?
		auto it = std::find(static_meshes_to_add.begin(), static_meshes_to_add.end(), mesh);
		if (it != static_meshes_to_add.end()) {
			static_meshes_to_add.erase(it);
		}
	}
}

void SteamAudioServer::add_dynamic_mesh(IPLInstancedMesh mesh) {
	if (is_global_state_init.load()) {
		std::lock_guard<std::mutex> q(queue_mux);
		pending_ops.push_back({ PendingOp::INSTANCED_ADD, iplInstancedMeshRetain(mesh) });
	} else {
		SteamAudio::log(SteamAudio::log_error, "Adding a dynamic mesh, but SteamAudio is not initialized. Probably crashing soon.");
	}
}

void SteamAudioServer::remove_dynamic_mesh(IPLInstancedMesh mesh) {
	if (!is_global_state_init.load()) {
		return; // We've probably already deleted the scene.
	}

	std::lock_guard<std::mutex> q(queue_mux);
	pending_ops.push_back({ PendingOp::INSTANCED_REMOVE, iplInstancedMeshRetain(mesh) });
}

SteamAudioServer::SteamAudioServer() {
	self = this;
	is_global_state_init.store(false);
	is_refl_thread_processing.store(false);
	is_running.store(true);
	local_states_have_changed.store(false);
}

void SteamAudioServer::shutdown() {
	is_running.store(false);
	{
		std::unique_lock<std::mutex> lock(refl_mux);
		cv.notify_one();
	}
	if (refl_thread.is_valid() && refl_thread->is_started()) {
		refl_thread->wait_to_finish();
	}
	refl_thread.unref();
}

SteamAudioServer::~SteamAudioServer() {
	is_running.store(false);
	{
		std::unique_lock<std::mutex> lock(refl_mux);
		cv.notify_one();
	}
	if (refl_thread.is_valid()) {
		refl_thread->wait_to_finish();
		refl_thread.unref();
	}

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
