#include "speech.h"

#include "core/dir.h"
#include "game/settings.h"

#include <sound>

void sound_speech_set_volume(int percentage)
{
    sound_device_set_channel_volume(SOUND_CHANNEL_SPEECH, percentage);
}

void sound_speech_play_file(const char *filename)
{
    if (!setting_sound(SOUND_SPEECH)->enabled)
    {
        return;
    }
    if (sound_device_is_channel_playing(SOUND_CHANNEL_SPEECH))
    {
        sound_device_stop_channel(SOUND_CHANNEL_SPEECH);
    }
    const char *cased_filename = dir_get_case_corrected_file(filename);
    if (cased_filename)
    {
        sound_device_play_sound_on_channel(cased_filename, SOUND_CHANNEL_SPEECH);
        sound_speech_set_volume(setting_sound(SOUND_SPEECH)->volume);
    }
}

void sound_speech_stop()
{
    sound_device_stop_channel(SOUND_CHANNEL_SPEECH);
}
