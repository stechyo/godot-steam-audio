#ifndef STEAM_AUDIO_MATERIAL_H
#define STEAM_AUDIO_MATERIAL_H

#include "godot_cpp/classes/wrapped.hpp"
#include <phonon.h>
#include <godot_cpp/classes/resource.hpp>

using namespace godot;

class SteamAudioMaterial : public Resource {
	GDCLASS(SteamAudioMaterial, Resource);

	IPLMaterial mat{};

protected:
	static void _bind_methods();

public:
	SteamAudioMaterial();
	~SteamAudioMaterial();

	IPLMaterial get_material();

	float get_absorption_low();
	void set_absorption_low(float p_absorption_low);
	float get_absorption_mid();
	void set_absorption_mid(float p_absorption_mid);
	float get_absorption_high();
	void set_absorption_high(float p_absorption_high);
	float get_scattering();
	void set_scattering(float p_scattering);
	float get_transmission_low();
	void set_transmission_low(float p_transmission_low);
	float get_transmission_mid();
	void set_transmission_mid(float p_transmission_mid);
	float get_transmission_high();
	void set_transmission_high(float p_transmission_high);
};

#endif // STEAM_AUDIO_MATERIAL_H
