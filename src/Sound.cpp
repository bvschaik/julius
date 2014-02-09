#include "Sound.h"
#include "SoundDevice.h"
#include "Data/Settings.h"

#define SPEECH_CHANNEL 0

void Sound_setMusicVolume(int percentage)
{
	SoundDevice_setMusicVolume(percentage);
}

void Sound_setSpeechVolume(int percentage)
{
	SoundDevice_setChannelVolume(SPEECH_CHANNEL, percentage);
}

void Sound_setEffectsVolume(int percentage)
{
	for (int i = 1; i < 10; i++) {
		if (SoundDevice_hasChannel(i)) {
			SoundDevice_setChannelVolume(i, percentage);
		}
	}
}

void Sound_setCityVolume(int percentage)
{
	for (int i = 10; i < 150; i++) {
		if (SoundDevice_hasChannel(i)) {
			SoundDevice_setChannelVolume(i, percentage);
		}
	}
}

void Sound_stopMusic()
{
	SoundDevice_stopMusic();
}

void Sound_stopSpeech()
{
	SoundDevice_stopChannel(SPEECH_CHANNEL);
}

void Sound_playCityChannel(int channel, int direction)
{
	if (!Data_Settings.soundCityEnabled) {
		return;
	}
	if (!SoundDevice_hasChannel(channel) || SoundDevice_isChannelPlaying(channel)) {
		return;
	}
	int leftPan;
	int rightPan;
	switch (direction) {
		case SoundDirectionCenter:
			leftPan = rightPan = 255;
			break;
		case SoundDirectionLeft:
			leftPan = 255;
			rightPan = 0;
			break;
		case SoundDirectionRight:
			leftPan = 0;
			rightPan = 255;
			break;
		default:
			leftPan = rightPan = 0;
			break;
	}
	SoundDevice_setChannelPanning(channel, leftPan, rightPan);
	SoundDevice_playChannel(channel);
}
