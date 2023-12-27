#ifndef STEAM_AUDIO_LISTENER_HPP
#define STEAM_AUDIO_LISTENER_HPP

#include "godot_cpp/classes/wrapped.hpp"
#include <godot_cpp/classes/node3d.hpp>

using namespace godot;

class SteamAudioListener : public Node3D {
	GDCLASS(SteamAudioListener, Node3D);

protected:
	static void _bind_methods();

public:
	void _ready() override;
	SteamAudioListener();
	~SteamAudioListener();
};

#endif // STEAM_AUDIO_LISTENER_HPP
