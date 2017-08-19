#ifndef DATA_SOUND_H
#define DATA_SOUND_H

#include "core/time.h"

extern struct Data_Sound_City {
	int available;
	int totalViews;
	int viewsThreshold;
	int directionViews[5];
	int currentChannel;
	int numChannels;
	int channels[8];
	int inUse;
	int timesPlayed;
	time_millis lastPlayedTime;
	time_millis delayMillis;
	int shouldPlay;
	int unused[9];
} Data_Sound_City[70];

#endif
