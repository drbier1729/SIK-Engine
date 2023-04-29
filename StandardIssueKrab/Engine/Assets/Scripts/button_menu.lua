if is_clicked_and_released then
	is_clicked_and_released = false
	PlayClickSound()
	StateManager.PopState()
	start_state.PushStart()
end