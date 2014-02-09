#include "Sound.h"
#include "SoundDevice.h"
#include "Data/Settings.h"

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
