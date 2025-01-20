#ifndef GODOT_STEAM_AUDIO_REFLECTION_BAKER_HPP
#define GODOT_STEAM_AUDIO_REFLECTION_BAKER_HPP

#include "probe.hpp"
#include "baked_reflection_data.hpp"

#include <godot_cpp/classes/node.hpp>

#include <phonon.h>

using namespace godot;

class SteamAudioBakedReflections : public Node {
    GDCLASS(SteamAudioBakedReflections, Node)

private:
    IPLProbeBatch probe_batch;
    bool is_baking;
    float bake_progress;

    // config
    int num_rays;
    int num_bounces;
    float duration;
    int num_diffuse_samples;
    float irradiance_min_distance;
    bool bake_parametric;
    bool bake_convolution;

    // scene
    Vector3 center;
    float radius;

    // probes
    float probe_spacing;
    float probe_height;
    Vector3 probe_generation_extents;

    // bake data
    Ref<SteamAudioBakedReflectionData> baked_data;

protected:
    static void _bind_methods();

public:
    SteamAudioBakedReflections();
    ~SteamAudioBakedReflections();

    void generate_probes();

    void set_baked_data(const Ref<SteamAudioBakedReflectionData>& p_data);
    Ref<SteamAudioBakedReflectionData> get_baked_data() const;

    void start_bake();
    bool get_is_baking() const;
    float get_bake_progress() const;

    void set_num_rays(int p_num_rays);
    int get_num_rays() const;

    void set_num_bounces(int p_num_bounces);
    int get_num_bounces() const;

    void set_duration(float p_duration);
    float get_duration() const;

    void set_num_diffuse_samples(int p_num_diffuse_samples);
    int get_num_diffuse_samples() const;

    void set_irradiance_min_distance(float p_irradiance_min_distance);
    float get_irradiance_min_distance() const;

    void set_bake_parametric(bool p_bake_parametric);
    bool get_bake_parametric() const;

    void set_bake_convolution(bool p_bake_convolution);
    bool get_bake_convolution() const;

    void set_center(Vector3 p_center);
    Vector3 get_center() const;

    void set_radius(float p_radius);
    float get_radius() const;

    void set_probe_spacing(float p_probe_spacing);
    float get_probe_spacing() const;

    void set_probe_height(float p_probe_height);
    float get_probe_height() const;

    void set_probe_generation_extents(Vector3 p_probe_generation_extents);
    Vector3 get_probe_generation_extents() const;

    IPLReflectionsBakeParams get_bake_params() const;

private:
    static void bake_progress_callback(float progress, void* user_data);
};

#endif // GODOT_STEAM_AUDIO_REFLECTION_BAKER_HPP
