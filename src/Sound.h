#ifndef SOUND_H
#define SOUND_H

enum {
	SoundDirectionLeft = 0,
	SoundDirectionCenter = 2,
	SoundDirectionRight = 4
};

enum {
	SoundChannel_Speech = 0,

	// user interface effects
	SoundChannel_EffectsMin = 1,
	SoundChannel_EffectsMax = 44,

	// city sounds (from buildings)
	SoundChannel_CityOffset = 15,
	SoundChannel_CityMin = 45,
	SoundChannel_CityMax = 148,

	SoundChannel_TotalChannels = SoundChannel_CityMax + 1
};

void Sound_init();
void Sound_shutdown();

#endif
