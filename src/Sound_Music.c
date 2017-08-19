#include "Sound.h"

#include "SoundDevice.h"

#include "Data/Settings.h"
#include "Data/CityInfo.h"

#include "core/dir.h"

enum {
	Track_None = 0,
	Track_City1 = 1,
	Track_City2 = 2,
	Track_City3 = 3,
	Track_City4 = 4,
	Track_City5 = 5,
	Track_CombatShort = 6,
	Track_CombatLong = 7,
	Track_Intro = 8
};

static int currentTrack;
static int nextCheck = 0;

static const char tracks[][32] = {
	"",
	"wavs/ROME1.WAV",
	"wavs/ROME2.WAV",
	"wavs/ROME3.WAV",
	"wavs/ROME4.WAV",
	"wavs/ROME5.WAV",
	"wavs/Combat_Long.wav",
	"wavs/Combat_Short.wav",
	"wavs/setup.wav"
};

static void playTrack(int track)
{
	SoundDevice_stopMusic();
	SoundDevice_playMusic(dir_get_case_corrected_file(tracks[track]));
	SoundDevice_setMusicVolume(Data_Settings.soundMusicPercentage);
	currentTrack = track;
}

void Sound_Music_playIntro()
{
	playTrack(Track_Intro);
}

void Sound_Music_reset()
{
	currentTrack = Track_None;
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
	int track = Track_None;
	if (Data_CityInfo.numEnemiesInCity + Data_CityInfo.numImperialSoldiersInCity >= 32) {
		track = Track_CombatLong;
	} else if (Data_CityInfo.numEnemiesInCity + Data_CityInfo.numImperialSoldiersInCity > 0) {
		track = Track_CombatShort;
	} else if (Data_CityInfo.population < 1000) {
		track = Track_City1;
	} else if (Data_CityInfo.population < 2000) {
		track = Track_City2;
	} else if (Data_CityInfo.population < 5000) {
		track = Track_City3;
	} else if (Data_CityInfo.population < 7000) {
		track = Track_City4;
	} else {
		track = Track_City5;
	}

	if (track == currentTrack) {
		return;
	}

	playTrack(track);
	nextCheck = 10;
}
