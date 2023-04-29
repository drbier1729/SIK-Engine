-- number of boxes destroyed
local boxes_destroyed = gameplay_state.max_destructibles - gameplay_state.GetDestructibleCount()

-- resources information
if (boxes_destroyed > 15 and timer < 10) then
	timer = timer + dt
	EnableRender()
end

if (boxes_destroyed > 25 or timer > 10) then
	DisableRender()
end