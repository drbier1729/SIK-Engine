#pragma once

/*
* Sound Asset Credits:
* https://kronbits.itch.io/freesfx
*/

#define VOLUME_DIVIDE 100.0f

class AudioManager {

public:

	AudioManager();
	~AudioManager();

	void Update();
	void LoadSound(const char* filepath, Bool is_stream, StringID tag);

	Int32 PlayAudio(StringID tag, FMOD::ChannelGroup* channel_group, Float32 volume, Float32 pitch, Bool is_paused, Int32 loop_count);

	void Pause(FMOD::ChannelGroup* chanel);
	void Unpause(FMOD::ChannelGroup* chanel);

	void Mute(FMOD::ChannelGroup* chanel);
	void UnMute(FMOD::ChannelGroup* chanel);

	Bool IsSfxMute();
	void MuteSfx();
	void UnmuteSfx();

	Bool IsMusicMute();
	void MuteBackMusic();
	void UnMuteBackMusic();

	void MuteAll();
	void UnMuteAll();
	void StopAll();

	Float32 GetVolume(FMOD::ChannelGroup* chanel);
	Float32 GetVolume(Int32 chanel_id);

	void SetVolume(Float32 volume, FMOD::ChannelGroup* chanel_group);
	void SetVolume(Float32 volume, Int32 chanel_id);

	void Stop(Int32 channel_id_to_stop);

	Bool IsPlaying(Int32 channel_id);

	FMOD::ChannelGroup* background_music_chanel_group, 
						* menu_stuff_chanel_group, 
						* sfx_chanel_group;

	Float32 skid_vol, 
			wood_destroy_vol, 
			turret_impact_vol, 
			turret_destroy_vol,
			turret_shoot_vol;

	Float32 construction_arena_back_vol;

private:
	Int32 channel_id;

	FMOD::System* fmod_system;
	FMOD_RESULT result;

	UnorderedMap<StringID, FMOD::Sound*> loaded_audio_files_map;
	typedef Map<Int32, FMOD::Channel*> ChannelsMap;

	ChannelsMap channels_map;
};

//Declared as an extern variable so it can be accessed throughout the project
extern AudioManager* p_audio_manager;
