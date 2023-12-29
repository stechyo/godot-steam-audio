#ifndef STEAM_AUDIO_LISTENER_HPP
#define STEAM_AUDIO_LISTENER_HPP

#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/packed_string_array.hpp"
#include <godot_cpp/classes/node3d.hpp>

using namespace godot;

class SteamAudioListener : public Node3D {
	GDCLASS(SteamAudioListener, Node3D);

private:
	int num_refl_rays = 4096;
	int num_refl_bounces = 16;
	float refl_duration = 2.0f;
	int refl_ambisonics_order = 1;
	float irradiance_min_dist = 1.0f;

protected:
	static void _bind_methods();

public:
	void _ready() override;
	SteamAudioListener();
	~SteamAudioListener();

	int get_num_refl_rays();
	void set_num_refl_rays(int p_num_refl_rays);
	int get_num_refl_bounces();
	void set_num_refl_bounces(int p_num_refl_bounces);
	int get_refl_ambisonics_order();
	void set_refl_ambisonics_order(int p_refl_ambisonics_order);
	float get_refl_duration();
	void set_refl_duration(float p_refl_duration);
	float get_irradiance_min_dist();
	void set_irradiance_min_dist(float p_irradiance_min_dist);

	PackedStringArray _get_configuration_warnings() const override;
};

#endif // STEAM_AUDIO_LISTENER_HPP
