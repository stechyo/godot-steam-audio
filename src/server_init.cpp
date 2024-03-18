#include "server_init.hpp"
#include "config.hpp"
#include "phonon.h"
#include "steam_audio.hpp"

using namespace godot;

IPLAudioSettings create_audio_cfg() {
	int sample_rate = ProjectSettings::get_singleton()->get_setting("audio/driver/mix_rate");
	int output_latency = ProjectSettings::get_singleton()->get_setting("audio/driver/output_latency");
	int frame_size = closest_power_of_2(output_latency * sample_rate / 1000);
	return IPLAudioSettings{ sample_rate, int(frame_size) };
}

IPLHRTF create_hrtf(IPLContext ctx, IPLAudioSettings audio_cfg) {
	IPLHRTFSettings hrtf_cfg;
	hrtf_cfg.type = IPL_HRTFTYPE_DEFAULT;
	hrtf_cfg.volume = SteamAudioConfig::hrtf_volume;

	IPLHRTF hrtf;
	IPLerror err = iplHRTFCreate(ctx, &audio_cfg, &hrtf_cfg, &hrtf);
	handleErr(err);
	return hrtf;
}

IPLAmbisonicsDecodeEffect create_ambisonics_decode_effect(IPLContext ctx, IPLAudioSettings audio_cfg, IPLHRTF hrtf) {
	IPLAmbisonicsDecodeEffectSettings ambi_dec_cfg;
	ambi_dec_cfg.maxOrder = SteamAudioConfig::max_ambisonics_order;
	ambi_dec_cfg.hrtf = hrtf;
	IPLSpeakerLayout layout;
	layout.type = IPL_SPEAKERLAYOUTTYPE_STEREO;
	ambi_dec_cfg.speakerLayout = layout;

	IPLAmbisonicsDecodeEffect ambi_dec_effect;
	IPLerror err = iplAmbisonicsDecodeEffectCreate(ctx, &audio_cfg,
			&ambi_dec_cfg,
			&ambi_dec_effect);
	handleErr(err);
	return ambi_dec_effect;
}

IPLAmbisonicsEncodeEffect create_ambisonics_encode_effect(IPLContext ctx, IPLAudioSettings audio_cfg) {
	IPLAmbisonicsEncodeEffectSettings ambi_enc_cfg;
	ambi_enc_cfg.maxOrder = SteamAudioConfig::max_ambisonics_order;

	IPLAmbisonicsEncodeEffect ambi_enc_effect;
	IPLerror err = iplAmbisonicsEncodeEffectCreate(ctx, &audio_cfg, &ambi_enc_cfg, &ambi_enc_effect);
	handleErr(err);
	return ambi_enc_effect;
}

IPLSimulator create_simulator(IPLContext ctx, IPLAudioSettings audio_cfg, IPLSceneSettings scene_cfg) {
	IPLSimulationSettings sim_cfg{};
	sim_cfg.flags = static_cast<IPLSimulationFlags>(IPL_SIMULATIONFLAGS_DIRECT | IPL_SIMULATIONFLAGS_REFLECTIONS);
	sim_cfg.sceneType = scene_cfg.type;
	sim_cfg.frameSize = audio_cfg.frameSize;
	sim_cfg.samplingRate = audio_cfg.samplingRate;
	// TODO: make configurable
	sim_cfg.reflectionType = IPL_REFLECTIONEFFECTTYPE_CONVOLUTION;
	sim_cfg.maxNumOcclusionSamples = SteamAudioConfig::max_num_occ_samples;
	sim_cfg.maxNumRays = SteamAudioConfig::max_num_refl_rays;
	sim_cfg.numDiffuseSamples = SteamAudioConfig::num_diffuse_samples;
	sim_cfg.maxDuration = SteamAudioConfig::max_refl_duration;
	sim_cfg.maxOrder = SteamAudioConfig::max_ambisonics_order;
	sim_cfg.maxNumSources = SteamAudioConfig::max_num_refl_srcs;
	sim_cfg.numThreads = SteamAudioConfig::num_refl_threads;

	IPLSimulator sim;
	IPLerror err = iplSimulatorCreate(ctx, &sim_cfg, &sim);
	handleErr(err);
	return sim;
}

IPLSceneSettings create_scene_cfg(IPLContext ctx) {
	IPLSceneSettings scene_cfg = {};
	scene_cfg.type = SteamAudioConfig::scene_type;
	if (scene_cfg.type == IPL_SCENETYPE_EMBREE) {
		IPLEmbreeDeviceSettings embree_cfg{};
		if (embree_dev == nullptr) {
			IPLerror err = iplEmbreeDeviceCreate(ctx, &embree_cfg, &embree_dev);
			handleErr(err);
		}
		scene_cfg.embreeDevice = embree_dev;
	}
	return scene_cfg;
}

IPLContext create_ctx() {
	IPLContextSettings ctx_cfg{};
	ctx_cfg.version = STEAMAUDIO_VERSION;
	IPLContext ctx;
	IPLerror err = iplContextCreate(&ctx_cfg, &ctx);
	handleErr(err);
	return ctx;
}
