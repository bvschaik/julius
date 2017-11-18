#include "speech.h"

#include "core/dir.h"
#include "game/settings.h"

#include "sounddevice.h"
#include <sound>

void sound_speech_set_volume(int percentage)
{
    SoundDevice_setChannelVolume(SOUND_CHANNEL_SPEECH, percentage);
}

void sound_speech_playfile(const char *filename)
{
    if (!setting_sound(SOUND_SPEECH)->enabled)
    {
        return;
    }
    if (SoundDevice_isChannelPlaying(SOUND_CHANNEL_SPEECH))
    {
        SoundDevice_stopChannel(SOUND_CHANNEL_SPEECH);
    }
    const char *cased_filename = dir_get_case_corrected_file(filename);
    if (cased_filename)
    {
        SoundDevice_playSoundOnChannel(cased_filename, SOUND_CHANNEL_SPEECH);
        sound_speech_set_volume(setting_sound(SOUND_SPEECH)->volume);
    }
}

void sound_speech_stop()
{
    SoundDevice_stopChannel(SOUND_CHANNEL_SPEECH);
}
