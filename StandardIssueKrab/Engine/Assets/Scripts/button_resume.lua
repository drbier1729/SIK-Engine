if is_clicked_and_released or IsActionTriggered(ACTION_START) or IsActionTriggered(ACTION_2)then
	is_clicked_and_released = false
	PlayClickSound()
	StateManager.PopState()
end