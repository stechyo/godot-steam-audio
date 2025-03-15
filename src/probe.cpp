#include <godot_cpp/core/class_db.hpp>

#include "probe.hpp"
#include "server.hpp"

#include "phonon.h"

using namespace godot;

void SteamAudioProbe::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_radius", "radius"), &SteamAudioProbe::set_radius);
    ClassDB::bind_method(D_METHOD("get_radius"), &SteamAudioProbe::get_radius);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius", "get_radius");
}

SteamAudioProbe::SteamAudioProbe() {
    radius = 1.0f;
    mesh = memnew(MeshInstance3D);
    sphere.instantiate();
    mesh->set_name("mesh");
    mesh->set_mesh(sphere);
    sphere->set_radius(radius);
    add_child(mesh);
}

SteamAudioProbe::~SteamAudioProbe() {
}

void SteamAudioProbe::set_radius(float p_radius) {
    radius = p_radius;
    sphere->set_radius(radius);
}

float SteamAudioProbe::get_radius() const {
    return radius;
}
