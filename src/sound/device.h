#ifndef SOUND_DEVICE_H
#define SOUND_DEVICE_H

#define CHANNEL_FILENAME_MAX 32

void sound_device_open();
void sound_device_close();

void sound_device_init_channels(int num_channels, const char filenames[][CHANNEL_FILENAME_MAX]);
int sound_device_has_channel(int channel);
int sound_device_is_channel_playing(int channel);

void sound_device_set_music_volume(int volume_percentage);
void sound_device_set_channel_volume(int channel, int volume_percentage);
void sound_device_set_channel_panning(int channel, int left_pct, int right_pct);

void sound_device_play_music(const char *filename);
void sound_device_play_file_on_channel(const char *filename, int channel);
void sound_device_play_channel(int channel);
void sound_device_stop_music();
void sound_device_stop_channel(int channel);

void sound_device_use_custom_music_player(int bitdepth, int channels, int rate, const unsigned char *(*callback)(int *out_len));
void sound_device_use_default_music_player();

#endif // SOUND_DEVICE_H
