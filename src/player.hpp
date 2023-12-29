
#ifndef STEAM_AUDIO_PLAYER_H
#define STEAM_AUDIO_PLAYER_H

#include "godot_cpp/classes/audio_stream.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include <phonon.h>
#include <godot_cpp/classes/audio_stream_player.hpp>

using namespace godot;

class SteamAudioPlayer : public AudioStreamPlayer {
	GDCLASS(SteamAudioPlayer, AudioStreamPlayer);

private:
	Ref<AudioStream> sub_stream;
	float occlusion_radius = 4.0f;
	int occlusion_samples = 32;
	int transmission_rays = 16;
	float min_attenuation_dist = 0.0f;
	int ambisonics_order = 1;
	bool loop_sub_stream = false;

protected:
	static void _bind_methods();

public:
	SteamAudioPlayer();
	~SteamAudioPlayer();
	void _ready() override;

	Ref<AudioStream> get_sub_stream();
	void set_sub_stream(Ref<AudioStream> p_sub_stream);

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
	bool get_loop_sub_stream();
	void set_loop_sub_stream(bool p_loop_sub_stream);

	PackedStringArray _get_configuration_warnings() const override;
};

#endif // STEAM_AUDIO_PLAYER_H
