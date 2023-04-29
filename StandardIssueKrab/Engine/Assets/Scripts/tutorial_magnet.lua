DisableRender()

-- upgrade information
if (garage_state.HasMagnetUpgrade() and timer < 4) then
	timer = timer + dt
	EnableRender()
end

if (timer > 4) then
	DisableRender()
end

-- set appropriate texture
if (AreControllersConnected()) then
	ChangeTexture("instructions_magnet_controller.png")
else
	ChangeTexture("instructions_magnet_keyboard.png")
end