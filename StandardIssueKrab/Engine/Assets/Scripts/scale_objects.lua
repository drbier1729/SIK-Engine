if (ScaleX() > 10) then
	speed = -1
elseif (ScaleX() < 1) then
	speed = 1
end


SetScale(ScaleX() + (speed * dt), ScaleY() + (speed * dt), ScaleZ() + (speed * dt))	
