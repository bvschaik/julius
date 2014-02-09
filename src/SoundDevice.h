#ifndef SOUNDDEVICE_H
#define SOUNDDEVICE_H

//void SoundDevice_open();
//void SoundDevice_close();

//void SoundDevice_initChannels(/* TODO args */);
int SoundDevice_hasChannel(int channel);
int SoundDevice_isChannelPlaying(int channel);

void SoundDevice_setMusicVolume(int volumePercentage);
void SoundDevice_setChannelVolume(int channel, int volumePercentage);
void SoundDevice_setChannelPanning(int channel, int left, int right);

//void SoundDevice_playMusic(const char *filename);
//void SoundDevice_playSound(const char *filename);
void SoundDevice_playChannel(int channel);
void SoundDevice_stopMusic();
void SoundDevice_stopChannel(int channel);

#endif
