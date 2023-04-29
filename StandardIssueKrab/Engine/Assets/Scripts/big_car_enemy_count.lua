curr_big_car_count = gameplay_state.GetBigCarCount()
text_string =  (gameplay_state.max_big_cars - curr_big_car_count).."/"..gameplay_state.max_big_cars

SetText(text_string)