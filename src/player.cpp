#include "player.hpp"
#include "config.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "stream.hpp"

void SteamAudioPlayer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_sub_stream"), &SteamAudioPlayer::get_sub_stream);
	ClassDB::bind_method(D_METHOD("set_sub_stream", "p_sub_stream"), &SteamAudioPlayer::set_sub_stream);
	ClassDB::bind_method(D_METHOD("get_loop_sub_stream"), &SteamAudioPlayer::get_loop_sub_stream);
	ClassDB::bind_method(D_METHOD("set_loop_sub_stream", "p_loop_sub_stream"), &SteamAudioPlayer::set_loop_sub_stream);
	ClassDB::bind_method(D_METHOD("is_dist_attn_on"), &SteamAudioPlayer::is_dist_attn_on);
	ClassDB::bind_method(D_METHOD("set_dist_attn_on", "p_dist_attn_on"), &SteamAudioPlayer::set_dist_attn_on);
	ClassDB::bind_method(D_METHOD("get_min_attenuation_distance"), &SteamAudioPlayer::get_min_attenuation_dist);
	ClassDB::bind_method(D_METHOD("set_min_attenuation_distance", "p_min_attenuation_distance"), &SteamAudioPlayer::set_min_attenuation_dist);
	ClassDB::bind_method(D_METHOD("is_occlusion_on"), &SteamAudioPlayer::is_occlusion_on);
	ClassDB::bind_method(D_METHOD("set_occlusion_on", "p_occlusion_on"), &SteamAudioPlayer::set_occlusion_on);
	ClassDB::bind_method(D_METHOD("is_ambisonics_on"), &SteamAudioPlayer::is_ambisonics_on);
	ClassDB::bind_method(D_METHOD("set_ambisonics_on", "p_ambisonics_on"), &SteamAudioPlayer::set_ambisonics_on);
	ClassDB::bind_method(D_METHOD("is_reflection_on"), &SteamAudioPlayer::is_reflection_on);
	ClassDB::bind_method(D_METHOD("set_reflection_on", "p_reflection_on"), &SteamAudioPlayer::set_reflection_on);
	ClassDB::bind_method(D_METHOD("get_occlusion_radius"), &SteamAudioPlayer::get_occlusion_radius);
	ClassDB::bind_method(D_METHOD("set_occlusion_radius", "p_occlusion_radius"), &SteamAudioPlayer::set_occlusion_radius);
	ClassDB::bind_method(D_METHOD("get_occlusion_samples"), &SteamAudioPlayer::get_occlusion_samples);
	ClassDB::bind_method(D_METHOD("set_occlusion_samples", "p_occlusion_samples"), &SteamAudioPlayer::set_occlusion_samples);
	ClassDB::bind_method(D_METHOD("get_transmission_rays"), &SteamAudioPlayer::get_transmission_rays);
	ClassDB::bind_method(D_METHOD("set_transmission_rays", "p_transmission_rays"), &SteamAudioPlayer::set_transmission_rays);
	ClassDB::bind_method(D_METHOD("get_ambisonics_order"), &SteamAudioPlayer::get_ambisonics_order);
	ClassDB::bind_method(D_METHOD("set_ambisonics_order", "p_ambisonics_order"), &SteamAudioPlayer::set_ambisonics_order);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "sub_stream", PROPERTY_HINT_RESOURCE_TYPE, "AudioStream"), "set_sub_stream", "get_sub_stream");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "loop_sub_stream"), "set_loop_sub_stream", "get_loop_sub_stream");

	ADD_GROUP("Distance Attenuation", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "distance_attenuation"), "set_dist_attn_on", "is_dist_attn_on");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_attenuation_distance", PROPERTY_HINT_RANGE, "0.0,100.0,0.1"), "set_min_attenuation_distance", "get_min_attenuation_distance");

	ADD_GROUP("Occlusion and Transmission", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "occlusion"), "set_occlusion_on", "is_occlusion_on");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "occlusion_radius", PROPERTY_HINT_RANGE, "0.0,20.0,0.1"), "set_occlusion_radius", "get_occlusion_radius");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "occlusion_samples", PROPERTY_HINT_RANGE, "0,512,1"), "set_occlusion_samples", "get_occlusion_samples");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "transmission_rays", PROPERTY_HINT_RANGE, "0,512,1"), "set_transmission_rays", "get_transmission_rays");

	ADD_GROUP("Ambisonics", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "ambisonics"), "set_ambisonics_on", "is_ambisonics_on");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "ambisonics_order", PROPERTY_HINT_RANGE, "0,5,1"), "set_ambisonics_order", "get_ambisonics_order");

	ADD_GROUP("Reflection", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "reflection"), "set_reflection_on", "is_reflection_on");
}

