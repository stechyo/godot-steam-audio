extends MeshInstance3D

@onready var audio_geometry: SteamAudioGeometry = $SteamAudioGeometry
@onready var col: CollisionShape3D = $StaticBody3D/CollisionShape3D

@onready var _door_start_pos: Vector3 = position


func _process(_delta):
	if Input.is_action_just_pressed("toggle_door"):
		if audio_geometry.disabled:
			audio_geometry.disabled = false
			col.disabled = false
			show()
		else:
			audio_geometry.disabled = true
			col.disabled = true
			hide()

	if Input.is_action_just_pressed("move_door"):
		if position == _door_start_pos:
			position.z += 1.0
		else:
			position = _door_start_pos
		audio_geometry.recalculate()
