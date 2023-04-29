curr_destructible_count = gameplay_state.GetDestructibleCount()
text_string =  (gameplay_state.max_destructibles - curr_destructible_count).."/"..gameplay_state.max_destructibles

SetText(text_string)