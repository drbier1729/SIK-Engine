DisableRender()

if (gameplay_state.IsLastLevel()) then
	ChangeTexture("game_completed.png")
else
	ChangeTexture("arena_unlocked.png")
end

-- Check if we just completed the level
if ((gameplay_state.GetProgress() == gameplay_state.PROGRESS_COMPLETE) and 
	(gameplay_state.GetPreviousProgress() == gameplay_state.PROGRESS_WORKING)) then
	
	timer = timer + dt
	EnableRender()
end

-- Display for only 10 seconds
if (timer > 10) then
	DisableRender()
end