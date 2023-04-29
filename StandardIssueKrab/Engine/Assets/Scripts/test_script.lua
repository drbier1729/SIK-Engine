--IsKeyPressed(38)
--IsKeyTriggered(38)
--IsKeyReleased(38)
--IsMouseButtonPressed(1)
--IsMouseButtonTriggered(1)
--IsMouseButtonReleased(1)
--IsControllerButtonPressed(1)
--IsControllerButtonTriggered(1)
--IsControllerButtonReleased(1)
--IsAxisPressedPositive(1)
--IsAxisPressedNegative(1)
--IsAxisTriggeredPositive(1)
--IsAxisTriggeredNegative(1)
--EnableGameObject()
--DisableGameObject()
--QuitGame()

mouse_button_left = 1

SIK_INFO = 2

if IsMouseButtonTriggered(mouse_button_left) then
	ScriptLog(SIK_INFO, "info from the script: left mouse button triggered")
	dt_str = "DT is "..dt
	ScriptLog(SIK_INFO, dt_str)
end