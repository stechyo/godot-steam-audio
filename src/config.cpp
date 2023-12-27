#include "config.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "server.hpp"
#include "steam_audio.hpp"

SteamAudio::GodotSteamAudioLogLevel SteamAudioConfig::log_level = SteamAudio::log_debug;

float SteamAudioConfig::hrtf_volume = 1.0f;

void SteamAudioConfig::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_global_log_level"), &SteamAudioConfig::get_global_log_level);
	ClassDB::bind_method(D_METHOD("set_global_log_level", "p_global_log_level"), &SteamAudioConfig::set_global_log_level);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "global_log_level", PROPERTY_HINT_ENUM, "Debug,Info,Error"), "set_global_log_level", "get_global_log_level");

	ClassDB::bind_method(D_METHOD("get_hrtf_volume"), &SteamAudioConfig::get_hrtf_volume);
	ClassDB::bind_method(D_METHOD("set_hrtf_volume", "p_hrtf_volume"), &SteamAudioConfig::set_hrtf_volume);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "hrtf_volume", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_hrtf_volume", "get_hrtf_volume");
}

SteamAudioConfig::SteamAudioConfig() {}
SteamAudioConfig::~SteamAudioConfig() {}

void SteamAudioConfig::_physics_process(double delta) {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	SteamAudioServer::get_singleton()->tick();
}

SteamAudio::GodotSteamAudioLogLevel SteamAudioConfig::get_global_log_level() { return log_level; }
void SteamAudioConfig::set_global_log_level(SteamAudio::GodotSteamAudioLogLevel p_global_log_level) { log_level = p_global_log_level; }

float SteamAudioConfig::get_hrtf_volume() { return hrtf_volume; }
void SteamAudioConfig::set_hrtf_volume(float p_hrtf_volume) { hrtf_volume = p_hrtf_volume; }
