extends SteamAudioPlayer

@export var anim: AnimationPlayer

func _process(_delta):
	if !is_playing():
		play()
		anim.play("fade")
