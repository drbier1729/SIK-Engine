if garage_state.HasMagnetUpgrade() then
	if is_clicked_and_released then
		is_clicked_and_released = false
		PlayDisabledClickSound()
	end
	return
end

local upgrade_cost = 500
local resource_1 = 0

if is_clicked_and_released then
	is_clicked_and_released = false
	if player_object.GetCollectableCount(resource_1) >= upgrade_cost then
		player_object.RemoveCollectables(resource_1, upgrade_cost)
		PlayClickSound()
		ChangeTexture("mag_button_unlocked.png")
		ChangeAltTexture("mag_button_high_unlocked.png")
		garage_state.UpgradeMagnet()
	else
		PlayDisabledClickSound()
	end
end