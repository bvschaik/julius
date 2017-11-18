#include "effect.h"

#include "game/settings.h"

#include "sounddevice.h"
#include <sound>

void sound_effect_set_volume(int percentage)
{
    for (int i = SOUND_CHANNEL_EFFECTS_MIN; i <= SOUND_CHANNEL_EFFECTS_MAX; i++)
    {
        if (SoundDevice_hasChannel(i))
        {
            SoundDevice_setChannelVolume(i, percentage);
        }
    }
}

void sound_effect_play(int effect)
{
    if (!setting_sound(SOUND_EFFECTS)->enabled)
    {
        return;
    }
    if (!SoundDevice_hasChannel(effect) || SoundDevice_isChannelPlaying(effect))
    {
        return;
    }
    SoundDevice_playChannel(effect);
}
