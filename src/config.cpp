#include "config.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "server.hpp"
#include "steam_audio.hpp"

SteamAudio::GodotSteamAudioLogLevel SteamAudioConfig::log_level = SteamAudio::log_info;
float SteamAudioConfig::hrtf_volume = 1.0f;
int SteamAudioConfig::max_ambisonics_order = 1;
int SteamAudioConfig::max_num_occ_samples = 64;
int SteamAudioConfig::max_num_refl_rays = 4096;
int SteamAudioConfig::num_diffuse_samples = 32;
float SteamAudioConfig::max_refl_duration = 2.0f;
int SteamAudioConfig::max_num_refl_srcs = 8;
int SteamAudioConfig::num_refl_threads = 2;
IPLSceneType SteamAudioConfig::scene_type = IPL_SCENETYPE_EMBREE; // TODO: support more types

void SteamAudioConfig::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_global_log_level"), &SteamAudioConfig::get_global_log_level);
	ClassDB::bind_method(D_METHOD("set_global_log_level", "p_global_log_level"), &SteamAudioConfig::set_global_log_level);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "global_log_level", PROPERTY_HINT_ENUM, "Debug,Info,Warning,Error"), "set_global_log_level", "get_global_log_level");

	ADD_GROUP("Performance", "");
	ClassDB::bind_method(D_METHOD("get_scene_type"), &SteamAudioConfig::get_scene_type);
	ClassDB::bind_method(D_METHOD("set_scene_type", "p_scene_type"), &SteamAudioConfig::set_scene_type);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "scene_type", PROPERTY_HINT_ENUM, "Default,Embree"), "set_scene_type", "get_scene_type");

	ClassDB::bind_method(D_METHOD("get_hrtf_volume"), &SteamAudioConfig::get_hrtf_volume);
	ClassDB::bind_method(D_METHOD("set_hrtf_volume", "p_hrtf_volume"), &SteamAudioConfig::set_hrtf_volume);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "hrtf_volume", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_hrtf_volume", "get_hrtf_volume");

	ClassDB::bind_method(D_METHOD("get_max_refl_duration"), &SteamAudioConfig::get_max_refl_duration);
	ClassDB::bind_method(D_METHOD("set_max_refl_duration", "p_max_refl_duration"), &SteamAudioConfig::set_max_refl_duration);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_reflection_duration", PROPERTY_HINT_RANGE, "0.1,10.0,0.1"), "set_max_refl_duration", "get_max_refl_duration");

	ClassDB::bind_method(D_METHOD("get_max_ambisonics_order"), &SteamAudioConfig::get_max_ambisonics_order);
	ClassDB::bind_method(D_METHOD("set_max_ambisonics_order", "p_max_ambisonics_order"), &SteamAudioConfig::set_max_ambisonics_order);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_ambisonics_order", PROPERTY_HINT_RANGE, "0,5,1"), "set_max_ambisonics_order", "get_max_ambisonics_order");

	ClassDB::bind_method(D_METHOD("get_max_num_occ_samples"), &SteamAudioConfig::get_max_num_occ_samples);
	ClassDB::bind_method(D_METHOD("set_max_num_occ_samples", "p_max_num_occ_samples"), &SteamAudioConfig::set_max_num_occ_samples);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_occlusion_samples", PROPERTY_HINT_RANGE, "1,256,1"), "set_max_num_occ_samples", "get_max_num_occ_samples");

	ClassDB::bind_method(D_METHOD("get_max_num_refl_rays"), &SteamAudioConfig::get_max_num_refl_rays);
	ClassDB::bind_method(D_METHOD("set_max_num_refl_rays", "p_max_num_refl_rays"), &SteamAudioConfig::set_max_num_refl_rays);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_reflection_rays", PROPERTY_HINT_RANGE, "1,8192,1"), "set_max_num_refl_rays", "get_max_num_refl_rays");

	ClassDB::bind_method(D_METHOD("get_num_diffuse_samples"), &SteamAudioConfig::get_num_diffuse_samples);
	ClassDB::bind_method(D_METHOD("set_num_diffuse_samples", "p_num_diffuse_samples"), &SteamAudioConfig::set_num_diffuse_samples);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "diffuse_samples", PROPERTY_HINT_RANGE, "1,256,1"), "set_num_diffuse_samples", "get_num_diffuse_samples");

	ClassDB::bind_method(D_METHOD("get_max_num_refl_srcs"), &SteamAudioConfig::get_max_num_refl_srcs);
	ClassDB::bind_method(D_METHOD("set_max_num_refl_srcs", "p_max_num_refl_srcs"), &SteamAudioConfig::set_max_num_refl_srcs);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_reflection_sources", PROPERTY_HINT_RANGE, "1,512,1"), "set_max_num_refl_srcs", "get_max_num_refl_srcs");

	ClassDB::bind_method(D_METHOD("get_num_refl_threads"), &SteamAudioConfig::get_num_refl_threads);
	ClassDB::bind_method(D_METHOD("set_num_refl_threads", "p_num_refl_threads"), &SteamAudioConfig::set_num_refl_threads);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "reflection_threads", PROPERTY_HINT_RANGE, "1,64,1"), "set_num_refl_threads", "get_num_refl_threads");
}

