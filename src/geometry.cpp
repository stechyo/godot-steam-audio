#include "geometry.hpp"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/mesh.hpp"
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "phonon.h"
#include "server.hpp"
#include "steam_audio.hpp"

void SteamAudioGeometry::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_material"), &SteamAudioGeometry::get_material);
	ClassDB::bind_method(D_METHOD("set_material", "p_material"), &SteamAudioGeometry::set_material);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "SteamAudioMaterial"), "set_material", "get_material");
}

SteamAudioGeometry::SteamAudioGeometry() {}

SteamAudioGeometry::~SteamAudioGeometry() {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	for (auto mesh : meshes) {
		iplStaticMeshRemove(mesh, SteamAudioServer::get_singleton()->get_global_state()->scene);
	}
	iplSceneCommit(SteamAudioServer::get_singleton()->get_global_state()->scene);
}

void SteamAudioGeometry::_ready() {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	Ref<Mesh> mesh;

	auto mesh_inst = get_node<MeshInstance3D>("..");
	if (mesh_inst == nullptr) {
		UtilityFunctions::push_error("SteamAudioGeometry must have a parent MeshInstance3D.");
		return;
	}

	mesh = mesh_inst->get_mesh();
	Transform3D trf = mesh_inst->get_global_transform();

	IPLMaterial material;
	if (mat == nullptr) {
		material = IPLMaterial{ { 0.f, 0.f, 0.f }, 0.f, { 0.f, 0.f, 0.f } };
	} else {
		material = mat->get_material();
	}

	// TODO: mesh scale? not sure how it's handled.
	for (int i = 0; i < mesh->get_surface_count(); i++) {
		Array dat = mesh->surface_get_arrays(i);
		Array verts = dat[Mesh::ARRAY_VERTEX];
		Array tris = dat[Mesh::ARRAY_INDEX];

		IPLVector3 ipl_verts[verts.size()];
		IPLTriangle ipl_tris[tris.size() / 3];
		IPLint32 ipl_mat_indices[tris.size() / 3];

		for (int j = 0; j < verts.size(); j++) {
			Vector3 vert = verts[j];
			vert = trf.basis.xform(vert);
			vert += trf.origin;
			ipl_verts[j] = ipl_vec3_from(vert);
		}

		for (int j = 0; j < tris.size(); j += 3) {
			// godot tris are cw, ipl tris are ccw
			ipl_tris[j / 3].indices[0] = tris[j];
			ipl_tris[j / 3].indices[1] = tris[j + 2];
			ipl_tris[j / 3].indices[2] = tris[j + 1];
			ipl_mat_indices[j / 3] = 0;
		}

		IPLStaticMeshSettings static_mesh_cfg{};
		static_mesh_cfg.numVertices = int(verts.size());
		static_mesh_cfg.numTriangles = int(tris.size() / 3);
		static_mesh_cfg.numMaterials = 1;
		static_mesh_cfg.vertices = ipl_verts;
		static_mesh_cfg.triangles = ipl_tris;
		static_mesh_cfg.materialIndices = ipl_mat_indices;
		static_mesh_cfg.materials = &material;
		IPLStaticMesh ipl_mesh;
		iplStaticMeshCreate(SteamAudioServer::get_singleton()->get_global_state()->scene, &static_mesh_cfg, &ipl_mesh);

		meshes.push_back(ipl_mesh);
	}

	for (IPLStaticMesh m : meshes) {
		iplStaticMeshAdd(m, SteamAudioServer::get_singleton()->get_global_state()->scene);
	}
}

PackedStringArray SteamAudioGeometry::_get_configuration_warnings() const {
	PackedStringArray res;
	auto mesh_inst = get_node<MeshInstance3D>("..");
	if (mesh_inst == nullptr) {
		res.push_back("The parent of SteamAudioGeometry must be a MeshInstance3D.");
	}
	return res;
}

Ref<SteamAudioMaterial> SteamAudioGeometry::get_material() { return mat; }
void SteamAudioGeometry::set_material(Ref<SteamAudioMaterial> p_material) { mat = p_material; }
