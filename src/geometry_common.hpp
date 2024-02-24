#ifndef STEAM_AUDIO_GEOMETRY_COMMON_H
#define STEAM_AUDIO_GEOMETRY_COMMON_H

#include "godot_cpp/classes/array_mesh.hpp"
#include "godot_cpp/classes/box_mesh.hpp"
#include "godot_cpp/classes/box_shape3d.hpp"
#include "godot_cpp/classes/capsule_mesh.hpp"
#include "godot_cpp/classes/capsule_shape3d.hpp"
#include "godot_cpp/classes/collision_shape3d.hpp"
#include "godot_cpp/classes/concave_polygon_shape3d.hpp"
#include "godot_cpp/classes/cylinder_mesh.hpp"
#include "godot_cpp/classes/cylinder_shape3d.hpp"
#include "godot_cpp/classes/mesh.hpp"
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/sphere_mesh.hpp"
#include "godot_cpp/classes/sphere_shape3d.hpp"
#include "material.hpp"
#include "phonon.h"
#include "steam_audio.hpp"
#include <vector>
using namespace godot;

inline IPLStaticMesh godot_mesh_to_ipl_mesh(Ref<Mesh> mesh, IPLScene scene, IPLMaterial material, Transform3D trf, int surface_idx) {
	Array dat = mesh->surface_get_arrays(surface_idx);
	Array verts = dat[Mesh::ARRAY_VERTEX];
	Array tris = dat[Mesh::ARRAY_INDEX];

	std::vector<IPLVector3> ipl_verts(verts.size());
	std::vector<IPLTriangle> ipl_tris(tris.size() / 3);
	std::vector<IPLint32> ipl_mat_indices(tris.size() / 3);

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

	IPLMaterial mats[1] = { material };
	IPLStaticMeshSettings static_mesh_cfg{};
	static_mesh_cfg.numVertices = int(verts.size());
	static_mesh_cfg.numTriangles = int(tris.size() / 3);
	static_mesh_cfg.numMaterials = 1;
	static_mesh_cfg.vertices = ipl_verts.data();
	static_mesh_cfg.triangles = ipl_tris.data();
	static_mesh_cfg.materialIndices = ipl_mat_indices.data();
	static_mesh_cfg.materials = mats;

	IPLStaticMesh ipl_mesh;
	iplStaticMeshCreate(scene, &static_mesh_cfg, &ipl_mesh);

	return ipl_mesh;
}

inline std::vector<IPLStaticMesh> create_meshes_from_mesh_inst_3d(MeshInstance3D *mesh_inst, IPLScene scene, Ref<SteamAudioMaterial> mat, bool ignore_trf = false) {
	std::vector<IPLStaticMesh> p_meshes;
	Ref<Mesh> mesh = mesh_inst->get_mesh();

	Transform3D trf;
	if (ignore_trf) {
		trf = Transform3D(Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), Vector3(0, 0, 0));
	} else {
		trf = mesh_inst->get_global_transform();
	}

	IPLMaterial material;
	if (mat == nullptr) {
		material = IPLMaterial{ { 0.f, 0.f, 0.f }, 0.f, { 0.f, 0.f, 0.f } };
	} else {
		material = mat->get_material();
	}

	for (int i = 0; i < mesh->get_surface_count(); i++) {
		auto ipl_mesh = godot_mesh_to_ipl_mesh(mesh, scene, material, trf, i);
		p_meshes.push_back(ipl_mesh);
	}

	return p_meshes;
}

inline std::vector<IPLStaticMesh> create_meshes_from_coll_inst_3d(CollisionShape3D *coll_inst, IPLScene scene, Ref<SteamAudioMaterial> mat, bool ignore_trf = false) {
	std::vector<IPLStaticMesh> p_meshes;
	Transform3D trf;
	if (ignore_trf) {
		trf = Transform3D(Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), Vector3(0, 0, 0));
	} else {
		trf = coll_inst->get_global_transform();
	}

	Ref<Mesh> mesh;

	IPLMaterial material;
	if (mat == nullptr) {
		material = IPLMaterial{ { 0.f, 0.f, 0.f }, 0.f, { 0.f, 0.f, 0.f } };
	} else {
		material = mat->get_material();
	}

	if (Object::cast_to<BoxShape3D>(coll_inst->get_shape().ptr())) {
		Ref<BoxShape3D> shape = coll_inst->get_shape();
		Ref<BoxMesh> box_mesh;
		box_mesh.instantiate();
		box_mesh->set_size(shape->get_size());
		mesh = box_mesh;
	} else if (Object::cast_to<CylinderShape3D>(coll_inst->get_shape().ptr())) {
		Ref<CylinderShape3D> shape = coll_inst->get_shape();
		Ref<CylinderMesh> cyl_mesh;
		cyl_mesh.instantiate();
		cyl_mesh->set_radial_segments(4);
		cyl_mesh->set_rings(4);
		cyl_mesh->set_bottom_radius(shape->get_radius());
		cyl_mesh->set_top_radius(shape->get_radius());
		cyl_mesh->set_height(shape->get_height());
		mesh = cyl_mesh;
	} else if (Object::cast_to<CapsuleShape3D>(coll_inst->get_shape().ptr())) {
		Ref<CapsuleShape3D> shape = coll_inst->get_shape();
		Ref<CapsuleMesh> cap_mesh;
		cap_mesh.instantiate();
		cap_mesh->set_radial_segments(4);
		cap_mesh->set_rings(4);
		cap_mesh->set_radius(shape->get_radius());
		cap_mesh->set_height(shape->get_height());
		mesh = cap_mesh;
	} else if (Object::cast_to<SphereShape3D>(coll_inst->get_shape().ptr())) {
		Ref<SphereShape3D> shape = coll_inst->get_shape();
		Ref<SphereMesh> sph_mesh;
		sph_mesh.instantiate();
		sph_mesh->set_radial_segments(4);
		sph_mesh->set_rings(4);
		sph_mesh->set_radius(shape->get_radius());
		sph_mesh->set_height(shape->get_radius() * 2);
		mesh = sph_mesh;
	} else if (Object::cast_to<ConcavePolygonShape3D>(coll_inst->get_shape().ptr())) {
		Ref<ConcavePolygonShape3D> concave = coll_inst->get_shape();
		Ref<ArrayMesh> arr_mesh;
		arr_mesh.instantiate();
		auto faces = concave->get_faces();

		PackedInt32Array tris;
		tris.resize(faces.size());
		for (int i = 0; i < faces.size(); i++) {
			tris[i] = i;
		}

		Array surface_array;
		surface_array.resize(Mesh::ArrayType::ARRAY_MAX);
		surface_array[Mesh::ArrayType::ARRAY_VERTEX] = faces;
		surface_array[Mesh::ArrayType::ARRAY_INDEX] = tris;

		arr_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, surface_array);
		mesh = arr_mesh;
	} else {
		SteamAudio::log(SteamAudio::log_error, "SteamAudioGeometry supports sphere, box, cylinder, capsule and concave polygon shapes. Something else was provided, so this geometry will not do anything.");
		return p_meshes;
	}

	for (int i = 0; i < mesh->get_surface_count(); i++) {
		auto ipl_mesh = godot_mesh_to_ipl_mesh(mesh, scene, material, trf, i);
		p_meshes.push_back(ipl_mesh);
	}
	mesh.unref();
	return p_meshes;
}

#endif // STEAM_AUDIO_GEOMETRY_COMMON_H
