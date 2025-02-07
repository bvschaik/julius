#include "sound/device.h"

void sound_device_open(void)
{}

void sound_device_close(void)
{}

void sound_device_init_channels(void)
{}

void sound_device_set_music_volume(int volume_pct)
{}

void sound_device_set_volume_for_type(int channel, int volume_pct)
{}

int sound_device_play_music(const char *filename, int volume_pct, int loop)
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

void sound_device_stop_type(sound_type type)
{}
