-- minimum distance from player to turret for prompt to appear
local min_dist_to_turret = 30

-- turret information
if (tutorial_state.GetDistanceToTurret() < min_dist_to_turret and timer < 1) then
	start_timer = true
end

if (start_timer) then
	timer = timer + dt
	EnableRender()
end

if (timer > 8) then
	ChangeTexture("instructions_health.png")
end

if (timer > 16) then
	DisableRender()
end