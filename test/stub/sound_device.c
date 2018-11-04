#include "sound/device.h"

void sound_device_open(void)
{}

void sound_device_close(void)
{}

void sound_device_init_channels(int numChannels, const char filenames[][CHANNEL_FILENAME_MAX])
{}

int sound_device_has_channel(int channel)
{
    return 0;
}

int sound_device_is_channel_playing(int channel)
{
    return 0;
}

void sound_device_set_music_volume(int volumePercentage)
{}

void sound_device_set_channel_volume(int channel, int volumePercentage)
{}

void sound_device_set_channel_panning(int channel, int leftPct, int rightPct)
{}

void sound_device_play_music(const char *filename)
{}

void sound_device_play_file_on_channel(const char *filename, int channel)
{}

void sound_device_play_channel(int channel)
{}

void sound_device_stop_music(void)
{}

void sound_device_stop_channel(int channel)
{}
