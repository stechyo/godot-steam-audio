#ifndef GODOT_STEAM_AUDIO_PROBE_HPP
#define GODOT_STEAM_AUDIO_PROBE_HPP

#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/sphere_shape3d.hpp"

#include "phonon.h"

using namespace godot;

class SteamAudioProbe : public Node3D {
    GDCLASS(SteamAudioProbe, Node3D)

private:
    MeshInstance3D *mesh;
    Ref<SphereShape3D> sphere;
    float radius;

protected:
    static void _bind_methods();

public:
    SteamAudioProbe();
    ~SteamAudioProbe();

    void set_radius(float p_radius);
    float get_radius() const;
};

#endif // GODOT_STEAM_AUDIO_PROBE_HPP 
