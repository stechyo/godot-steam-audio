extends SteamAudioPlayer

@export var anim: AnimationPlayer

var timeout := 0.0

func _process(delta):
	if !is_playing():
		timeout -= delta 

		if timeout < 0:
			play()
			anim.play("fade")
			timeout = 0.5
