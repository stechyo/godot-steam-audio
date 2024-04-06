#ifndef STEAM_AUDIO_DYNAMIC_GEOMETRY_H
#define STEAM_AUDIO_DYNAMIC_GEOMETRY_H

#include "godot_cpp/classes/node3d.hpp"
#include "material.hpp"
#include "phonon.h"
#include <atomic>

using namespace godot;

class SteamAudioDynamicGeometry : public Node3D {
	GDCLASS(SteamAudioDynamicGeometry, Node3D);

private:
	std::atomic_bool is_init;
	IPLInstancedMesh mesh = nullptr;
	IPLScene sub_scene = nullptr;
	std::vector<IPLStaticMesh> meshes;

	Ref<SteamAudioMaterial> mat;

	void create_geometry();
	void destroy_geometry();
	void register_geometry();
	void unregister_geometry();

	void ready_internal();
	void process_internal(double delta);

protected:
	static void _bind_methods();

public:
	SteamAudioDynamicGeometry();
	~SteamAudioDynamicGeometry();
	void _notification(int p_what);

	void recalculate();
	void init_mesh();

	Ref<SteamAudioMaterial> get_material();
	void set_material(Ref<SteamAudioMaterial> p_material);

	PackedStringArray _get_configuration_warnings() const override;
};

#endif // STEAM_AUDIO_DYNAMIC_GEOMETRY_H
