#include "Sound.h"
#include "SoundDevice.h"

#include "core/file.h"
#include "game/settings.h"

#include <ctype.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

struct FilesystemName {
	char lower[32];
	char cased[32];
};

static struct FilesystemName fsFilenames[500];
static int numFsNames;

static char channelFilenames[SoundChannel_TotalChannels][32] = {
	"", // speech channel
	"wavs/panel1.wav",
	"wavs/panel3.wav",
	"wavs/icon1.wav",
	"wavs/build1.wav",
	"wavs/explod1.wav",
	"wavs/fanfare.wav",
	"wavs/fanfare2.wav",
	"wavs/arrow.wav",
	"wavs/arrow_hit.wav",
	"wavs/axe.wav",
	"wavs/ballista.wav",
	"wavs/ballista_hit_ground.wav",
	"wavs/ballista_hit_person.wav",
	"wavs/club.wav",
	"wavs/camel1.wav",
	"wavs/elephant.wav",
	"wavs/elephant_hit.wav",
	"wavs/elephant_die.wav",
	"wavs/horse.wav",
	"wavs/horse2.wav",
	"wavs/horse_mov.wav",
	"wavs/javelin.wav",
	"wavs/lion_attack.wav",
	"wavs/lion_die.wav",
	"wavs/marching.wav",
	"wavs/sword.wav",
	"wavs/sword_swing.wav",
	"wavs/sword_light.wav",
	"wavs/spear_attack.wav",
	"wavs/wolf_attack.wav",
	"wavs/wolf_die.wav",
	"wavs/die1.wav",
	"wavs/die2.wav",
	"wavs/die4.wav",
	"wavs/die10.wav",
	"wavs/die3.wav",
	"wavs/die5.wav",
	"wavs/die8.wav",
	"wavs/die9.wav",
	"wavs/sheep_die.wav",
	"wavs/zebra_die.wav",
	"wavs/wolf_howl.wav",
	"wavs/fire_splash.wav",
	"wavs/formation_shield.wav",
	// city sounds
	"wavs/house_slum1.wav",
	"",
	"",
	"",
	"wavs/house_poor1.wav",
	"",
	"",
	"",
	"wavs/house_mid1.wav",
	"",
	"",
	"",
	"wavs/house_good1.wav",
	"",
	"",
	"",
	"wavs/house_posh1.wav",
	"",
	"",
	"",
	"wavs/ampitheatre.wav",
	"wavs/theatre.wav",
	"wavs/hippodrome.wav",
	"wavs/colloseum.wav",
	"wavs/glad_pit.wav",
	"wavs/lion_pit.wav",
	"wavs/art_pit.wav",
	"wavs/char_pit.wav",
	"wavs/gardens1.wav",
	"",
	"",
	"",
	"wavs/clinic.wav",
	"wavs/hospital.wav",
	"wavs/baths.wav",
	"wavs/barber.wav",
	"wavs/school.wav",
	"wavs/academy.wav",
	"wavs/library.wav",
	"wavs/prefecture.wav",
	"wavs/fort1.wav",
	"",
	"",
	"",
	"wavs/tower1.wav",
	"",
	"",
	"",
	"wavs/temp_farm.wav",
	"wavs/temp_ship.wav",
	"wavs/temp_comm.wav",
	"wavs/temp_war.wav",
	"wavs/temp_love.wav",
	"wavs/market1.wav",
	"",
	"",
	"",
	"wavs/granary1.wav",
	"",
	"wavs/warehouse1.wav",
	"",
	"wavs/shipyard1.wav",
	"",
	"wavs/dock1.wav",
	"",
	"wavs/wharf1.wav",
	"",
	"wavs/palace.wav",
	"wavs/eng_post.wav",
	"wavs/senate.wav",
	"wavs/forum.wav",
	"wavs/resevoir.wav",
	"wavs/fountain1.wav",
	"",
	"",
	"",
	"wavs/well1.wav",
	"",
	"",
	"",
	"wavs/mil_acad.wav",
	"wavs/oracle.wav",
	"wavs/burning_ruin.wav",
	"wavs/wheat_farm.wav",
	"wavs/veg_farm.wav",
	"wavs/figs_farm.wav",
	"wavs/olives_farm.wav",
	"wavs/vines_farm.wav",
	"wavs/meat_farm.wav",
	"wavs/quarry.wav",
	"wavs/mine.wav",
	"wavs/lumber_mill.wav",
	"wavs/clay_pit.wav",
	"wavs/wine_workshop.wav",
	"wavs/oil_workshop.wav",
	"wavs/weap_workshop.wav",
	"wavs/furn_workshop.wav",
	"wavs/pott_workshop.wav",
	"wavs/empty_land1.wav",
	"",
	"",
	"",
	"wavs/river.wav",
	"wavs/mission.wav",
};

static int compareLower(const void *va, const void *vb)
{
	const struct FilesystemName *a = (const struct FilesystemName*)va;
	const struct FilesystemName *b = (const struct FilesystemName*)vb;
	return strcmp(a->lower, b->lower);
}

