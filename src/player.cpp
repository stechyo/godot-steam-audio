#include "player.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "stream.hpp"

void SteamAudioPlayer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_sub_stream"), &SteamAudioPlayer::get_sub_stream);
	ClassDB::bind_method(D_METHOD("set_sub_stream", "p_sub_stream"), &SteamAudioPlayer::set_sub_stream);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "sub_stream", PROPERTY_HINT_RESOURCE_TYPE, "AudioStream"), "set_sub_stream", "get_sub_stream");

	ADD_GROUP("Performance", "");
	ClassDB::bind_method(D_METHOD("get_occlusion_radius"), &SteamAudioPlayer::get_occlusion_radius);
	ClassDB::bind_method(D_METHOD("set_occlusion_radius", "p_occlusion_radius"), &SteamAudioPlayer::set_occlusion_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "occlusion_radius", PROPERTY_HINT_RANGE, "0.0,20.0,0.1"), "set_occlusion_radius", "get_occlusion_radius");
	ClassDB::bind_method(D_METHOD("get_occlusion_samples"), &SteamAudioPlayer::get_occlusion_samples);
	ClassDB::bind_method(D_METHOD("set_occlusion_samples", "p_occlusion_samples"), &SteamAudioPlayer::set_occlusion_samples);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "occlusion_samples", PROPERTY_HINT_RANGE, "0,512,1"), "set_occlusion_samples", "get_occlusion_samples");
	ClassDB::bind_method(D_METHOD("get_transmission_rays"), &SteamAudioPlayer::get_transmission_rays);
	ClassDB::bind_method(D_METHOD("set_transmission_rays", "p_transmission_rays"), &SteamAudioPlayer::set_transmission_rays);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "transmission_rays", PROPERTY_HINT_RANGE, "0,512,1"), "set_transmission_rays", "get_transmission_rays");
	ClassDB::bind_method(D_METHOD("get_min_attenuation_distance"), &SteamAudioPlayer::get_min_attenuation_dist);
	ClassDB::bind_method(D_METHOD("set_min_attenuation_distance", "p_min_attenuation_distance"), &SteamAudioPlayer::set_min_attenuation_dist);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_attenuation_distance", PROPERTY_HINT_RANGE, "0.0,100.0,0.1"), "set_min_attenuation_distance", "get_min_attenuation_distance");
}

SteamAudioPlayer::SteamAudioPlayer() {}
SteamAudioPlayer::~SteamAudioPlayer() {}

void SteamAudioPlayer::_ready() {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	Node3D *parent = get_node<Node3D>("..");
	if (parent == nullptr) {
		UtilityFunctions::push_error(" \
			The parent of a SteamAudioPlayer must be a 3D Node. \
			No sound will be played.");
		return;
	}

	auto pb = dynamic_cast<SteamAudioStreamPlayback *>(get_stream_playback().ptr());
	if (pb == nullptr) {
		UtilityFunctions::push_error(" \
			The stream of a SteamAudioPlayer must be a \
			SteamAudioStream. No sound will be played.");
	}

	pb->set_parent(parent);
	pb->set_cfg(SteamAudioSourceConfig{
			occlusion_radius, occlusion_samples,
			transmission_rays, min_attenuation_dist });
	pb->play_stream(sub_stream, 0.0f, float(get_volume_db()), float(get_pitch_scale()));
}

Ref<AudioStream> SteamAudioPlayer::get_sub_stream() { return sub_stream; }
void SteamAudioPlayer::set_sub_stream(Ref<AudioStream> p_sub_stream) { sub_stream = p_sub_stream; }

float SteamAudioPlayer::get_occlusion_radius() { return occlusion_radius; }
void SteamAudioPlayer::set_occlusion_radius(float p_occlusion_radius) { occlusion_radius = p_occlusion_radius; }
int SteamAudioPlayer::get_occlusion_samples() { return occlusion_samples; }
void SteamAudioPlayer::set_occlusion_samples(int p_occlusion_samples) { occlusion_samples = p_occlusion_samples; }
int SteamAudioPlayer::get_transmission_rays() { return transmission_rays; }
void SteamAudioPlayer::set_transmission_rays(int p_transmission_rays) { transmission_rays = p_transmission_rays; }
float SteamAudioPlayer::get_min_attenuation_dist() { return min_attenuation_dist; }
void SteamAudioPlayer::set_min_attenuation_dist(float p_min_attenuation_dist) { min_attenuation_dist = p_min_attenuation_dist; }

PackedStringArray SteamAudioPlayer::_get_configuration_warnings() const {
	PackedStringArray res;
	if (sub_stream == nullptr) {
		res.push_back("Sub stream must be set, otherwise no audio will play.");
	}
	Node3D *parent = get_node<Node3D>("..");
	if (parent == nullptr) {
		res.push_back("The parent of a SteamAudioPlayer must be a Node3D or something that inherits Node3D.");
	}
	return res;
}
