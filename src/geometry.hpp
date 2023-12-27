#ifndef STEAM_AUDIO_GEOMETRY_H
#define STEAM_AUDIO_GEOMETRY_H

#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "material.hpp"
#include "phonon.h"

using namespace godot;

class SteamAudioGeometry : public Node3D {
	GDCLASS(SteamAudioGeometry, Node3D);

private:
	std::vector<IPLStaticMesh> meshes;
	Ref<SteamAudioMaterial> mat;

protected:
	static void _bind_methods();

public:
	SteamAudioGeometry();
	~SteamAudioGeometry();
	void _ready() override;

	Ref<SteamAudioMaterial> get_material();
	void set_material(Ref<SteamAudioMaterial> p_material);

	PackedStringArray _get_configuration_warnings() const override;
};

#endif // STEAM_AUDIO_GEOMETRY_H
