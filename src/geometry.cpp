#include "geometry.hpp"
#include "geometry_common.hpp"
#include "godot_cpp/classes/collision_shape3d.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "phonon.h"
#include "server.hpp"

void SteamAudioGeometry::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_material"), &SteamAudioGeometry::get_material);
	ClassDB::bind_method(D_METHOD("set_material", "p_material"), &SteamAudioGeometry::set_material);
	ClassDB::bind_method(D_METHOD("is_disabled"), &SteamAudioGeometry::is_disabled);
	ClassDB::bind_method(D_METHOD("set_disabled", "p_disabled"), &SteamAudioGeometry::set_disabled);
	ClassDB::bind_method(D_METHOD("recalculate"), &SteamAudioGeometry::recalculate);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "disabled"), "set_disabled", "is_disabled");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "SteamAudioMaterial"), "set_material", "get_material");
}

SteamAudioGeometry::SteamAudioGeometry() {}

SteamAudioGeometry::~SteamAudioGeometry() {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	unregister_geometry();
	destroy_geometry();
}

void SteamAudioGeometry::_ready() {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	create_geometry();
	register_geometry();
}

void SteamAudioGeometry::set_disabled(bool p_disabled) {
	if (disabled == p_disabled) {
		return;
	}

	if (p_disabled) {
		unregister_geometry();
	} else {
		register_geometry();
	}

	disabled = p_disabled;
}

void SteamAudioGeometry::recalculate() {
	unregister_geometry();
	destroy_geometry();
	create_geometry();
	register_geometry();
}

void SteamAudioGeometry::create_geometry() {
	// FIXME: we probably don't even have a global state yet
	if (Object::cast_to<MeshInstance3D>(get_parent())) {
		meshes = create_meshes_from_mesh_inst_3d(
				Object::cast_to<MeshInstance3D>(get_parent()),
				SteamAudioServer::get_singleton()->get_global_state()->scene, mat);
	} else if (Object::cast_to<CollisionShape3D>(get_parent())) {
		meshes = create_meshes_from_coll_inst_3d(
				Object::cast_to<CollisionShape3D>(get_parent()),
				SteamAudioServer::get_singleton()->get_global_state()->scene, mat);
	} else {
		UtilityFunctions::push_error("The parent of SteamAudioGeometry must be a MeshInstance3D or a CollisionShape3D.");
		return;
	}
}

void SteamAudioGeometry::destroy_geometry() {
	for (auto mesh : meshes) {
		iplStaticMeshRelease(&mesh);
	}
	meshes.clear();
}

void SteamAudioGeometry::register_geometry() {
	for (auto mesh : meshes) {
		SteamAudioServer::get_singleton()->add_static_mesh(mesh);
	}
}

void SteamAudioGeometry::unregister_geometry() {
	for (auto ipl_mesh : meshes) {
		SteamAudioServer::get_singleton()->remove_static_mesh(ipl_mesh);
	}
}

PackedStringArray SteamAudioGeometry::_get_configuration_warnings() const {
	PackedStringArray res;
	if (!Object::cast_to<MeshInstance3D>(get_parent()) && !Object::cast_to<CollisionShape3D>(get_parent())) {
		res.push_back("The parent of SteamAudioGeometry must be a MeshInstance3D or a CollisionShape3D.");
	}
	return res;
}

Ref<SteamAudioMaterial> SteamAudioGeometry::get_material() { return mat; }
void SteamAudioGeometry::set_material(Ref<SteamAudioMaterial> p_material) { mat = p_material; }
