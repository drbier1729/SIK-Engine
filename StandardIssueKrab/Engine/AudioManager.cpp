#include "stdafx.h"
#include "AudioManager.h"
#include "ResourceManager.h"

/*
* Sound Asset Credits:
* https://kronbits.itch.io/freesfx
*/

/**
	 * Default constructor
	 * Creating and initializing the FMOD system
*/
AudioManager::AudioManager():
	channel_id{0}
{

	result = FMOD::System_Create(&fmod_system);


	if (result != FMOD_OK)
		SIK_ERROR("Cannot create FMOD system");

	result = fmod_system->init(512, FMOD_INIT_NORMAL, nullptr);

	if (result != FMOD_OK)
		SIK_ERROR("Cannot intialize");

	fmod_system->createChannelGroup("background_chanel", &background_music_chanel_group);
	background_music_chanel_group->setMode(FMOD_CREATECOMPRESSEDSAMPLE | FMOD_LOOP_NORMAL);

	fmod_system->createChannelGroup("music_chanel", &menu_stuff_chanel_group);
	menu_stuff_chanel_group->setMode(FMOD_CREATECOMPRESSEDSAMPLE | FMOD_LOOP_NORMAL);
	
	fmod_system->createChannelGroup("sfx_chanel", &sfx_chanel_group);
	sfx_chanel_group->setMode(FMOD_CREATECOMPRESSEDSAMPLE | FMOD_LOOP_OFF);

	skid_vol = 25.0f;
	wood_destroy_vol = 15.0f;
	turret_impact_vol = 60.0f;
	turret_destroy_vol = 6.0f;
	turret_shoot_vol = 10.0f;

	construction_arena_back_vol = 3.0f;
}

AudioManager::~AudioManager() {

	for (auto& it : loaded_audio_files_map)
	{
		it.second->release();
	}
	fmod_system->close();
	fmod_system->release();
}


void AudioManager::Update() {
	
	std::erase_if(channels_map, [](auto&& c) {
		Bool is_playing = false;
		c.second->isPlaying(&is_playing);
		return !is_playing;
	});

	result = fmod_system->update();

	if (result != FMOD_OK)
		SIK_ERROR("Fmod System not updating");
}

/**
	 * Loads a sound from the Sounds folder
	 * Only reads the audio file and loads into the audio engine
*/
void AudioManager::LoadSound(const char* filepath, Bool is_stream, StringID tag) {

	FMOD::Sound* temp;

	if (is_stream)
		result = fmod_system->createSound(filepath, FMOD_CREATECOMPRESSEDSAMPLE | FMOD_LOOP_NORMAL, nullptr, &temp);
	else
		result = fmod_system->createSound(filepath, FMOD_CREATECOMPRESSEDSAMPLE | FMOD_LOOP_NORMAL, nullptr, &temp);

	if (result != FMOD_OK) {
		SIK_WARN("Sound not loaded \"{}\"", filepath);
		return;
	}

	temp->setLoopCount(-1);
	loaded_audio_files_map[tag] = temp;
}

/**
	* Plays a sound file using FMOD's audio system and returns the channel it uses to play the file.
*/
Int32 AudioManager::PlayAudio(StringID tag, FMOD::ChannelGroup* channel_group, Float32 volume, Float32 pitch, Bool is_paused, Int32 loop_count) {

	FMOD::Channel* temp_channel = nullptr;
	result = fmod_system->playSound(loaded_audio_files_map[tag], channel_group, true, &temp_channel);
	
	if (result != FMOD_OK) {
		SIK_ERROR("Can't play the audio");
		return -1;
	}

	if (channel_id > 100000) {
		channel_id = 0;
	}

	temp_channel->setVolume(volume/VOLUME_DIVIDE);
	temp_channel->setPitch(pitch);
	temp_channel->setMode(FMOD_LOOP_NORMAL);
	temp_channel->setLoopCount(loop_count);
	temp_channel->setPaused(is_paused);


	channels_map[++channel_id] = temp_channel;

	return channel_id;
}

