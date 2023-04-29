SIK_INFO = 2

if (PositionX() > 50) then
	speed = -10
elseif (PositionX() < -50) then
	speed = 10
end


SetPosition(PositionX() + (speed * dt), PositionY() + (0.0 * dt), PositionZ() + (0.0 * dt))	
