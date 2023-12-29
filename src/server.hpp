#ifndef STEAM_AUDIO_SERVER_H
#define STEAM_AUDIO_SERVER_H

#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/classes/thread.hpp"
#include "listener.hpp"
#include "steam_audio.hpp"
#include <atomic>
#include <condition_variable>
#include <mutex>

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
	std::mutex init_mux;
	std::mutex refl_mux;
	std::condition_variable cv;

	// TODO: allow for multiple
	SteamAudioListener *listener;

	void init_scene(IPLSceneSettings *scene_cfg);
	void start_refl_sim();
	void run_refl_sim();
	Thread refl_thread = Thread();

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

	void tick();
};

#endif // STEAM_AUDIO_SERVER_H
