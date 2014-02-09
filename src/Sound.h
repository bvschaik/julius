#ifndef SOUND_H
#define SOUND_H

enum {
	SoundDirectionLeft = 0,
	SoundDirectionCenter = 2,
	SoundDirectionRight = 4
};

// NB: only internal use!
void Sound_playCityChannel(int channel, int direction);

void Sound_City_init();
void Sound_City_markBuildingView(int buildingId, int direction);
void Sound_City_decayViews();
void Sound_City_play();

void Sound_Music_playTrack(int trackId);

void Sound_Walker_playPhrase(int walkerType, int phraseId);

#endif
