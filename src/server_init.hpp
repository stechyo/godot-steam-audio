#include "godot_cpp/classes/project_settings.hpp"
#include <phonon.h>

using namespace godot;

IPLAudioSettings create_audio_cfg();
void create_scene(IPLContext ctx, IPLSceneSettings *scene_cfg, IPLScene *scene);
IPLHRTF create_hrtf(IPLContext ctx, IPLAudioSettings audio_cfg);
IPLAmbisonicsDecodeEffect create_ambisonics_decode_effect(IPLContext ctx, IPLAudioSettings audio_cfg, IPLHRTF hrtf);
IPLAmbisonicsEncodeEffect create_ambisonics_encode_effect(IPLContext ctx, IPLAudioSettings audio_cfg);
IPLSimulator create_simulator(IPLContext ctx, IPLAudioSettings audio_cfg, IPLSceneSettings scene_cfg);
IPLSceneSettings create_scene_cfg(IPLContext ctx);
IPLContext create_ctx();
