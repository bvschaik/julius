#ifndef SOUND_H
#define SOUND_H

enum {
	SoundDirectionLeft = 0,
	SoundDirectionCenter = 2,
	SoundDirectionRight = 4
};

void Sound_setMusicVolume(int percentage);
void Sound_setSpeechVolume(int percentage);
void Sound_setEffectsVolume(int percentage);
void Sound_setCityVolume(int percentage);

void Sound_stopMusic();
void Sound_stopSpeech();

// NB: only internal use!
void Sound_playCityChannel(int channel, int direction);

void Sound_City_init();
void Sound_City_markBuildingView(int buildingId, int direction);
void Sound_City_decayViews();
void Sound_City_play();

void Sound_Music_reset();
void Sound_Music_update();

void Sound_Walker_playPhrase(int walkerType, int phraseId);

#endif
