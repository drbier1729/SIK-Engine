mouse_button_left = 1

SIK_INFO = 2

if IsMouseButtonTriggered(mouse_button_left) then
	ScriptLog(SIK_INFO, "info from the script: left mouse button triggered")
	dt_str = "DT is "..dt
	ScriptLog(SIK_INFO, dt_str)
	
end