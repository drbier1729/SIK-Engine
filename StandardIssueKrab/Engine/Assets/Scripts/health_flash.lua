timer = timer - dt
if (timer < 0) then
	timer  = 0
end

if (player_object.TookDamageLastFrame()) then
	-- The flash timer for the health icon in seconds
	timer = 0.2
end

if (timer > 0) then
	ChangeTexture("Heart_flash.png")
	SetGUIScaleX(1.5)
	SetGUIScaleY(1.5)
else
	ChangeTexture("Heart.png")
	SetGUIScaleX(1)
	SetGUIScaleY(1)
end