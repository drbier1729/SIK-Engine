if not garage_state.CompletedConstruction() then
	if is_clicked_and_released then
		is_clicked_and_released = false
		PlayDisabledClickSound()
	end
	return
else
	ChangeTexture("musclebeach_button.png")
	ChangeAltTexture("musclebeach_button_high.png")
end

if is_clicked_and_released then
	is_clicked_and_released = false
	PlayClickSound()
	garage_state.EnterMuscleArena()
end