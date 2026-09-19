extends Node3D
# Stress godot-steam-audio the way a game uses it, with the REAL audio driver (not headless: headless = dummy driver,
# which mixes on the main thread and hides every audio-thread race). Every frame, at random: create a player and play
# a stream, free one mid-mix, restart one with a new stream, move sources and the listener, toggle occlusion and
# reflection, and add/remove a piece of geometry. Runs SECONDS (default 90) and reports; a crash ends the process.

var players: Array[Node3D] = []
var geoms: Array[Node3D] = []
var streams: Array[AudioStream] = []
var cam: Camera3D
var ops := 0
var t := 0.0
var secs := 90.0
var rng := RandomNumberGenerator.new()
var allowed := "01234567"

func tube_mesh() -> ArrayMesh:
	var st := SurfaceTool.new()
	st.begin(Mesh.PRIMITIVE_TRIANGLES)
	for r in 6:
		for s in 10:
			var a0 := TAU * s / 10.0
			var a1 := TAU * (s + 1) / 10.0
			var p: Array = [Vector3(cos(a0) * 3, sin(a0) * 3, r * 2.0), Vector3(cos(a1) * 3, sin(a1) * 3, r * 2.0),
					Vector3(cos(a1) * 3, sin(a1) * 3, r * 2.0 + 2), Vector3(cos(a0) * 3, sin(a0) * 3, r * 2.0 + 2)]
			for i in [0, 1, 2, 0, 2, 3]:
				st.add_vertex(p[i])
	st.index()
	return st.commit()

func _ready() -> void:
	rng.seed = 1
	if OS.get_environment("SECONDS") != "":
		secs = float(OS.get_environment("SECONDS"))
	if OS.get_environment("OPS") != "":
		allowed = OS.get_environment("OPS")
	print("race: driver %s, %d s, ops %s" % [AudioServer.get_driver_name(), secs, allowed])
	add_child(ClassDB.instantiate("SteamAudioConfig"))
	cam = Camera3D.new()
	add_child(cam)
	cam.add_child(ClassDB.instantiate("SteamAudioListener"))
	for f in ["res://sound/2.mp3", "res://sound/pluck1.mp3", "res://sound/pluck2.mp3"]:
		streams.append(load(f))

func add_player() -> void:
	if OS.get_environment("PLAIN") == "1":
		var q := AudioStreamPlayer3D.new()
		add_child(q)
		q.global_position = Vector3(rng.randf_range(-15, 15), rng.randf_range(-2, 4), rng.randf_range(-15, 15))
		q.stream = streams[rng.randi() % streams.size()]
		q.play(rng.randf_range(0, 1))
		players.append(q)
		return
	var p: Node3D = ClassDB.instantiate("SteamAudioPlayer")
	p.set("occlusion_on", rng.randf() < 0.7)
	p.set("reflection_on", rng.randf() < 0.5)
	add_child(p)
	p.global_position = Vector3(rng.randf_range(-15, 15), rng.randf_range(-2, 4), rng.randf_range(-15, 15))
	p.call("play_stream", streams[rng.randi() % streams.size()], rng.randf_range(0, 1), -20.0, 1.0)
	players.append(p)

func _process(delta: float) -> void:
	t += delta
	for k in 20:
		var op := rng.randi() % 8
		if not str(op) in allowed:
			continue
		match op:
			0, 1:
				if players.size() < 48: add_player()
			2:
				if players.size() > 0:
					var i := rng.randi() % players.size()
					players[i].queue_free()
					players.remove_at(i)
			3:
				if players.size() > 0:
					players[rng.randi() % players.size()].call("play_stream", streams[rng.randi() % streams.size()], 0.0, -20.0, rng.randf_range(0.8, 1.2))
			4:
				if players.size() > 0:
					players[rng.randi() % players.size()].global_position += Vector3(rng.randf_range(-1, 1), 0, rng.randf_range(-1, 1))
			5:
				cam.global_position = Vector3(rng.randf_range(-5, 5), 1.6, rng.randf_range(-5, 5))
				cam.rotation.y = rng.randf_range(-PI, PI)
			6:
				if geoms.size() < 12:
					var mi := MeshInstance3D.new()
					mi.mesh = tube_mesh()
					add_child(mi)
					mi.global_position = Vector3(rng.randf_range(-20, 20), 0, rng.randf_range(-20, 20))
					mi.add_child(ClassDB.instantiate("SteamAudioGeometry"))
					geoms.append(mi)
			7:
				if geoms.size() > 0:
					var j := rng.randi() % geoms.size()
					geoms[j].queue_free()
					geoms.remove_at(j)
		ops += 1
	if int(t) != int(t - delta) and int(t) % 10 == 0:
		print("race: %d s, %d ops, %d players, %d geometry" % [int(t), ops, players.size(), geoms.size()])
	if t > secs:
		print("RACE: survived %d s and %d ops" % [int(t), ops])
		if OS.get_environment("GRACEFUL") == "1":
			set_process(false)
			for q in players:
				(q as AudioStreamPlayer3D).stop()
			for i in 6:
				await get_tree().process_frame
		get_tree().quit(0)