SteamAudioPlayer::SteamAudioPlayer() {}
SteamAudioPlayer::~SteamAudioPlayer() {}

void SteamAudioPlayer::_ready() {
	set_panning_strength(0.0f);
	set_attenuation_model(ATTENUATION_DISABLED);

	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	if (loop_sub_stream && sub_stream->get("loop").get_type() != Variant::NIL) {
		sub_stream->set("loop", true);
	}

	auto pb = dynamic_cast<SteamAudioStreamPlayback *>(get_stream_playback().ptr());
	if (pb == nullptr) {
		UtilityFunctions::push_error(" \
			The stream of a SteamAudioPlayer must be a \
			SteamAudioStream. No sound will be played.");
	}

	if (cfg.ambisonics_order > SteamAudioConfig::max_ambisonics_order) {
		cfg.ambisonics_order = SteamAudioConfig::max_ambisonics_order;
	}
	if (cfg.occ_samples > SteamAudioConfig::max_num_occ_samples) {
		cfg.occ_samples = SteamAudioConfig::max_num_occ_samples;
	}

	pb->set_parent(this);
	pb->set_cfg(cfg);
	pb->play_stream(sub_stream, 0.0f, float(get_volume_db()), float(get_pitch_scale()));
}

void SteamAudioPlayer::_process(double delta) {
	if (get_panning_strength() > 0.0f) {
		UtilityFunctions::push_warning("Panning strength is always zero on SteamAudioPlayer. You can control panning by enabling or disabling ambisonics.");
		set_panning_strength(0.0f);
	}
	if (get_attenuation_model() != ATTENUATION_DISABLED) {
		UtilityFunctions::push_warning("The attenuation model is always disabled on SteamAudioPlayer. You can control attenuation by enabling or disabling it above.");
		set_attenuation_model(ATTENUATION_DISABLED);
	}
}

Ref<AudioStream> SteamAudioPlayer::get_sub_stream() { return sub_stream; }
void SteamAudioPlayer::set_sub_stream(Ref<AudioStream> p_sub_stream) { sub_stream = p_sub_stream; }

float SteamAudioPlayer::get_occlusion_radius() { return cfg.occ_radius; }
void SteamAudioPlayer::set_occlusion_radius(float p_occlusion_radius) { cfg.occ_radius = p_occlusion_radius; }
int SteamAudioPlayer::get_occlusion_samples() { return cfg.occ_samples; }
void SteamAudioPlayer::set_occlusion_samples(int p_occlusion_samples) { cfg.occ_samples = p_occlusion_samples; }
int SteamAudioPlayer::get_transmission_rays() { return cfg.transm_rays; }
void SteamAudioPlayer::set_transmission_rays(int p_transmission_rays) { cfg.transm_rays = p_transmission_rays; }
float SteamAudioPlayer::get_min_attenuation_dist() { return cfg.min_attn_dist; }
void SteamAudioPlayer::set_min_attenuation_dist(float p_min_attenuation_dist) { cfg.min_attn_dist = p_min_attenuation_dist; }
int SteamAudioPlayer::get_ambisonics_order() { return cfg.ambisonics_order; }
void SteamAudioPlayer::set_ambisonics_order(int p_ambisonics_order) { cfg.ambisonics_order = p_ambisonics_order; }
bool SteamAudioPlayer::get_loop_sub_stream() { return loop_sub_stream; }
void SteamAudioPlayer::set_loop_sub_stream(bool p_loop_sub_stream) { loop_sub_stream = p_loop_sub_stream; }

bool SteamAudioPlayer::is_dist_attn_on() { return cfg.is_dist_attn_on; }
void SteamAudioPlayer::set_dist_attn_on(bool p_dist_attn_on) { cfg.is_dist_attn_on = p_dist_attn_on; }
bool SteamAudioPlayer::is_ambisonics_on() { return cfg.is_ambisonics_on; }
void SteamAudioPlayer::set_ambisonics_on(bool p_ambisonics_on) { cfg.is_ambisonics_on = p_ambisonics_on; }
bool SteamAudioPlayer::is_reflection_on() { return cfg.is_reflection_on; }
void SteamAudioPlayer::set_reflection_on(bool p_reflection_on) { cfg.is_reflection_on = p_reflection_on; }
bool SteamAudioPlayer::is_occlusion_on() { return cfg.is_occlusion_on; }
void SteamAudioPlayer::set_occlusion_on(bool p_occlusion_on) { cfg.is_occlusion_on = p_occlusion_on; }

PackedStringArray SteamAudioPlayer::_get_configuration_warnings() const {
	PackedStringArray res;
	if (sub_stream == nullptr) {
		res.push_back("Sub stream must be set, otherwise no audio will play.");
	}
	return res;
}
