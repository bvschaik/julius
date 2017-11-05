#ifndef SOUNDDEVICE_H
#define SOUNDDEVICE_H

void SoundDevice_open();
void SoundDevice_close();

void SoundDevice_initChannels(int numChannels, const char filenames[][32]);
int SoundDevice_hasChannel(int channel);
int SoundDevice_isChannelPlaying(int channel);

void SoundDevice_setMusicVolume(int volumePercentage);
void SoundDevice_setChannelVolume(int channel, int volumePercentage);
void SoundDevice_setChannelPanning(int channel, int leftPct, int rightPct);

void SoundDevice_playMusic(const char *filename);
void SoundDevice_playSoundOnChannel(const char *filename, int channel);
void SoundDevice_playChannel(int channel);
void SoundDevice_stopMusic();
void SoundDevice_stopChannel(int channel);

void SoundDevice_useCustomMusicPlayer(int bitdepth, int channels, int rate, const unsigned char *(*callback)(int *outLen));
void SoundDevice_useDefaultMusicPlayer();

#endif
