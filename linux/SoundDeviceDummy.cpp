#include "../src/SoundDevice.h"
#include "SDL_mixer.h"

#include <stdio.h>

#define AUDIO_RATE 22050
#define AUDIO_FORMAT AUDIO_S16MSB
#define AUDIO_CHANNELS 2
#define AUDIO_BUFFERS 4096

static int initialized = 0;
static Mix_Music *music = 0;

static int percentageToVolume(int percentage)
{
	return percentage * 128 / 100;
}

void SoundDevice_open()
{
	if (0 == Mix_OpenAudio(AUDIO_RATE, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_BUFFERS)) {
		initialized = 1;
		printf("SOUND: OK\n");
	} else {
		printf("SOUND: not initialized\n");
	}
}

void SoundDevice_close()
{
	if (initialized) {
		Mix_CloseAudio();
	}
}

void SoundDevice_initChannels(/* TODO args */)
{
}

int SoundDevice_hasChannel(int channel)
{
	return 0;
}
int SoundDevice_isChannelPlaying(int channel)
{
	return 0;
}

void SoundDevice_setMusicVolume(int volumePercentage)
{
	Mix_VolumeMusic(percentageToVolume(volumePercentage));
}

void SoundDevice_setChannelVolume(int channel, int volumePercentage)
{
}

void SoundDevice_setChannelPanning(int channel, int left, int right)
{}

void SoundDevice_playMusic(const char *filename)
{
	printf("SOUND: trying music file: %s\n", filename);
	if (initialized) {
		SoundDevice_stopMusic();
		music = Mix_LoadMUS(filename);
		if (music) {
			printf("SOUND: playing music file: %s\n", filename);
			Mix_PlayMusic(music, -1);
		}
	}
}

void SoundDevice_playSound(const char *filename)
{}

void SoundDevice_playChannel(int channel)
{}

void SoundDevice_stopMusic()
{
	if (initialized) {
		if (music) {
			Mix_HaltMusic();
			Mix_FreeMusic(music);
			music = 0;
		}
	}
}

void SoundDevice_stopChannel(int channel)
{}
