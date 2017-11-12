#include "effect.h"

#include "game/settings.h"

#include "Sound.h"
#include "SoundDevice.h"

void sound_effect_set_volume(int percentage)
{
    for (int i = SoundChannel_EffectsMin; i <= SoundChannel_EffectsMax; i++) {
        if (SoundDevice_hasChannel(i)) {
            SoundDevice_setChannelVolume(i, percentage);
        }
    }
}

void sound_effect_play(int effect)
{
    if (!setting_sound(SOUND_EFFECTS)->enabled) {
        return;
    }
    if (!SoundDevice_hasChannel(effect) || SoundDevice_isChannelPlaying(effect)) {
        return;
    }
    SoundDevice_playChannel(effect);
}
