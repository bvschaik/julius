#include "system.h"

#include "core/dir.h"
#include "game/settings.h"
#include "sound/channel.h"
#include "sound/city.h"
#include "sound/device.h"
#include "sound/effect.h"
#include "sound/music.h"
#include "sound/speech.h"

#include <string.h>

static char channel_filenames[SOUND_CHANNEL_MAX][CHANNEL_FILENAME_MAX] = {
    "", // speech channel
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
    "wavs/sheep_die.wav",
    "wavs/zebra_die.wav",
    "wavs/wolf_howl.wav",
    "wavs/fire_splash.wav",
    "wavs/formation_shield.wav",
    // city sounds
    "wavs/house_slum1.wav",
    "",
    "",
    "",
    "wavs/house_poor1.wav",
    "",
    "",
    "",
    "wavs/house_mid1.wav",
    "",
    "",
    "",
    "wavs/house_good1.wav",
    "",
    "",
    "",
    "wavs/house_posh1.wav",
    "",
    "",
    "",
    "wavs/ampitheatre.wav",
    "wavs/theatre.wav",
    "wavs/hippodrome.wav",
    "wavs/colloseum.wav",
    "wavs/glad_pit.wav",
    "wavs/lion_pit.wav",
    "wavs/art_pit.wav",
    "wavs/char_pit.wav",
    "wavs/gardens1.wav",
    "",
    "",
    "",
    "wavs/clinic.wav",
    "wavs/hospital.wav",
    "wavs/baths.wav",
    "wavs/barber.wav",
    "wavs/school.wav",
    "wavs/academy.wav",
    "wavs/library.wav",
    "wavs/prefecture.wav",
    "wavs/fort1.wav",
    "",
    "",
    "",
    "wavs/tower1.wav",
    "",
    "",
    "",
    "wavs/temp_farm.wav",
    "wavs/temp_ship.wav",
    "wavs/temp_comm.wav",
    "wavs/temp_war.wav",
    "wavs/temp_love.wav",
    "wavs/market1.wav",
    "",
    "",
    "",
    "wavs/granary1.wav",
    "",
    "wavs/warehouse1.wav",
    "",
    "wavs/shipyard1.wav",
    "",
    "wavs/dock1.wav",
    "",
    "wavs/wharf1.wav",
    "",
    "wavs/palace.wav",
    "wavs/eng_post.wav",
    "wavs/senate.wav",
    "wavs/forum.wav",
    "wavs/resevoir.wav",
    "wavs/fountain1.wav",
    "",
    "",
    "",
    "wavs/well1.wav",
    "",
    "",
    "",
    "wavs/mil_acad.wav",
    "wavs/oracle.wav",
    "wavs/burning_ruin.wav",
    "wavs/wheat_farm.wav",
    "wavs/veg_farm.wav",
    "wavs/figs_farm.wav",
    "wavs/olives_farm.wav",
    "wavs/vines_farm.wav",
    "wavs/meat_farm.wav",
    "wavs/quarry.wav",
    "wavs/mine.wav",
    "wavs/lumber_mill.wav",
    "wavs/clay_pit.wav",
    "wavs/wine_workshop.wav",
    "wavs/oil_workshop.wav",
    "wavs/weap_workshop.wav",
    "wavs/furn_workshop.wav",
    "wavs/pott_workshop.wav",
    "wavs/empty_land1.wav",
    "",
    "",
    "",
    "wavs/river.wav",
    "wavs/mission.wav",
};

static void correct_channel_filenames(void)
{
    for (int i = 1; i < SOUND_CHANNEL_MAX; i++) {
        if (!channel_filenames[i][0]) {
            continue;
        }

        char *original = channel_filenames[i];
        const char *corrected = dir_get_file(original, MAY_BE_LOCALIZED);
        if (!corrected) {
            channel_filenames[i][0] = 0;
        } else if (corrected != original) {
            strncpy(original, corrected, CHANNEL_FILENAME_MAX);
        }
    }
}

void sound_system_init(void)
{
    correct_channel_filenames();

    sound_device_open();
    sound_device_init_channels(SOUND_CHANNEL_MAX, channel_filenames);

    sound_city_set_volume(setting_sound(SOUND_CITY)->volume);
    sound_effect_set_volume(setting_sound(SOUND_EFFECTS)->volume);
    sound_music_set_volume(setting_sound(SOUND_MUSIC)->volume);
    sound_speech_set_volume(setting_sound(SOUND_SPEECH)->volume);
}

void sound_system_shutdown(void)
{
    sound_device_close();
}
