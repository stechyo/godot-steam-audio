#include "reflection_baker.hpp"
#include "server.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "steam_audio.hpp"

using namespace godot;

void SteamAudioReflectionBaker::_bind_methods() {
    // Bind methods
    ClassDB::bind_method(D_METHOD("set_baked_data", "data"), &SteamAudioReflectionBaker::set_baked_data);
    ClassDB::bind_method(D_METHOD("get_baked_data"), &SteamAudioReflectionBaker::get_baked_data);
    ClassDB::bind_method(D_METHOD("start_bake"), &SteamAudioReflectionBaker::start_bake);
    ClassDB::bind_method(D_METHOD("get_is_baking"), &SteamAudioReflectionBaker::get_is_baking);
    ClassDB::bind_method(D_METHOD("get_bake_progress"), &SteamAudioReflectionBaker::get_bake_progress);
    ClassDB::bind_method(D_METHOD("set_num_rays", "num_rays"), &SteamAudioReflectionBaker::set_num_rays);
    ClassDB::bind_method(D_METHOD("get_num_rays"), &SteamAudioReflectionBaker::get_num_rays);
    ClassDB::bind_method(D_METHOD("set_num_bounces", "num_bounces"), &SteamAudioReflectionBaker::set_num_bounces);
    ClassDB::bind_method(D_METHOD("get_num_bounces"), &SteamAudioReflectionBaker::get_num_bounces);
    ClassDB::bind_method(D_METHOD("set_duration", "duration"), &SteamAudioReflectionBaker::set_duration);
    ClassDB::bind_method(D_METHOD("get_duration"), &SteamAudioReflectionBaker::get_duration);
    ClassDB::bind_method(D_METHOD("set_num_diffuse_samples", "num_diffuse_samples"), &SteamAudioReflectionBaker::set_num_diffuse_samples);
    ClassDB::bind_method(D_METHOD("get_num_diffuse_samples"), &SteamAudioReflectionBaker::get_num_diffuse_samples);
    ClassDB::bind_method(D_METHOD("set_irradiance_min_distance", "irradiance_min_distance"), &SteamAudioReflectionBaker::set_irradiance_min_distance);
    ClassDB::bind_method(D_METHOD("get_irradiance_min_distance"), &SteamAudioReflectionBaker::get_irradiance_min_distance);
    ClassDB::bind_method(D_METHOD("set_bake_parametric", "bake_parametric"), &SteamAudioReflectionBaker::set_bake_parametric);
    ClassDB::bind_method(D_METHOD("get_bake_parametric"), &SteamAudioReflectionBaker::get_bake_parametric);
    ClassDB::bind_method(D_METHOD("set_bake_convolution", "bake_convolution"), &SteamAudioReflectionBaker::set_bake_convolution);
    ClassDB::bind_method(D_METHOD("get_bake_convolution"), &SteamAudioReflectionBaker::get_bake_convolution);
    ClassDB::bind_method(D_METHOD("set_center", "center"), &SteamAudioReflectionBaker::set_center);
    ClassDB::bind_method(D_METHOD("get_center"), &SteamAudioReflectionBaker::get_center);
    ClassDB::bind_method(D_METHOD("set_radius", "radius"), &SteamAudioReflectionBaker::set_radius);
    ClassDB::bind_method(D_METHOD("get_radius"), &SteamAudioReflectionBaker::get_radius);
    ClassDB::bind_method(D_METHOD("set_probe_spacing", "probe_spacing"), &SteamAudioReflectionBaker::set_probe_spacing);
    ClassDB::bind_method(D_METHOD("get_probe_spacing"), &SteamAudioReflectionBaker::get_probe_spacing);
    ClassDB::bind_method(D_METHOD("set_probe_height", "probe_height"), &SteamAudioReflectionBaker::set_probe_height);
    ClassDB::bind_method(D_METHOD("get_probe_height"), &SteamAudioReflectionBaker::get_probe_height);
    ClassDB::bind_method(D_METHOD("set_probe_generation_extents", "probe_generation_extents"), &SteamAudioReflectionBaker::set_probe_generation_extents);
    ClassDB::bind_method(D_METHOD("get_probe_generation_extents"), &SteamAudioReflectionBaker::get_probe_generation_extents);

    // Add properties
    ADD_GROUP("Reflection Bake", "");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "num_rays"), "set_num_rays", "get_num_rays");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "num_bounces"), "set_num_bounces", "get_num_bounces");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "duration"), "set_duration", "get_duration");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "num_diffuse_samples"), "set_num_diffuse_samples", "get_num_diffuse_samples");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "irradiance_min_distance"), "set_irradiance_min_distance", "get_irradiance_min_distance");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "bake_parametric"), "set_bake_parametric", "get_bake_parametric");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "bake_convolution"), "set_bake_convolution", "get_bake_convolution");
    
    ADD_GROUP("Scene", "");
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "center"), "set_center", "get_center");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius", "get_radius");

    ADD_GROUP("Probes", "");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "probe_spacing"), "set_probe_spacing", "get_probe_spacing");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "probe_height"), "set_probe_height", "get_probe_height");
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "probe_generation_extents"), "set_probe_generation_extents", "get_probe_generation_extents");

    ADD_GROUP("Bake Data", "");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "baked_data", PROPERTY_HINT_RESOURCE_TYPE, "BakedReflectionData"), "set_baked_data", "get_baked_data");
}

