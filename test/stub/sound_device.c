#include "sound/device.h"

void sound_device_open(void)
{}

void sound_device_close(void)
{}

void sound_device_init_channels(int num_channels, char filenames[][CHANNEL_FILENAME_MAX])
{}

int sound_device_is_channel_playing(int channel)
{
    return 0;
}

void sound_device_set_music_volume(int volume_pct)
{}

void sound_device_set_channel_volume(int channel, int volume_pct)
{}

int sound_device_play_music(const char *filename, int volume_pct)
{
    return 0;
}

void sound_device_play_file_on_channel(const char *filename, int channel, int volume_pct)
{}

void sound_device_play_channel(int channel, int volume_pct)
{}

void sound_device_play_channel_panned(int channel, int volume_pct, int left_pct, int right_pct)
{}

void sound_device_stop_music(void)
{}

void sound_device_stop_channel(int channel)
{}