SteamAudioConfig::SteamAudioConfig() {}
SteamAudioConfig::~SteamAudioConfig() {}

void SteamAudioConfig::ready_internal() {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	// Initialize global state
	SteamAudioServer::get_singleton()->get_global_state();
	set_physics_process(true);
}

void SteamAudioConfig::process_internal(double delta) {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	SteamAudioServer::get_singleton()->tick();
}

void SteamAudioConfig::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
			ready_internal();
			break;
		case NOTIFICATION_PHYSICS_PROCESS:
			process_internal(get_physics_process_delta_time());
			break;
	}
}

SteamAudio::GodotSteamAudioLogLevel SteamAudioConfig::get_global_log_level() { return log_level; }
void SteamAudioConfig::set_global_log_level(SteamAudio::GodotSteamAudioLogLevel p_global_log_level) { log_level = p_global_log_level; }

float SteamAudioConfig::get_hrtf_volume() { return hrtf_volume; }
void SteamAudioConfig::set_hrtf_volume(float p_hrtf_volume) { hrtf_volume = p_hrtf_volume; }

int SteamAudioConfig::get_max_ambisonics_order() { return max_ambisonics_order; }
void SteamAudioConfig::set_max_ambisonics_order(int p_max_ambisonics_order) { max_ambisonics_order = p_max_ambisonics_order; }

IPLSceneType SteamAudioConfig::get_scene_type() { return scene_type; }
void SteamAudioConfig::set_scene_type(IPLSceneType p_scene_type) { scene_type = p_scene_type; }

float SteamAudioConfig::get_num_refl_threads() { return num_refl_threads; }
void SteamAudioConfig::set_num_refl_threads(float p_num_refl_threads) { num_refl_threads = p_num_refl_threads; }

float SteamAudioConfig::get_max_num_refl_srcs() { return max_num_refl_srcs; }
void SteamAudioConfig::set_max_num_refl_srcs(float p_max_num_refl_srcs) { max_num_refl_srcs = p_max_num_refl_srcs; }

float SteamAudioConfig::get_max_refl_duration() { return max_refl_duration; }
void SteamAudioConfig::set_max_refl_duration(float p_max_refl_duration) { max_refl_duration = p_max_refl_duration; }

int SteamAudioConfig::get_num_diffuse_samples() { return num_diffuse_samples; }
void SteamAudioConfig::set_num_diffuse_samples(int p_num_diffuse_samples) { num_diffuse_samples = p_num_diffuse_samples; }

int SteamAudioConfig::get_max_num_refl_rays() { return max_num_refl_rays; }
void SteamAudioConfig::set_max_num_refl_rays(int p_max_num_refl_rays) { max_num_refl_rays = p_max_num_refl_rays; }

int SteamAudioConfig::get_max_num_occ_samples() { return max_num_occ_samples; }
void SteamAudioConfig::set_max_num_occ_samples(int p_max_num_occ_samples) { max_num_occ_samples = p_max_num_occ_samples; }