SteamAudioReflectionBaker::SteamAudioReflectionBaker() {
    probe_batch = nullptr;
    is_baking = false;
    bake_progress = 0.0f;

    // Initialize properties from ReflectionBakeConfig
    num_rays = 32768;
    num_bounces = 64;
    duration = 2.0f;
    num_diffuse_samples = 1024;
    irradiance_min_distance = 1.0f;
    bake_parametric = true;
    bake_convolution = true;
}

SteamAudioReflectionBaker::~SteamAudioReflectionBaker() {
    if (probe_batch) {
        iplProbeBatchRelease(&probe_batch);
    }
}

void SteamAudioReflectionBaker::generate_probes() {
    float x = probe_generation_extents.x;
    float y = probe_generation_extents.y;
    float z = probe_generation_extents.z;
    IPLMatrix4x4 box_transform = IPLMatrix4x4{ {
        {x,    0.0f, 0.0f, 0.0f},
        {0.0f, y,    0.0f, 0.0f},
        {0.0f, 0.0f, z,    0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    } };

    IPLProbeGenerationParams probe_params{};
    probe_params.type = IPL_PROBEGENERATIONTYPE_UNIFORMFLOOR;
    probe_params.spacing = probe_spacing;
    probe_params.height = probe_height;
    probe_params.transform = box_transform;

    IPLContext context = SteamAudioServer::get_singleton()->get_global_state(false)->ctx;
    IPLScene scene = SteamAudioServer::get_singleton()->get_global_state(false)->scene;

    IPLProbeArray probe_array = nullptr;
    iplProbeArrayCreate(context, &probe_array);
    iplProbeArrayGenerateProbes(probe_array, scene, &probe_params);

    iplProbeBatchCreate(context, &probe_batch);
    iplProbeBatchAddProbeArray(probe_batch, probe_array);

    TypedArray<Node> probes = find_children("*", "SteamAudioProbe", true, false);
    for (int i = 0; i < probes.size(); i++) {
        SteamAudioProbe *p = Object::cast_to<SteamAudioProbe>(probes[i]);
        IPLSphere sphere{ipl_vec3_from(p->get_global_position()), p->get_radius()};
        iplProbeBatchAddProbe(probe_batch, sphere);
    }

    iplProbeBatchCommit(probe_batch);
}

void SteamAudioReflectionBaker::set_baked_data(const Ref<SteamAudioBakedReflectionData>& p_data) {
}

Ref<SteamAudioBakedReflectionData> SteamAudioReflectionBaker::get_baked_data() const {
    return nullptr;
}

IPLReflectionsBakeParams SteamAudioReflectionBaker::get_bake_params() const {
    IPLBakedDataIdentifier identifier{};
    identifier.type = IPL_BAKEDDATATYPE_REFLECTIONS;
    identifier.variation = IPL_BAKEDDATAVARIATION_DYNAMIC;
    identifier.endpointInfluence.center = ipl_vec3_from(center); // world-space position of the source 
    identifier.endpointInfluence.radius = radius; // only bake reflections for probes within 100m of the source


    IPLReflectionsBakeParams params{};
    params.scene = SteamAudioServer::get_singleton()->get_global_state(false)->scene;
    params.probeBatch; // filled out by bake manager

    params.sceneType = IPL_SCENETYPE_DEFAULT;
    params.identifier = identifier;
    params.savedDuration = 2.0f;
    params.order = 2;
    params.numThreads = 8;

    params.numRays = num_rays;
    params.numBounces = num_bounces;
    params.numDiffuseSamples = num_diffuse_samples;
    params.irradianceMinDistance = irradiance_min_distance;
    if (bake_parametric) {
        params.bakeFlags = static_cast<IPLReflectionsBakeFlags>(
            static_cast<int>(params.bakeFlags) | IPL_REFLECTIONSBAKEFLAGS_BAKEPARAMETRIC
        );
    }
    if (bake_convolution) {
        params.bakeFlags = static_cast<IPLReflectionsBakeFlags>(
            static_cast<int>(params.bakeFlags) | IPL_REFLECTIONSBAKEFLAGS_BAKECONVOLUTION
        );
    }
    return params;
}

void SteamAudioReflectionBaker::start_bake() {
    if (is_baking) {
        return;
    }

    is_baking = true;

    generate_probes();
    IPLContext context = SteamAudioServer::get_singleton()->get_global_state(false)->ctx;
    IPLReflectionsBakeParams params = get_bake_params();
    iplReflectionsBakerBake(context, &params, nullptr, nullptr);

    is_baking = false;
}

void SteamAudioReflectionBaker::bake_progress_callback(float progress, void* user_data) {
    SteamAudioReflectionBaker* manager = static_cast<SteamAudioReflectionBaker*>(user_data);
    manager->bake_progress = progress;
    
    // Print progress every 10%
    static int last_percentage = -1;
    int current_percentage = static_cast<int>(progress * 100);
    if (current_percentage / 10 != last_percentage / 10) {
        last_percentage = current_percentage;
        UtilityFunctions::print("[Steam Audio] Baking reflections: ", String::num_int64(current_percentage), "%");
    }
}

bool SteamAudioReflectionBaker::get_is_baking() const {return is_baking;}
float SteamAudioReflectionBaker::get_bake_progress() const {return bake_progress;}
void SteamAudioReflectionBaker::set_num_rays(int p_num_rays) { num_rays = p_num_rays; }
int SteamAudioReflectionBaker::get_num_rays() const { return num_rays; }
void SteamAudioReflectionBaker::set_num_bounces(int p_num_bounces) { num_bounces = p_num_bounces; }
int SteamAudioReflectionBaker::get_num_bounces() const { return num_bounces; }
void SteamAudioReflectionBaker::set_duration(float p_duration) { duration = p_duration; }
float SteamAudioReflectionBaker::get_duration() const { return duration; }
void SteamAudioReflectionBaker::set_num_diffuse_samples(int p_num_diffuse_samples) { num_diffuse_samples = p_num_diffuse_samples; }
int SteamAudioReflectionBaker::get_num_diffuse_samples() const { return num_diffuse_samples; }
void SteamAudioReflectionBaker::set_irradiance_min_distance(float p_irradiance_min_distance) { irradiance_min_distance = p_irradiance_min_distance; }
float SteamAudioReflectionBaker::get_irradiance_min_distance() const { return irradiance_min_distance; }
void SteamAudioReflectionBaker::set_bake_parametric(bool p_bake_parametric) { bake_parametric = p_bake_parametric; }
bool SteamAudioReflectionBaker::get_bake_parametric() const { return bake_parametric; }
void SteamAudioReflectionBaker::set_bake_convolution(bool p_bake_convolution) { bake_convolution = p_bake_convolution; }
bool SteamAudioReflectionBaker::get_bake_convolution() const { return bake_convolution; }
void SteamAudioReflectionBaker::set_center(Vector3 p_center) { center = p_center; }
Vector3 SteamAudioReflectionBaker::get_center() const { return center; }
void SteamAudioReflectionBaker::set_radius(float p_radius) { radius = p_radius; }
float SteamAudioReflectionBaker::get_radius() const { return radius; }
void SteamAudioReflectionBaker::set_probe_spacing(float p_probe_spacing) { probe_spacing = p_probe_spacing; }
float SteamAudioReflectionBaker::get_probe_spacing() const { return probe_spacing; }
void SteamAudioReflectionBaker::set_probe_height(float p_probe_height) { probe_height = p_probe_height; }
float SteamAudioReflectionBaker::get_probe_height() const { return probe_height; }
void SteamAudioReflectionBaker::set_probe_generation_extents(Vector3 p_probe_generation_extents) { probe_generation_extents = p_probe_generation_extents; }
Vector3 SteamAudioReflectionBaker::get_probe_generation_extents() const { return probe_generation_extents; }
