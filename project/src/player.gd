extends CharacterBody3D

const max_cam_angle := deg_to_rad(90)
const SPEED = 3.0
const JUMP_VELOCITY = 4.5

# Get the gravity from the project settings to be synced with RigidBody nodes.
var gravity = ProjectSettings.get_setting("physics/3d/default_gravity")
var mDelta: Vector2
@export var cam: Camera3D

func _ready():
		Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)

func _input(evt):
	if evt is InputEventMouseMotion and Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
		mDelta = evt.relative
	
func _physics_process(delta):
	# Add the gravity.
	if not is_on_floor():
		velocity.y -= gravity * delta
		
	# Handle camera motion
	cam.rotation.x -= mDelta.y * delta
	cam.rotation.x = clamp(cam.rotation.x, -max_cam_angle, max_cam_angle)
	rotation.y -= mDelta.x * delta
	mDelta = Vector2()


	# Handle jump.
	if Input.is_action_just_pressed("ui_accept") and is_on_floor():
		velocity.y = JUMP_VELOCITY

	# Get the input direction and handle the movement/deceleration.
	# As good practice, you should replace UI actions with custom gameplay actions.
	var input_dir = Input.get_vector("ui_left", "ui_right", "ui_up", "ui_down")
	var direction = (transform.basis * Vector3(input_dir.x, 0, input_dir.y)).normalized()
	if direction:
		velocity.x = direction.x * SPEED
		velocity.z = direction.z * SPEED
	else:
		velocity.x = move_toward(velocity.x, 0, SPEED)
		velocity.z = move_toward(velocity.z, 0, SPEED)

	move_and_slide()
