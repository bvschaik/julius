#include "effect.h"

#include "game/settings.h"

#include <sound>

void sound_effect_set_volume(int percentage)
{
    for (int i = SOUND_CHANNEL_EFFECTS_MIN; i <= SOUND_CHANNEL_EFFECTS_MAX; i++)
    {
        if (sound_device_has_channel(i))
        {
            sound_device_set_channel_volume(i, percentage);
        }
    }
}

void sound_effect_play(int effect)
{
    if (!setting_sound(SOUND_EFFECTS)->enabled)
    {
        return;
    }
    if (!sound_device_has_channel(effect) || sound_device_is_channel_playing(effect))
    {
        return;
    }
    sound_device_play_channel(effect);
}
