--Car moved threshold in seconds
local car_moved_threshold = 10
timer = timer + dt


if (timer > 2) then
	-- initial instructions: car movement and drifting
	if (tutorial_state.GetCarAmountMoved() < car_moved_threshold) then
		EnableRender()
	else
		DisableRender()
	end
else
	DisableRender()
end

-- set appropriate textures
if (AreControllersConnected()) then
	ChangeTexture("instructions_movement_controller.png")
else
	ChangeTexture("instructions_movement_keyboard.png")
end