// Pauses a channel group
void AudioManager::Pause(FMOD::ChannelGroup* chanel) {
	chanel->setPaused(true);
}

// Unpauses a channel group
void AudioManager::Unpause(FMOD::ChannelGroup* chanel) {
	chanel->setPaused(false);
}

// Mutes a channel group
void AudioManager::Mute(FMOD::ChannelGroup* chanel) {
	chanel->setMute(true);
}

// Unmutes a channel group
void AudioManager::UnMute(FMOD::ChannelGroup* chanel) {
	chanel->setMute(false);
}

Bool AudioManager::IsSfxMute() {
	Bool temp;
	sfx_chanel_group->getMute(&temp);
	return temp;
}

void AudioManager::MuteSfx() {
	sfx_chanel_group->setMute(true);
}

void AudioManager::UnmuteSfx() {
	sfx_chanel_group->setMute(false);
}

Bool AudioManager::IsMusicMute() {

	Bool temp;
	background_music_chanel_group->getMute(&temp);
	return temp;
}

void AudioManager::MuteBackMusic() {
	background_music_chanel_group->setMute(true);
}

void AudioManager::UnMuteBackMusic() {
	background_music_chanel_group->setMute(false);
}

// Mutes all the channel groups at once
void AudioManager::MuteAll() {
	menu_stuff_chanel_group->setMute(true);
	background_music_chanel_group->setMute(true);
	sfx_chanel_group->setMute(true);
}

// UnMutes all the channel groups at once
void AudioManager::UnMuteAll() {
	menu_stuff_chanel_group->setMute(false);
	background_music_chanel_group->setMute(false);
	sfx_chanel_group->setMute(false);
}

void AudioManager::StopAll() {
	menu_stuff_chanel_group->stop();
	background_music_chanel_group->stop();
	sfx_chanel_group->stop();
}

// Returns the volume of channelgroup between 0 and 100
Float32 AudioManager::GetVolume(FMOD::ChannelGroup* chanel) {
	Float32 volume = 0.0f;
	chanel->getVolume(&volume);
	volume = volume * VOLUME_DIVIDE;
	return volume;
}

// Returns the volume of channel between 0 and 100
Float32 AudioManager::GetVolume(Int32 channel_id)
{
	Float32 volume = 0.0f;
	auto channel = channels_map.find(channel_id);
	if (channel != channels_map.end()) {
		result  = channel->second->getVolume(&volume);
	}
	if (result != FMOD_OK) {
		SIK_ERROR("Cant set volume for the channel group");
	}
	return volume * VOLUME_DIVIDE;
}

// Sets the volume of a channelgroup
void AudioManager::SetVolume(Float32 volume, FMOD::ChannelGroup* chanel_group) {
	result = chanel_group->setVolume(volume / VOLUME_DIVIDE);

	if (result != FMOD_OK) {
		SIK_ERROR("Cant set volume for the channel group");
	}
}

void AudioManager::SetVolume(Float32 volume, Int32 chanel_id) {
	auto channel = channels_map.find(channel_id);

	result = FMOD_RESULT_FORCEINT;
	if (channel != channels_map.end())
		result = channel->second->setVolume(volume / VOLUME_DIVIDE);

}

void AudioManager::Stop(Int32 channel_id_to_stop) {
	auto channel = channels_map.find(channel_id_to_stop);

	if (channel != channels_map.end()) {
		result = channel->second->stop();
	}

	if (result != FMOD_OK) {
		SIK_ERROR("Cant stop channel");
	}
}

Bool AudioManager::IsPlaying(Int32 channel_id) {

	auto channel = channels_map.find(channel_id);
	Bool temp = false;

	if (channel != channels_map.end()) {
		result = channel->second->isPlaying(&temp);
	}

	if (result != FMOD_OK) {
		SIK_ERROR("Cant stop channel");
	}
	return temp;
}
