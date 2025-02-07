#include "speech.h"

#include "core/dir.h"
#include "game/settings.h"
#include "sound/device.h"

void sound_speech_set_volume(int percentage)
{
    sound_device_set_volume_for_type(SOUND_TYPE_SPEECH, percentage);
}

void sound_speech_play_file(const char *filename)
{
    if (!setting_sound(SOUND_TYPE_SPEECH)->enabled) {
        return;
    }
    sound_device_stop_type(SOUND_TYPE_SPEECH);

    const char *cased_filename = dir_get_file(filename, MAY_BE_LOCALIZED);
    if (cased_filename) {
        sound_device_play_file_on_channel(cased_filename, SOUND_TYPE_SPEECH, setting_sound(SOUND_TYPE_SPEECH)->volume);
    }
}

void sound_speech_stop(void)
{
    sound_device_stop_type(SOUND_TYPE_SPEECH);
}
