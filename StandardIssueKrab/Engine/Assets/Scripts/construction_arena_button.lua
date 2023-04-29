if not garage_state.CompletedJunkyard() then
	if is_clicked_and_released then
		is_clicked_and_released = false
		PlayDisabledClickSound()
	end
	return
else
	ChangeTexture("construction_button.png")
	ChangeAltTexture("construction_button_high.png")
end

if is_clicked_and_released then
	is_clicked_and_released = false
	PlayClickSound()
	garage_state.EnterConstructionArena()
end
