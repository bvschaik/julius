#include "Sound.h"
#include "Data/Settings.h"
#include "Data/CityInfo.h"

static int currentTrack;
static int nextCheck = 0;


void Sound_Music_reset()
{
	currentTrack = 0;
	nextCheck = 0;
}

void Sound_Music_update()
{
	if (nextCheck) {
		--nextCheck;
		return;
	}
	if (!Data_Settings.soundMusicEnabled) {
		return;
	}
	int track = 0;
	// TODO determine battle track ID
	if (Data_CityInfo.population < 1000) {
		track = 1;
	} else if (Data_CityInfo.population < 2000) {
		track = 2;
	} else if (Data_CityInfo.population < 5000) {
		track = 3;
	} else if (Data_CityInfo.population < 7000) {
		track = 4;
	} else {
		track = 5;
	}

	if (track != currentTrack) {
		// TODO playSound
		currentTrack = track;
		nextCheck = 10;
	}
}
