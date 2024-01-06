#ifndef STEAM_AUDIO_H
#define STEAM_AUDIO_H

#include "godot_cpp/variant/transform3d.hpp"
#include <phonon.h>
#include <godot_cpp/classes/audio_stream_player3d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <mutex>

using namespace godot;

class SteamAudio {
public:
	typedef enum {
		log_debug,
		log_info,
		log_error
	} GodotSteamAudioLogLevel;

	static void log(GodotSteamAudioLogLevel lvl, const char *str);
};

struct GlobalSteamAudioState {
	IPLScene scene;
	IPLAudioSettings audio_cfg;
	IPLContext ctx;
	IPLHRTF hrtf;
	IPLAmbisonicsEncodeEffect ambi_enc_effect;
	IPLAmbisonicsDecodeEffect ambi_dec_effect;
	IPLSimulationSettings sim_cfg;
	IPLSimulator sim;
	IPLCoordinateSpace3 listener_coords;
	std::mutex refl_ir_lock;
};

struct SteamAudioSource {
	AudioStreamPlayer3D *player = nullptr;
	IPLSource src;
};

struct SteamAudioSourceConfig {
	float occ_radius;
	int occ_samples;
	int transm_rays;
	float min_attn_dist;
	int ambisonics_order;
	bool is_dist_attn_on;
	bool is_ambisonics_on;
	bool is_occlusion_on;
	bool is_reflection_on;
};

struct SteamAudioEffects {
	IPLDirectEffect direct;
	IPLReflectionEffect refl;
	IPLAmbisonicsDecodeEffect dec;
	IPLAmbisonicsDecodeEffect refl_dec;
	IPLAmbisonicsEncodeEffect enc;
};

struct LocalSteamAudioBuffers {
	IPLAudioBuffer in;
	IPLAudioBuffer direct;
	IPLAudioBuffer mono;
	IPLAudioBuffer refl_ambi;
	IPLAudioBuffer refl_out;
	IPLAudioBuffer ambi;
	IPLAudioBuffer out;
};

struct LocalSteamAudioState {
	SteamAudioSource src;
	Vector3 dir_to_listener;
	IPLDirectEffectParams direct_outputs{ {} };
	IPLReflectionEffectParams refl_outputs{ {} };
	LocalSteamAudioBuffers bufs;
	SteamAudioEffects fx;
	SteamAudioSourceConfig cfg;
	std::mutex mux;
};

inline int ambisonic_channels_from(int order) {
	return (order + 1) * (order + 1);
}

inline IPLVector3 ipl_vec3_from(Vector3 v) { return IPLVector3{ v.x, v.y, v.z }; }

inline IPLCoordinateSpace3 ipl_coords_from(Transform3D trf) {
	auto orig = trf.origin;
	auto right = trf.get_basis().get_column(0);
	auto up = trf.get_basis().get_column(1);
	auto fwd = -trf.get_basis().get_column(2);

	IPLCoordinateSpace3 coords;
	coords.origin = ipl_vec3_from(orig);
	coords.right = ipl_vec3_from(right);
	coords.up = ipl_vec3_from(up);
	coords.ahead = ipl_vec3_from(fwd);

	return coords;
}

inline void handleErr(IPLerror err) {
	switch (err) {
		case IPL_STATUS_SUCCESS:
			return;
		case IPL_STATUS_FAILURE:
			SteamAudio::log(SteamAudio::log_error, "Unspecified error in init");
			return;
		case IPL_STATUS_OUTOFMEMORY:
			SteamAudio::log(SteamAudio::log_error, "Out of memory in init");
			return;
		case IPL_STATUS_INITIALIZATION:
			SteamAudio::log(SteamAudio::log_error, "Failed to handle external dependency in init");
			return;
	}
}

#endif
