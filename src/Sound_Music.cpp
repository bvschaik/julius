#include "Sound.h"
#include "SoundDevice.h"
#include "Data/Settings.h"
#include "Data/CityInfo.h"

static int currentTrack;
static int nextCheck = 0;

static const char tracks[][32] = {
	"",
	"wavs/rome1.wav",
	"wavs/rome2.wav",
	"wavs/rome3.wav",
	"wavs/rome4.wav",
	"wavs/rome5.wav",
	"wavs/Combat_Long.wav",
	"wavs/Combat_short.wav"
};

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
	if (Data_CityInfo.numEnemiesInCity + Data_CityInfo.numImperialSoldiersInCity >= 32) {
		track = 6;
	} else if (Data_CityInfo.numEnemiesInCity + Data_CityInfo.numImperialSoldiersInCity > 0) {
		track = 7;
	} else if (Data_CityInfo.population < 1000) {
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

	if (track == currentTrack) {
		return;
	}

	SoundDevice_stopMusic();
	SoundDevice_playMusic(tracks[track]);
	currentTrack = track;
	nextCheck = 10;
}
