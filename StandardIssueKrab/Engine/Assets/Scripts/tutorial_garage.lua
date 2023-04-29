-- set appropriate textures
if (AreControllersConnected()) then
	ChangeTexture("instructions_garage_controller.png")
else
	ChangeTexture("instructions_garage_keyboard.png")
end