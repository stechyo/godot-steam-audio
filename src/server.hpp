#ifndef STEAM_AUDIO_SERVER_H
#define STEAM_AUDIO_SERVER_H

#include "godot_cpp/classes/object.hpp"
#include "listener.hpp"
#include "steam_audio.hpp"
#include <atomic>
#include <mutex>

using namespace godot;

class SteamAudioServer : public Object {
	GDCLASS(SteamAudioServer, Object)

private:
	static SteamAudioServer *self;
	GlobalSteamAudioState global_state{};
	std::vector<LocalSteamAudioState *> local_states;

	std::atomic<bool> is_global_state_init;
	std::mutex init_mux;

	// TODO: allow for multiple
	SteamAudioListener *listener;

protected:
	static void _bind_methods();

public:
	SteamAudioServer();
	~SteamAudioServer();

	static SteamAudioServer *get_singleton();
	GlobalSteamAudioState *get_global_state();

	void add_listener(SteamAudioListener *listener);
	void add_local_state(LocalSteamAudioState *ls);
	void remove_local_state(LocalSteamAudioState *ls);

	void tick();
};

#endif // STEAM_AUDIO_SERVER_H
