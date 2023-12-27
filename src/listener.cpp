#include "listener.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "server.hpp"

void SteamAudioListener::_bind_methods() {}

void SteamAudioListener::_ready() {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	SteamAudioServer::get_singleton()->add_listener(this);
}

SteamAudioListener::SteamAudioListener() {}
SteamAudioListener::~SteamAudioListener() {}
