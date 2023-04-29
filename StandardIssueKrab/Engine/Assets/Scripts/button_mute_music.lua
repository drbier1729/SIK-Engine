is_mute = IsMusicMute()

if is_clicked_and_released then
	if is_mute then
		UnMuteBackMusic()
		ChangeTexture("music_button.png")
		ChangeAltTexture("music_button_high.png")
	else
		ChangeTexture("music_button_muted.png")
		ChangeAltTexture("music_button_high_muted.png")
		MuteBackMusic()
	end
end