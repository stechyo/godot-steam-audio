
#ifndef STEAM_AUDIO_PLAYER_H
#define STEAM_AUDIO_PLAYER_H

#include "godot_cpp/classes/audio_stream.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "steam_audio.hpp"
#include <phonon.h>
#include <godot_cpp/classes/audio_stream_player3d.hpp>

using namespace godot;

class SteamAudioPlayer : public AudioStreamPlayer3D {
	GDCLASS(SteamAudioPlayer, AudioStreamPlayer3D);

private:
	// This ref is kept because at destruction we can't get the playback
	// since the player has stopped (even though the playback still mixes...)
	Ref<AudioStreamPlayback> pb;

	// TODO: we can probably move these values inside local state
	// for cleanup and the ability to adjust them at runtime
	SteamAudioSourceConfig cfg{
		4.0f,
		32,
		16,
		0.0f,
		1,
		10000.0f,
		false,
		true,
		true,
		false
	};

	LocalSteamAudioState local_state;
	std::atomic<bool> is_local_state_init;
	std::atomic<bool> can_load_local_state;

	void init_local_state();

protected:
	static void _bind_methods();

public:
	SteamAudioPlayer();
	~SteamAudioPlayer();
	void ready_internal();
	void process_internal(double delta);
	void _notification(int p_what);

	LocalSteamAudioState *get_local_state();

	float get_occlusion_radius();
	void set_occlusion_radius(float p_occlusion_radius);
	int get_occlusion_samples();
	void set_occlusion_samples(int p_occlusion_samples);
	int get_transmission_rays();
	void set_transmission_rays(int p_transmission_rays);
	float get_min_attenuation_dist();
	void set_min_attenuation_dist(float p_min_attenuation_dist);
	int get_ambisonics_order();
	void set_ambisonics_order(int p_ambisonics_order);
	float get_max_reflection_dist();
	void set_max_reflection_dist(float p_max_reflection_dist);

	bool is_dist_attn_on();
	void set_dist_attn_on(bool p_dist_attn_on);
	bool is_ambisonics_on();
	void set_ambisonics_on(bool p_ambisonics_on);
	bool is_reflection_on();
	void set_reflection_on(bool p_reflection_on);
	bool is_occlusion_on();
	void set_occlusion_on(bool p_occlusion_on);

	PackedStringArray _get_configuration_warnings() const override;
};

#endif // STEAM_AUDIO_PLAYER_H
