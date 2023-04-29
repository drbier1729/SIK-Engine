if garage_state.HasWreckingBallUpgrade() then
	if is_clicked_and_released then
		is_clicked_and_released = false
		PlayDisabledClickSound()
	end
	return
end

local upgrade_cost = 20
local resource_1 = 0

if is_clicked_and_released then
	is_clicked_and_released = false
	if player_object.GetCollectableCount(resource_1) >= upgrade_cost then
		player_object.RemoveCollectables(resource_1, upgrade_cost)
		ChangeTexture("wb_button_unlocked.png")
		ChangeAltTexture("wb_button_high_unlocked.png")
		PlayClickSound()
		garage_state.UpgradeWreckingBall()
	else
		PlayDisabledClickSound()
	end
end