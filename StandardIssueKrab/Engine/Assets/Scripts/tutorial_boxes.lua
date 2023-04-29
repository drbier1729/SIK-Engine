-- number of boxes destroyed
local boxes_destroyed = gameplay_state.max_destructibles - gameplay_state.GetDestructibleCount()

-- box information
if (boxes_destroyed > 0 and timer < 10) then
	timer = timer + dt
	EnableRender()
end

if (boxes_destroyed > 15 or timer > 10) then
	DisableRender()
end