#include "listener.hpp"
#include "config.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "server.hpp"

void SteamAudioListener::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_refl_duration"), &SteamAudioListener::get_refl_duration);
	ClassDB::bind_method(D_METHOD("set_refl_duration", "p_refl_duration"), &SteamAudioListener::set_refl_duration);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "reflection_duration", PROPERTY_HINT_RANGE, "0.1,10.0,0.1"), "set_refl_duration", "get_refl_duration");
	ClassDB::bind_method(D_METHOD("get_irradiance_min_dist"), &SteamAudioListener::get_irradiance_min_dist);
	ClassDB::bind_method(D_METHOD("set_irradiance_min_dist", "p_irradiance_min_dist"), &SteamAudioListener::set_irradiance_min_dist);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "irradiance_min_distance", PROPERTY_HINT_RANGE, "0.1,5.0,0.1"), "set_irradiance_min_dist", "get_irradiance_min_dist");
	ClassDB::bind_method(D_METHOD("get_num_refl_rays"), &SteamAudioListener::get_num_refl_rays);
	ClassDB::bind_method(D_METHOD("set_num_refl_rays", "p_num_refl_rays"), &SteamAudioListener::set_num_refl_rays);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "reflection_rays", PROPERTY_HINT_RANGE, "1,8192,1"), "set_num_refl_rays", "get_num_refl_rays");
	ClassDB::bind_method(D_METHOD("get_num_refl_bounces"), &SteamAudioListener::get_num_refl_bounces);
	ClassDB::bind_method(D_METHOD("set_num_refl_bounces", "p_num_refl_bounces"), &SteamAudioListener::set_num_refl_bounces);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "reflection_bounces", PROPERTY_HINT_RANGE, "1,64,1"), "set_num_refl_bounces", "get_num_refl_bounces");
	ClassDB::bind_method(D_METHOD("get_refl_ambisonics_order"), &SteamAudioListener::get_refl_ambisonics_order);
	ClassDB::bind_method(D_METHOD("set_refl_ambisonics_order", "p_refl_ambisonics_order"), &SteamAudioListener::set_refl_ambisonics_order);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "reflection_ambisonics_order", PROPERTY_HINT_RANGE, "0,5,1"), "set_refl_ambisonics_order", "get_refl_ambisonics_order");
}

void SteamAudioListener::ready_internal() {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	if (refl_ambisonics_order > SteamAudioConfig::max_ambisonics_order) {
		refl_ambisonics_order = SteamAudioConfig::max_ambisonics_order;
	}
	if (refl_duration > SteamAudioConfig::max_refl_duration) {
		refl_duration = SteamAudioConfig::max_refl_duration;
	}
	if (num_refl_rays > SteamAudioConfig::max_num_refl_rays) {
		num_refl_rays = SteamAudioConfig::max_num_refl_rays;
	}
	SteamAudioServer::get_singleton()->add_listener(this);
}

void SteamAudioListener::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
			ready_internal();
			break;
	}
}

SteamAudioListener::SteamAudioListener() {}
SteamAudioListener::~SteamAudioListener() {}

int SteamAudioListener::get_num_refl_rays() { return num_refl_rays; }
void SteamAudioListener::set_num_refl_rays(int p_num_refl_rays) { num_refl_rays = p_num_refl_rays; }
int SteamAudioListener::get_num_refl_bounces() { return num_refl_bounces; }
void SteamAudioListener::set_num_refl_bounces(int p_num_refl_bounces) { num_refl_bounces = p_num_refl_bounces; }
int SteamAudioListener::get_refl_ambisonics_order() { return refl_ambisonics_order; }
void SteamAudioListener::set_refl_ambisonics_order(int p_refl_ambisonics_order) { refl_ambisonics_order = p_refl_ambisonics_order; }
float SteamAudioListener::get_refl_duration() { return refl_duration; }
void SteamAudioListener::set_refl_duration(float p_refl_duration) { refl_duration = p_refl_duration; }
float SteamAudioListener::get_irradiance_min_dist() { return irradiance_min_dist; }
void SteamAudioListener::set_irradiance_min_dist(float p_irradiance_min_dist) { irradiance_min_dist = p_irradiance_min_dist; }

PackedStringArray SteamAudioListener::_get_configuration_warnings() const {
	PackedStringArray res;

	if (refl_ambisonics_order > SteamAudioConfig::max_ambisonics_order) {
		res.push_back("Ambisonics order exceeds maximum set in SteamAudioConfig. \
				When the game starts, the order on this player will be set to the maximum.");
	}
	if (refl_duration > SteamAudioConfig::max_refl_duration) {
		res.push_back("Reflection duration exceeds maximum set in SteamAudioConfig. \
				When the game starts, the value on this listener will be set to the maximum.");
	}
	if (num_refl_rays > SteamAudioConfig::max_num_refl_rays) {
		res.push_back("Reflection rays exceed maximum set in SteamAudioConfig. \
				When the game starts, the value on this listener will be set to the maximum.");
	}

	return res;
}
