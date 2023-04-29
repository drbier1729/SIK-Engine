curr_small_car_count = gameplay_state.GetSmallCarCount()
text_string =  (gameplay_state.max_small_cars - curr_small_car_count).."/"..gameplay_state.max_small_cars

SetText(text_string)