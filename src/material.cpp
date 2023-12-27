#include "material.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/object.hpp"

SteamAudioMaterial::SteamAudioMaterial() {}
SteamAudioMaterial::~SteamAudioMaterial() {}

void SteamAudioMaterial::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_absorption_low"), &SteamAudioMaterial::get_absorption_low);
	ClassDB::bind_method(D_METHOD("set_absorption_low", "p_absorption_low"), &SteamAudioMaterial::set_absorption_low);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "absorption_low", PROPERTY_HINT_RANGE, "0.0,1.0,0.001"), "set_absorption_low", "get_absorption_low");

	ClassDB::bind_method(D_METHOD("get_absorption_mid"), &SteamAudioMaterial::get_absorption_mid);
	ClassDB::bind_method(D_METHOD("set_absorption_mid", "p_absorption_mid"), &SteamAudioMaterial::set_absorption_mid);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "absorption_mid", PROPERTY_HINT_RANGE, "0.0,1.0,0.001"), "set_absorption_mid", "get_absorption_mid");

	ClassDB::bind_method(D_METHOD("get_absorption_high"), &SteamAudioMaterial::get_absorption_high);
	ClassDB::bind_method(D_METHOD("set_absorption_high", "p_absorption_high"), &SteamAudioMaterial::set_absorption_high);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "absorption_high", PROPERTY_HINT_RANGE, "0.0,1.0,0.001"), "set_absorption_high", "get_absorption_high");

	ClassDB::bind_method(D_METHOD("get_scattering"), &SteamAudioMaterial::get_scattering);
	ClassDB::bind_method(D_METHOD("set_scattering", "p_scattering"), &SteamAudioMaterial::set_scattering);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scattering", PROPERTY_HINT_RANGE, "0.0,1.0,0.001"), "set_scattering", "get_scattering");

	ClassDB::bind_method(D_METHOD("get_transmission_low"), &SteamAudioMaterial::get_transmission_low);
	ClassDB::bind_method(D_METHOD("set_transmission_low", "p_transmission_low"), &SteamAudioMaterial::set_transmission_low);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "transmission_low", PROPERTY_HINT_RANGE, "0.0,1.0,0.001"), "set_transmission_low", "get_transmission_low");

	ClassDB::bind_method(D_METHOD("get_transmission_mid"), &SteamAudioMaterial::get_transmission_mid);
	ClassDB::bind_method(D_METHOD("set_transmission_mid", "p_transmission_mid"), &SteamAudioMaterial::set_transmission_mid);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "transmission_mid", PROPERTY_HINT_RANGE, "0.0,1.0,0.001"), "set_transmission_mid", "get_transmission_mid");

	ClassDB::bind_method(D_METHOD("get_transmission_high"), &SteamAudioMaterial::get_transmission_high);
	ClassDB::bind_method(D_METHOD("set_transmission_high", "p_transmission_high"), &SteamAudioMaterial::set_transmission_high);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "transmission_high", PROPERTY_HINT_RANGE, "0.0,1.0,0.001"), "set_transmission_high", "get_transmission_high");
}

IPLMaterial SteamAudioMaterial::get_material() { return mat; }

float SteamAudioMaterial::get_absorption_low() { return mat.absorption[0]; }
void SteamAudioMaterial::set_absorption_low(float p_absorption_low) { mat.absorption[0] = p_absorption_low; }

float SteamAudioMaterial::get_absorption_mid() { return mat.absorption[1]; }
void SteamAudioMaterial::set_absorption_mid(float p_absorption_mid) { mat.absorption[1] = p_absorption_mid; }

float SteamAudioMaterial::get_absorption_high() { return mat.absorption[2]; }
void SteamAudioMaterial::set_absorption_high(float p_absorption_high) { mat.absorption[2] = p_absorption_high; }

float SteamAudioMaterial::get_scattering() { return mat.scattering; }
void SteamAudioMaterial::set_scattering(float p_scattering) { mat.scattering = p_scattering; }

float SteamAudioMaterial::get_transmission_low() { return mat.transmission[0]; }
void SteamAudioMaterial::set_transmission_low(float p_transmission_low) { mat.transmission[0] = p_transmission_low; }

float SteamAudioMaterial::get_transmission_mid() { return mat.transmission[1]; }
void SteamAudioMaterial::set_transmission_mid(float p_transmission_mid) { mat.transmission[1] = p_transmission_mid; }

float SteamAudioMaterial::get_transmission_high() { return mat.transmission[2]; }
void SteamAudioMaterial::set_transmission_high(float p_transmission_high) { mat.transmission[2] = p_transmission_high; }
