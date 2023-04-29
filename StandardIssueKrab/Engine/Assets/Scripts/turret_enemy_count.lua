curr_turret_count = gameplay_state.GetTurretCount()
text_string =  (gameplay_state.max_turrets - curr_turret_count).."/"..gameplay_state.max_turrets

SetText(text_string)