static void initFilesystemNames()
{
	char dirs[16][5] = {
		"wavs", "WAVS", "Wavs", "WAVs",
		"wavS", "waVs", "waVS", "wAvs",
		"wAvS", "wAVs", "wAVS", "WavS",
		"WaVs", "WaVS", "WAvs", "WAvS"
	};
	numFsNames = 0;
	DIR *d;
	int wavs;
	for (wavs = 0; wavs < 16; wavs++) {
		d = opendir(dirs[wavs]);
		if (d) {
			break;
		}
	}
	if (!d) return;

	struct dirent *entry;
	int i = 0;
	while ((entry = readdir(d)) && i < 500) {
		if (file_has_extension(entry->d_name, "wav")) {
			strncpy(fsFilenames[i].cased, dirs[wavs], 4);
			fsFilenames[i].cased[4] = '/';
			strncpy(&fsFilenames[i].cased[5], entry->d_name, 26);
			fsFilenames[i].cased[31] = 0;
			for (int n = 0; n < 32; n++) {
				fsFilenames[i].lower[n] = tolower(fsFilenames[i].cased[n]);
			}
			++i;
		}
	}
	closedir(d);
	numFsNames = i;
	qsort(fsFilenames, numFsNames, sizeof(struct FilesystemName), compareLower);
}

static void initChannelFilenames()
{
	struct FilesystemName key, *entry;
	for (int i = 0; i < SoundChannel_TotalChannels; i++) {
		if (channelFilenames[i][0] == 0) continue;

		strcpy(key.lower, channelFilenames[i]);
		entry = (struct FilesystemName*) bsearch(&key, fsFilenames, numFsNames,
			sizeof(struct FilesystemName), compareLower);
		if (entry) {
			strcpy(channelFilenames[i], entry->cased);
		} else {
			printf("Sound file not found: %s\n", key.lower);
		}
	}
}

static const char *getCasedFilename(const char *filename)
{
	struct FilesystemName key, *entry;
	strcpy(key.lower, filename);
	entry = (struct FilesystemName*) bsearch(&key, fsFilenames, numFsNames,
		sizeof(struct FilesystemName), compareLower);
	if (entry) {
		return entry->cased;
	}
	return 0;
}

void Sound_init()
{
	initFilesystemNames();
	initChannelFilenames();

	SoundDevice_open();
	SoundDevice_initChannels(SoundChannel_TotalChannels, channelFilenames);
}

void Sound_shutdown()
{
	SoundDevice_close();
}

void Sound_setMusicVolume(int percentage)
{
	SoundDevice_setMusicVolume(percentage);
}

void Sound_setSpeechVolume(int percentage)
{
	SoundDevice_setChannelVolume(SoundChannel_Speech, percentage);
}

void Sound_setEffectsVolume(int percentage)
{
	for (int i = SoundChannel_EffectsMin; i <= SoundChannel_EffectsMax; i++) {
		if (SoundDevice_hasChannel(i)) {
			SoundDevice_setChannelVolume(i, percentage);
		}
	}
}

void Sound_setCityVolume(int percentage)
{
	for (int i = SoundChannel_CityMin; i <= SoundChannel_CityMax; i++) {
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
	SoundDevice_stopChannel(SoundChannel_Speech);
}

void Sound_playCityChannel_internal(int channel, int direction)
{
	channel += SoundChannel_CityOffset;
	if (!setting_sound(SOUND_CITY)->enabled) {
		return;
	}
	if (!SoundDevice_hasChannel(channel) || SoundDevice_isChannelPlaying(channel)) {
		return;
	}
	int leftPan;
	int rightPan;
	switch (direction) {
		case SoundDirectionCenter:
			leftPan = rightPan = 100;
			break;
		case SoundDirectionLeft:
			leftPan = 100;
			rightPan = 0;
			break;
		case SoundDirectionRight:
			leftPan = 0;
			rightPan = 100;
			break;
		default:
			leftPan = rightPan = 0;
			break;
	}
	SoundDevice_setChannelPanning(channel, leftPan, rightPan);
	SoundDevice_playChannel(channel);
}

void Sound_Effects_playChannel(int channel)
{
	if (!setting_sound(SOUND_EFFECTS)->enabled) {
		return;
	}
	if (!SoundDevice_hasChannel(channel) || SoundDevice_isChannelPlaying(channel)) {
		return;
	}
	SoundDevice_playChannel(channel);
}

void Sound_Speech_playFile(const char *filename)
{
	if (!setting_sound(SOUND_SPEECH)->enabled) {
		return;
	}
	if (SoundDevice_isChannelPlaying(SoundChannel_Speech)) {
		SoundDevice_stopChannel(SoundChannel_Speech);
	}
	const char *casedFilename = getCasedFilename(filename);
	if (casedFilename) {
		SoundDevice_playSoundOnChannel(casedFilename, SoundChannel_Speech);
		SoundDevice_setChannelVolume(SoundChannel_Speech, setting_sound(SOUND_SPEECH)->volume);
	}
}
