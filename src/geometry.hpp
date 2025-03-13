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

	void create_geometry();
	void destroy_geometry();
	void register_geometry();
	void unregister_geometry();

	void ready_internal();

protected:
	static void _bind_methods();

public:
	std::atomic<bool> created;
	std::atomic<bool> registered;
	bool disabled = false;

	SteamAudioGeometry();
	~SteamAudioGeometry();
	void _notification(int p_what);

	void recalculate();
	Ref<SteamAudioMaterial> get_material();
	void set_material(Ref<SteamAudioMaterial> p_material);
	bool is_disabled() const { return disabled; }
	void set_disabled(bool p_disabled);

	PackedStringArray _get_configuration_warnings() const override;
};

#endif // STEAM_AUDIO_GEOMETRY_H
