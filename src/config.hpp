#ifndef STEAM_AUDIO_CONFIG_H
#define STEAM_AUDIO_CONFIG_H

#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "phonon.h"
#include "steam_audio.hpp"

using namespace godot;

VARIANT_ENUM_CAST(SteamAudio::GodotSteamAudioLogLevel);
VARIANT_ENUM_CAST(IPLSceneType);

class SteamAudioConfig : public Node3D {
	GDCLASS(SteamAudioConfig, Node3D);

private:
	std::vector<IPLStaticMesh> meshes;

protected:
	static void _bind_methods();

public:
	static SteamAudio::GodotSteamAudioLogLevel log_level;
	static float hrtf_volume;
	static int max_ambisonics_order;
	static int max_num_occ_samples;
	static int max_num_refl_rays;
	static int num_diffuse_samples;
	static float max_refl_duration;
	static int max_num_refl_srcs;
	static int num_refl_threads;
	static IPLSceneType scene_type;

	SteamAudioConfig();
	~SteamAudioConfig();
	void _ready() override;
	void _physics_process(double delta) override;

	SteamAudio::GodotSteamAudioLogLevel get_global_log_level();
	void set_global_log_level(SteamAudio::GodotSteamAudioLogLevel p_global_log_level);

	float get_hrtf_volume();
	void set_hrtf_volume(float p_hrtf_volume);
	int get_max_ambisonics_order();
	void set_max_ambisonics_order(int p_max_ambisonics_order);
	IPLSceneType get_scene_type();
	void set_scene_type(IPLSceneType p_scene_type);
	float get_num_refl_threads();
	void set_num_refl_threads(float p_num_refl_threads);
	float get_max_num_refl_srcs();
	void set_max_num_refl_srcs(float p_max_num_refl_srcs);
	float get_max_refl_duration();
	void set_max_refl_duration(float p_max_refl_duration);
	int get_num_diffuse_samples();
	void set_num_diffuse_samples(int p_num_diffuse_samples);
	int get_max_num_refl_rays();
	void set_max_num_refl_rays(int p_max_num_refl_rays);
	int get_max_num_occ_samples();
	void set_max_num_occ_samples(int p_max_num_occ_samples);
};

#endif // STEAM_AUDIO_CONFIG_H
