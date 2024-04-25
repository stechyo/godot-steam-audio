@tool
# Generates SteamAudioGeometry for imported scenes with hints.
class_name SteamAudioPostImport
extends EditorScenePostImport

var scene_root: Node
var STEAM_AUDIO_GEOM_PREFIX := "-sasg"
var STEAM_AUDIO_DYN_GEOM_PREFIX := "-sadg"

enum GeomType {
	NONE = 0,
	STATIC = 1,
	DYNAMIC = 2
}

func _post_import(scene: Node) -> Node:
	self.scene_root = scene
	iterate(scene)
	return scene


func iterate(n: Node) -> void:
	var typ := steam_audio_geom_type(n)
	if typ != GeomType.NONE:
		var g: Node 
		match typ:
			GeomType.STATIC:
				g = SteamAudioGeometry.new() 
				g.name = "SteamAudioGeometry"
			GeomType.DYNAMIC:
				g = SteamAudioDynamicGeometry.new()
				g.name = "SteamAudioDynamicGeometry"
		g.set_material(preload("res://addons/godot-steam-audio/materials/default_material.tres"))	
		n.add_child(g)
		g.owner = self.scene_root
		
	for child in n.get_children():
		iterate(child)


func steam_audio_geom_type(n: Node) -> GeomType:
	if n is MeshInstance3D:
		if n.name.ends_with(STEAM_AUDIO_GEOM_PREFIX):
			n.name = n.name.substr(0, n.name.length() - 5)
			return GeomType.STATIC
		elif n.name.ends_with(STEAM_AUDIO_DYN_GEOM_PREFIX):
			n.name = n.name.substr(0, n.name.length() - 5)
			return GeomType.DYNAMIC

	elif n is CollisionShape3D:
		# check parent name because -colonly generates plain names
		# for the collision shapes. we do not check the grandparents
		# (for the -col case) because we've already generated a geometry for the mesh instance.
		if n.get_parent().name.ends_with(STEAM_AUDIO_GEOM_PREFIX):
			n.get_parent().name = n.get_parent().name.substr(0, n.get_parent().name.length() - 5)
			return GeomType.STATIC
		elif n.get_parent().name.ends_with(STEAM_AUDIO_DYN_GEOM_PREFIX):
			n.get_parent().name = n.get_parent().name.substr(0, n.get_parent().name.length() - 5)
			return GeomType.DYNAMIC

	return GeomType.NONE

