SIK_INFO = 2

--Activate the collectable after 3 seconds

activate_time = activate_time - dt;
--ScriptLog(SIK_INFO, tostring(activate_time))
if activate_time < 0  and activate_time > -0.1 then
	ScriptLog(SIK_INFO, "Collectable activated")
	EnableRigidBody(true)
end

--Spin the collectable
const_rotate = 0.5
RotateOrientation(const_rotate);