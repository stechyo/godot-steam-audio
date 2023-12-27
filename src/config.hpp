#ifndef STEAM_AUDIO_CONFIG_H
#define STEAM_AUDIO_CONFIG_H

#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "phonon.h"
#include "steam_audio.hpp"

using namespace godot;

VARIANT_ENUM_CAST(SteamAudio::GodotSteamAudioLogLevel);

class SteamAudioConfig : public Node3D {
	GDCLASS(SteamAudioConfig, Node3D);

private:
	std::vector<IPLStaticMesh> meshes;

protected:
	static void _bind_methods();

public:
	static SteamAudio::GodotSteamAudioLogLevel log_level;
	static float hrtf_volume;

	SteamAudioConfig();
	~SteamAudioConfig();
	void _physics_process(double delta) override;

	SteamAudio::GodotSteamAudioLogLevel get_global_log_level();
	void set_global_log_level(SteamAudio::GodotSteamAudioLogLevel p_global_log_level);

	float get_hrtf_volume();
	void set_hrtf_volume(float p_hrtf_volume);
};

#endif // STEAM_AUDIO_CONFIG_H
