#include "Sound.h"
#include "SoundDevice.h"
#include "Data/Settings.h"

static const char filenames[SoundChannel_TotalChannels][32] = {
	"", // speech channel
	"wavs/PANEL1.WAV",
	"wavs/PANEL3.WAV",
	"wavs/ICON1.WAV",
	"wavs/BUILD1.WAV",
	"wavs/EXPLOD1.WAV",
	"wavs/FANFARE.WAV",
	"wavs/FANFARE2.WAV",
	"wavs/ARROW.WAV",
	"wavs/arrow_hit.wav",
	"wavs/AXE.WAV",
	"wavs/BALLISTA.WAV",
	"wavs/ballista_hit_ground.wav",
	"wavs/ballista_hit_person.wav",
	"wavs/CLUB.WAV",
	"wavs/CAMEL1.WAV",
	"wavs/ELEPHANT.WAV",
	"wavs/elephant_hit.wav",
	"wavs/elephant_die.wav",
	"wavs/HORSE.WAV",
	"wavs/HORSE2.WAV",
	"wavs/horse_mov.wav",
	"wavs/JAVELIN.WAV",
	"wavs/lion_attack.wav",
	"wavs/LION_DIE.WAV",
	"wavs/MARCHING.WAV",
	"wavs/SWORD.WAV",
	"wavs/sword_swing.wav",
	"wavs/sword_light.wav",
	"wavs/spear_attack.wav",
	"wavs/wolf_attack.wav",
	"wavs/WOLF_DIE.WAV",
	"wavs/DIE1.WAV",
	"wavs/DIE2.WAV",
	"wavs/DIE4.WAV",
	"wavs/DIE10.WAV",
	"wavs/DIE3.WAV",
	"wavs/DIE5.WAV",
	"wavs/DIE8.WAV",
	"wavs/DIE9.WAV",
	"wavs/sheep_die.wav",
	"wavs/zebra_die.wav",
	"wavs/wolf_howl.wav",
	"wavs/Fire_splash.wav",
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
	"wavs/THEATRE.WAV",
	"wavs/hippodrome.wav",
	"wavs/colloseum.wav",
	"wavs/GLAD_PIT.WAV",
	"wavs/LION_PIT.WAV",
	"wavs/ART_PIT.WAV",
	"wavs/CHAR_PIT.WAV",
	"wavs/GARDENS1.WAV",
	"",
	"",
	"",
	"wavs/CLINIC.WAV",
	"wavs/HOSPITAL.WAV",
	"wavs/BATHS.WAV",
	"wavs/BARBER.WAV",
	"wavs/SCHOOL.WAV",
	"wavs/ACADEMY.WAV",
	"wavs/LIBRARY.WAV",
	"wavs/prefecture.wav",
	"wavs/FORT1.WAV",
	"",
	"",
	"",
	"wavs/TOWER1.WAV",
	"",
	"",
	"",
	"wavs/temp_farm.wav",
	"wavs/temp_ship.wav",
	"wavs/temp_comm.wav",
	"wavs/TEMP_WAR.WAV",
	"wavs/temp_love.wav",
	"wavs/MARKET1.WAV",
	"",
	"",
	"",
	"wavs/GRANARY1.WAV",
	"",
	"wavs/warehouse1.wav",
	"",
	"wavs/shipyard1.wav",
	"",
	"wavs/DOCK1.WAV",
	"",
	"wavs/WHARF1.WAV",
	"",
	"wavs/PALACE.WAV",
	"wavs/eng_post.wav",
	"wavs/SENATE.WAV",
	"wavs/FORUM.WAV",
	"wavs/RESEVOIR.WAV",
	"wavs/fountain1.wav",
	"",
	"",
	"",
	"wavs/well1.wav",
	"",
	"",
	"",
	"wavs/MIL_ACAD.WAV",
	"wavs/ORACLE.WAV",
	"wavs/burning_ruin.wav",
	"wavs/wheat_farm.wav",
	"wavs/VEG_FARM.WAV",
	"wavs/figs_farm.wav",
	"wavs/olives_farm.wav",
	"wavs/vines_farm.wav",
	"wavs/meat_farm.wav",
	"wavs/QUARRY.WAV",
	"wavs/MINE.WAV",
	"wavs/lumber_mill.wav",
	"wavs/CLAY_PIT.WAV",
	"wavs/wine_workshop.wav",
	"wavs/oil_workshop.wav",
	"wavs/weap_workshop.wav",
	"wavs/furn_workshop.wav",
	"wavs/pott_workshop.wav",
	"wavs/Empty_land1.wav",
	"",
	"",
	"",
	"wavs/RIVER.WAV",
	"wavs/mission.wav",
};


void Sound_init()
{
	SoundDevice_open();
	SoundDevice_initChannels(SoundChannel_TotalChannels, filenames);
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
	if (!Data_Settings.soundEffectsEnabled) {
		return;
	}
	if (!SoundDevice_hasChannel(channel) || SoundDevice_isChannelPlaying(channel)) {
		return;
	}
	SoundDevice_playChannel(channel);
}

void Sound_Speech_playFile(const char *filename)
{
	if (!Data_Settings.soundSpeechEnabled) {
		return;
	}
	if (SoundDevice_isChannelPlaying(SoundChannel_Speech)) {
		SoundDevice_stopChannel(SoundChannel_Speech);
	}
	SoundDevice_playSoundOnChannel(filename, SoundChannel_Speech);
	SoundDevice_setChannelVolume(SoundChannel_Speech, Data_Settings.soundSpeechPercentage);
}
