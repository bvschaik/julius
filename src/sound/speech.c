#include "speech.h"

#include "core/dir.h"
#include "game/settings.h"

#include "Sound.h"
#include "SoundDevice.h"

void sound_speech_set_volume(int percentage)
{
    SoundDevice_setChannelVolume(SoundChannel_Speech, percentage);
}

void sound_speech_play_file(const char *filename)
{
    if (!setting_sound(SOUND_SPEECH)->enabled) {
        return;
    }
    if (SoundDevice_isChannelPlaying(SoundChannel_Speech)) {
        SoundDevice_stopChannel(SoundChannel_Speech);
    }
    const char *cased_filename = dir_get_case_corrected_file(filename);
    if (cased_filename) {
        SoundDevice_playSoundOnChannel(cased_filename, SoundChannel_Speech);
        sound_speech_set_volume(setting_sound(SOUND_SPEECH)->volume);
    }
}

void sound_speech_stop()
{
    SoundDevice_stopChannel(SoundChannel_Speech);
}
