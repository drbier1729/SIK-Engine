max_hp = player_object.max_health
curr_hp = player_object.GetCurrentHealth()

scale_value = curr_hp/max_hp
SetGUIScaleX(scale_value)

if (scale_value < 0.3) then
	ChangeTexture("Rectangle_Red.png")
elseif (scale_value < 0.6) then
	ChangeTexture("Rectangle_Orange.png")
else
	ChangeTexture("Rectangle_Green.png")
end