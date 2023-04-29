--Car moved threshold in seconds
local car_moved_threshold = 12

--Car drifted threshold in seconds
local car_drifted_threshold = 12

-- initial instructions: car movement and drifting
if (tutorial_state.GetCarAmountMoved() > car_moved_threshold) then
	if (tutorial_state.GetCarAmountDrifted() < car_drifted_threshold and timer < 8) then
		timer = timer + dt
		EnableRender()
	end

	if (tutorial_state.GetCarAmountDrifted() > car_drifted_threshold or timer > 8) then
		DisableRender()
	end
end

-- set appropriate textures
if (AreControllersConnected()) then
	ChangeTexture("instructions_drift_controller.png")
else
	ChangeTexture("instructions_drift_keyboard.png")
end