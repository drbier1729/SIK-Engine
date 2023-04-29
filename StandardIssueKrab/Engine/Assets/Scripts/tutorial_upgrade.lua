local upgrade_cost = 20
local resource_1 = 0

DisableRender()

-- upgrade information
if ((not garage_state.HasWreckingBallUpgrade()) and
	player_object.GetCollectableCount(resource_1) >= upgrade_cost and
	timer < 10) then
	timer = timer + dt
	EnableRender()
end

if (timer > 10) then
	DisableRender()
end