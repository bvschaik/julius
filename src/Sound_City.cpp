#include "Sound.h"
#include "Data/Sound.h"
#include "Time.h"

#include <string.h>

static TimeMillis lastUpdateTime;

void Sound_City_init()
{
	lastUpdateTime = Time_getMillis();
	memset(Data_Sound_City, 0, 8960);
	for (int i = 0; i < 70; i++) {
		Data_Sound_City[i].lastPlayedTime = lastUpdateTime;
	}
	for (int i = 1; i < 63; i++) {
		Data_Sound_City[i].inUse = 1;
		Data_Sound_City[i].viewsThreshold = 200;
		Data_Sound_City[i].numChannels = 1;
		Data_Sound_City[i].delayMillis = 30000;
	}
	Data_Sound_City[1].channels[0] = 30;
	Data_Sound_City[2].channels[0] = 34;
	Data_Sound_City[3].channels[0] = 38;
	Data_Sound_City[4].channels[0] = 42;
	Data_Sound_City[5].channels[0] = 46;
	Data_Sound_City[6].channels[0] = 50;
	Data_Sound_City[7].channels[0] = 51;
	Data_Sound_City[8].channels[0] = 52;
	Data_Sound_City[9].channels[0] = 53;
	Data_Sound_City[10].channels[0] = 54;
	Data_Sound_City[11].channels[0] = 55;
	Data_Sound_City[12].channels[0] = 56;
	Data_Sound_City[13].channels[0] = 57;
	Data_Sound_City[14].channels[0] = 58;
	Data_Sound_City[15].channels[0] = 62;
	Data_Sound_City[16].channels[0] = 63;
	Data_Sound_City[17].channels[0] = 64;
	Data_Sound_City[18].channels[0] = 65;
	Data_Sound_City[19].channels[0] = 66;
	Data_Sound_City[20].channels[0] = 67;
	Data_Sound_City[21].channels[0] = 68;
	Data_Sound_City[22].channels[0] = 69;
	Data_Sound_City[23].channels[0] = 70;
	Data_Sound_City[24].channels[0] = 74;
	Data_Sound_City[25].channels[0] = 78;
	Data_Sound_City[26].channels[0] = 79;
	Data_Sound_City[27].channels[0] = 80;
	Data_Sound_City[28].channels[0] = 81;
	Data_Sound_City[29].channels[0] = 82;
	Data_Sound_City[30].channels[0] = 83;
	Data_Sound_City[31].channels[0] = 87;
	Data_Sound_City[32].channels[0] = 89;
	Data_Sound_City[33].channels[0] = 91;
	Data_Sound_City[34].channels[0] = 93;
	Data_Sound_City[35].channels[0] = 95;
	Data_Sound_City[36].channels[0] = 97;
	Data_Sound_City[37].channels[0] = 98;
	Data_Sound_City[38].channels[0] = 99;
	Data_Sound_City[39].channels[0] = 100;
	Data_Sound_City[40].channels[0] = 101;
	Data_Sound_City[41].channels[0] = 102;
	Data_Sound_City[42].channels[0] = 106;
	Data_Sound_City[43].channels[0] = 110;
	Data_Sound_City[44].channels[0] = 111;
	Data_Sound_City[45].channels[0] = 112;
	Data_Sound_City[46].channels[0] = 113;
	Data_Sound_City[47].channels[0] = 114;
	Data_Sound_City[48].channels[0] = 115;
	Data_Sound_City[49].channels[0] = 116;
	Data_Sound_City[50].channels[0] = 117;
	Data_Sound_City[51].channels[0] = 118;
	Data_Sound_City[52].channels[0] = 119;
	Data_Sound_City[53].channels[0] = 120;
	Data_Sound_City[54].channels[0] = 121;
	Data_Sound_City[55].channels[0] = 122;
	Data_Sound_City[56].channels[0] = 123;
	Data_Sound_City[57].channels[0] = 124;
	Data_Sound_City[58].channels[0] = 125;
	Data_Sound_City[59].channels[0] = 126;
	Data_Sound_City[60].channels[0] = 127;
	Data_Sound_City[61].channels[0] = 128;
	Data_Sound_City[62].channels[0] = 132;
	Data_Sound_City[63].channels[0] = 133;
}

void Sound_City_markBuildingView(int buildingId, int direction)
{
	// TODO
}

void Sound_City_decayViews()
{
	for (int i = 0; i < 70; i++) {
		for (int d = 0; d < 5; d++) {
			Data_Sound_City[i].directionViews[d] = 0;
		}
		Data_Sound_City[i].totalViews /= 2;
	}
}

void Sound_City_play()
{
	TimeMillis now = Time_getMillis();
	for (int i = 1; i < 70; i++) {
		Data_Sound_City[i].shouldPlay = 0;
		if (Data_Sound_City[i].available) {
			Data_Sound_City[i].available = 0;
			if (Data_Sound_City[i].totalViews >= Data_Sound_City[i].viewsThreshold) {
				if (now - Data_Sound_City[i].lastPlayedTime >= Data_Sound_City[i].delayMillis) {
					Data_Sound_City[i].shouldPlay = 1;
				}
			}
		} else {
			Data_Sound_City[i].totalViews = 0;
			for (int d = 0; d < 5; d++) {
				Data_Sound_City[i].directionViews[d] = 0;
			}
		}
	}

	if (now - lastUpdateTime < 2000) {
		// Only play 1 sound every 2 seconds
		return;
	}
	TimeMillis maxDelay = 0;
	int maxSoundId = 0;
	for (int i = 1; i < 70; i++) {
		if (Data_Sound_City[i].shouldPlay) {
			if (now - Data_Sound_City[i].lastPlayedTime > maxDelay) {
				maxDelay = now - Data_Sound_City[i].lastPlayedTime;
				maxSoundId = i;
			}
		}
	}
	if (!maxSoundId) {
		return;
	}
	
	// always only one channel available... use it
	int channel = Data_Sound_City[maxSoundId].channels[0];
	int direction;
	if (Data_Sound_City[maxSoundId].directionViews[SoundDirectionCenter] > 10) {
		direction = SoundDirectionCenter;
	} else if (Data_Sound_City[maxSoundId].directionViews[SoundDirectionLeft] > 10) {
		direction = SoundDirectionLeft;
	} else if (Data_Sound_City[maxSoundId].directionViews[SoundDirectionRight] > 10) {
		direction = SoundDirectionRight;
	} else {
		direction = SoundDirectionCenter;
	}

	Sound_playCityChannel(channel, direction);
	lastUpdateTime = now;
	Data_Sound_City[maxSoundId].lastPlayedTime = now;
	Data_Sound_City[maxSoundId].totalViews = 0;
	for (int d = 0; d < 5; d++) {
		Data_Sound_City[maxSoundId].directionViews[d] = 0;
	}
	Data_Sound_City[maxSoundId].timesPlayed++;
}

