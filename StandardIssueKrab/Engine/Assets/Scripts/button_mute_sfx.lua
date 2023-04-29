is_mute = IsSfxMute()

if is_clicked_and_released then
	if is_mute then
		UnMuteSfx()
		ChangeTexture("sfx_button.png")
		ChangeAltTexture("sfx_button_high.png")
	else
		ChangeTexture("sfx_button_muted.png")
		ChangeAltTexture("sfx_button_high_muted.png")
		MuteSfx()
	end
end