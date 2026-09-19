#ifndef STEAM_AUDIO_SERVER_H
#define STEAM_AUDIO_SERVER_H

#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/classes/thread.hpp"
#include "listener.hpp"
#include "steam_audio.hpp"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>
#include <vector>

using namespace godot;

class SteamAudioServer : public Object {
	GDCLASS(SteamAudioServer, Object)

private:
	static SteamAudioServer *self;
	GlobalSteamAudioState global_state{};
	std::vector<LocalSteamAudioState *> local_states;

	std::atomic<bool> is_global_state_init;
	std::atomic<bool> is_refl_thread_processing;
	std::atomic<bool> is_running;
	std::atomic<bool> local_states_have_changed;
	std::mutex init_mux;
	std::mutex refl_mux;
	std::mutex tick_mux;
	std::condition_variable cv;

	// Simulator race, caught under a debugger (project/race.gd): the AUDIO thread ran
	// init_local_state -> iplSourceAdd + iplSimulatorCommit while the MAIN thread destroyed another player's source and
	// the reflection thread ran iplSimulatorRunReflections -- SimulationManager::commit copied a list node already freed
	// (0xfeeefeee). Steam Audio forbids changing the source list or the scene while a simulation runs. So: every such
	// change is QUEUED (queue_op, which retains the handle) and APPLIED in tick() only when the simulator is idle
	// (a try_lock on sim_mux: a reflection pass in flight just defers them a frame, it never stalls the main thread);
	// runs hold sim_mux shared, so direct and reflection runs may still overlap as Steam Audio allows.
	struct PendingOp {
		enum Kind { SOURCE_ADD, SOURCE_REMOVE, STATIC_ADD, STATIC_REMOVE, INSTANCED_ADD, INSTANCED_REMOVE } kind;
		void *handle;
	};
	std::mutex queue_mux;
	std::vector<PendingOp> pending_ops;
	void apply_pending_ops(); // caller holds sim_mux exclusively

	// meshes to add to the global state scene after it's initialized.
	std::vector<IPLStaticMesh> static_meshes_to_add;
	std::vector<IPLStaticMesh> dynamic_meshes_to_add;

	// TODO: allow for multiple
	SteamAudioListener *listener = nullptr;

	void init_scene(IPLSceneSettings *scene_cfg);
	void start_refl_sim();
	void run_refl_sim();
	Ref<Thread> refl_thread;

protected:
	static void _bind_methods();

public:
	SteamAudioServer();
	~SteamAudioServer();

	static SteamAudioServer *get_singleton();
	GlobalSteamAudioState *get_global_state(bool should_init = true);

	void add_listener(SteamAudioListener *listener);
	void add_local_state(LocalSteamAudioState *ls);
	void remove_local_state(LocalSteamAudioState *ls);
	void add_static_mesh(IPLStaticMesh mesh);
	void remove_static_mesh(IPLStaticMesh mesh);
	void add_dynamic_mesh(IPLInstancedMesh mesh);
	void remove_dynamic_mesh(IPLInstancedMesh mesh);

	void tick();
	void shutdown(); // stop and join the reflection thread (called on unload, SCENE level)

	std::shared_mutex sim_mux; // simulation runs: shared; source/scene changes: exclusive (tick only)
	void queue_source_add(IPLSource src);
	void queue_source_remove(IPLSource src);
};

#endif // STEAM_AUDIO_SERVER_H
