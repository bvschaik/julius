#include "effect.h"

#include "core/file.h"
#include "game/settings.h"
#include "sound/device.h"

static char effect_filenames[SOUND_EFFECT_MAX][FILE_NAME_MAX] = {
    "wavs/panel1.wav",
    "wavs/panel3.wav",
    "wavs/icon1.wav",
    "wavs/build1.wav",
    "wavs/explod1.wav",
    "wavs/fanfare.wav",
    "wavs/fanfare2.wav",
    "wavs/arrow.wav",
    "wavs/arrow_hit.wav",
    "wavs/axe.wav",
    "wavs/ballista.wav",
    "wavs/ballista_hit_ground.wav",
    "wavs/ballista_hit_person.wav",
    "wavs/club.wav",
    "wavs/camel1.wav",
    "wavs/elephant.wav",
    "wavs/elephant_hit.wav",
    "wavs/elephant_die.wav",
    "wavs/horse.wav",
    "wavs/horse2.wav",
    "wavs/horse_mov.wav",
    "wavs/javelin.wav",
    "wavs/lion_attack.wav",
    "wavs/lion_die.wav",
    "wavs/marching.wav",
    "wavs/sword.wav",
    "wavs/sword_swing.wav",
    "wavs/sword_light.wav",
    "wavs/spear_attack.wav",
    "wavs/wolf_attack.wav",
    "wavs/wolf_die.wav",
    "wavs/die1.wav",
    "wavs/die2.wav",
    "wavs/die4.wav",
    "wavs/die10.wav",
    "wavs/die3.wav",
    "wavs/die5.wav",
    "wavs/die8.wav",
    "wavs/die9.wav",
    "wavs/die7.wav",
    "wavs/die11.wav",
    "wavs/die6.wav",
    "wavs/sheep_die.wav",
    "wavs/zebra_die.wav",
    "wavs/wolf_howl.wav",
    "wavs/fire_splash.wav",
    "wavs/formation_shield.wav",
};

void sound_effect_set_volume(int percentage)
{
    sound_device_set_volume_for_type(SOUND_TYPE_EFFECTS, percentage);
}

void sound_effect_play(sound_effect_type effect)
{
    if (!setting_sound(SOUND_TYPE_EFFECTS)->enabled) {
        return;
    }
    if (sound_device_is_file_playing_on_channel(effect_filenames[effect], SOUND_TYPE_EFFECTS)) {
        return;
    }
    sound_device_play_file_on_channel(effect_filenames[effect], SOUND_TYPE_EFFECTS,
        setting_sound(SOUND_TYPE_EFFECTS)->volume);
}
