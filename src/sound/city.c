#include "city.h"

#include "building/properties.h"
#include "city/figures.h"
#include "core/file.h"
#include "core/random.h"
#include "core/time.h"
#include "game/settings.h"
#include "sound/device.h"

#include <string.h>

typedef enum {
    SOUND_AMBIENT_NONE = 0,
    SOUND_AMBIENT_FIRST = 1,
    SOUND_AMBIENT_EMPTY_LAND1 = 1,
    SOUND_AMBIENT_EMPTY_LAND2,
    SOUND_AMBIENT_EMPTY_LAND3,
    SOUND_AMBIENT_RIVER,
    SOUND_AMBIENT_MAX
} sound_ambient_type;

typedef struct {
    int in_use;
    int available;
    int total_views;
    int views_threshold;
    int direction_views[5];
    union {
        sound_city_type city;
        sound_ambient_type ambient;
    } type;
    time_millis last_played_time;
    time_millis delay_millis;
    const char *filename;
} background_sound;

static struct {
    background_sound city_sounds[SOUND_CITY_MAX];
    background_sound ambient_sounds[SOUND_AMBIENT_MAX];
    time_millis last_update_time;
} data;

static const char city_sound_files[SOUND_CITY_MAX][FILE_NAME_MAX] = {
    "",
    "wavs/house_slum1.wav",
    "wavs/house_slum2.wav",
    "wavs/house_slum3.wav",
    "wavs/house_slum4.wav",
    "wavs/house_poor1.wav",
    "wavs/house_poor2.wav",
    "wavs/house_poor3.wav",
    "wavs/house_poor4.wav",
    "wavs/house_mid1.wav",
    "wavs/house_mid2.wav",
    "wavs/house_mid3.wav",
    "wavs/house_mid4.wav",
    "wavs/house_good1.wav",
    "wavs/house_good2.wav",
    "wavs/house_good3.wav",
    "wavs/house_good4.wav",
    "wavs/house_posh1.wav",
    "wavs/house_posh2.wav",
    "wavs/house_posh3.wav",
    "wavs/house_posh4.wav",
    "wavs/ampitheatre.wav",
    "wavs/theatre.wav",
    "wavs/hippodrome.wav",
    "wavs/colloseum.wav",
    "wavs/glad_pit.wav",
    "wavs/lion_pit.wav",
    "wavs/art_pit.wav",
    "wavs/char_pit.wav",
    "wavs/gardens1.wav",
    "wavs/clinic.wav",
    "wavs/hospital.wav",
    "wavs/baths.wav",
    "wavs/barber.wav",
    "wavs/school.wav",
    "wavs/academy.wav",
    "wavs/library.wav",
    ASSETS_DIRECTORY "/Sounds/Prefect.wav", // "wavs/prefecture.wav",
    "wavs/fort1.wav",
    "wavs/tower1.wav",
    "wavs/tower2.wav", // WATCHTOWER
    "wavs/tower3.wav", // ARMOURY
    "wavs/tower4.wav", // WORKCAMP
    "wavs/temp_farm.wav",
    "wavs/temp_ship.wav",
    "wavs/temp_comm.wav",
    "wavs/temp_war.wav",
    "wavs/temp_love.wav",
    "wavs/market1.wav",
    "wavs/market2.wav", // CARAVANSERAI
    "wavs/market3.wav", // TAVERN
    "wavs/granary1.wav",
    "wavs/warehouse1.wav",
    "wavs/warehouse2.wav", // MESS_HALL
    "wavs/shipyard1.wav",
    "wavs/dock1.wav",
    "wavs/wharf1.wav",
    "wavs/palace.wav",
    ASSETS_DIRECTORY "/Sounds/Engineer.wav", // "wavs/eng_post.wav",
    "wavs/senate.wav",
    "wavs/forum.wav",
    "wavs/resevoir.wav",
    "wavs/fountain1.wav",
    "wavs/well1.wav",
    "wavs/mil_acad.wav",
    "wavs/barracks.wav",
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
    "wavs/coin.wav",
    "wavs/mission.wav",
    ASSETS_DIRECTORY "/Sounds/Brickworks.wav",
    ASSETS_DIRECTORY "/Sounds/Lighthouse.wav",
    ASSETS_DIRECTORY "/Sounds/Ox.wav", // DEPOT
    ASSETS_DIRECTORY "/Sounds/ConcreteMaker.wav",
};

static const char ambient_sound_files[SOUND_AMBIENT_MAX][FILE_NAME_MAX] = {
    "",
    "wavs/empty_land1.wav",
    "wavs/empty_land2.wav",
    "wavs/empty_land3.wav",
    "wavs/river.wav",
};

void sound_city_init(void)
{
    data.last_update_time = time_get_millis();
    memset(data.city_sounds, 0, sizeof(data.city_sounds));
    for (sound_city_type sound = SOUND_CITY_FIRST; sound < SOUND_CITY_MAX; sound++) {
        data.city_sounds[sound].last_played_time = data.last_update_time;
        data.city_sounds[sound].type.city = sound;
        data.city_sounds[sound].in_use = 1;
        data.city_sounds[sound].views_threshold = 200;
        data.city_sounds[sound].delay_millis = 30000;
        data.city_sounds[sound].filename = city_sound_files[sound];
    }

    memset(data.ambient_sounds, 0, sizeof(data.ambient_sounds));
    for (sound_ambient_type sound = SOUND_AMBIENT_FIRST; sound < SOUND_AMBIENT_MAX; sound++) {
        data.ambient_sounds[sound].last_played_time = data.last_update_time;
        data.ambient_sounds[sound].type.ambient = sound;
        data.ambient_sounds[sound].in_use = 1;
        data.ambient_sounds[sound].views_threshold = 200;
        data.ambient_sounds[sound].delay_millis = 30000;
        data.ambient_sounds[sound].filename = ambient_sound_files[sound];
    }
}

void sound_city_set_volume(int percentage)
{
    sound_device_set_volume_for_type(SOUND_TYPE_CITY, percentage);
}

void sound_city_mark_building_view(building_type type, int num_workers, int direction)
{
    sound_city_type sound = building_properties_for_type(type)->sound_id;

    if (sound == SOUND_CITY_NONE) {
        return;
    }
    if (type == BUILDING_THEATER || type == BUILDING_AMPHITHEATER ||
        type == BUILDING_GLADIATOR_SCHOOL || type == BUILDING_HIPPODROME) {
        // entertainment is shut off when caesar invades
        if (num_workers <= 0 || city_figures_imperial_soldiers() > 0) {
            return;
        }
    }

    data.city_sounds[sound].available = 1;
    ++data.city_sounds[sound].total_views;
    ++data.city_sounds[sound].direction_views[direction];
}

void sound_city_mark_construction_site_view(int direction)
{
    data.city_sounds[SOUND_CITY_CONSTRUCTION_SITE].available = 1;
    ++data.city_sounds[SOUND_CITY_CONSTRUCTION_SITE].total_views;
    ++data.city_sounds[SOUND_CITY_CONSTRUCTION_SITE].direction_views[direction];
}

void sound_city_decay_views(void)
{
    for (sound_city_type sound = SOUND_CITY_FIRST; sound < SOUND_CITY_MAX; sound++) {
        for (int d = 0; d < 5; d++) {
            data.city_sounds[sound].direction_views[d] = 0;
        }
        data.city_sounds[sound].total_views /= 2;
    }

    for (sound_ambient_type sound = SOUND_AMBIENT_FIRST; sound < SOUND_AMBIENT_MAX; sound++) {
        for (int d = 0; d < 5; d++) {
            data.ambient_sounds[sound].direction_views[d] = 0;
        }
        data.ambient_sounds[sound].total_views /= 2;
    }
}

void sound_city_progress_ambient(void)
{
    for (sound_ambient_type sound = SOUND_AMBIENT_FIRST; sound < SOUND_AMBIENT_MAX; sound++) {
        data.ambient_sounds[sound].available = 1;
        ++data.ambient_sounds[sound].total_views;
        ++data.ambient_sounds[sound].direction_views[SOUND_DIRECTION_CENTER];
    }
}

static void play_sound(const background_sound *sound, int direction)
{
    if (!setting_sound(SOUND_TYPE_CITY)->enabled || !*sound->filename) {
        return;
    }
    if (sound_device_is_file_playing_on_channel(sound->filename, SOUND_TYPE_CITY)) {
        return;
    }
    int left_pan;
    int right_pan;
    switch (direction) {
        case SOUND_DIRECTION_CENTER:
            left_pan = right_pan = 100;
            break;
        case SOUND_DIRECTION_LEFT:
            left_pan = 100;
            right_pan = 0;
            break;
        case SOUND_DIRECTION_RIGHT:
            left_pan = 0;
            right_pan = 100;
            break;
        default:
            left_pan = right_pan = 0;
            break;
    }
    sound_device_play_file_on_channel_panned(sound->filename, SOUND_TYPE_CITY,
        setting_sound(SOUND_TYPE_CITY)->volume, left_pan, right_pan);
}

void sound_city_play(void)
{
    time_millis now = time_get_millis();
    time_millis max_delay = 0;
    background_sound *sound_to_play = 0;
    for (sound_city_type sound = SOUND_CITY_FIRST; sound < SOUND_CITY_MAX; sound++) {
        background_sound *current_sound = &data.city_sounds[sound];
        if (current_sound->available) {
            current_sound->available = 0;
            if (current_sound->total_views >= current_sound->views_threshold) {
                if (now - current_sound->last_played_time >= current_sound->delay_millis) {
                    if (now - current_sound->last_played_time > max_delay) {
                        max_delay = now - current_sound->last_played_time;
                        sound_to_play = current_sound;
                    }
                }
            }
        } else {
            current_sound->total_views = 0;
            for (int d = 0; d < 5; d++) {
                current_sound->direction_views[d] = 0;
            }
        }
    }

    if (now - data.last_update_time < 2000) {
        // Only play 1 sound every 2 seconds
        return;
    }

    if (!sound_to_play) {
        // progress_ambient();
        return;
    }

    // always only one channel available... use it
    int direction;
    if (sound_to_play->direction_views[SOUND_DIRECTION_CENTER] > 10) {
        direction = SOUND_DIRECTION_CENTER;
    } else if (sound_to_play->direction_views[SOUND_DIRECTION_LEFT] > 10) {
        direction = SOUND_DIRECTION_LEFT;
    } else if (sound_to_play->direction_views[SOUND_DIRECTION_RIGHT] > 10) {
        direction = SOUND_DIRECTION_RIGHT;
    } else {
        direction = SOUND_DIRECTION_CENTER;
    }

    play_sound(sound_to_play, direction);
    data.last_update_time = now;
    sound_to_play->last_played_time = now;
    sound_to_play->total_views = 0;
    for (int d = 0; d < 5; d++) {
        sound_to_play->direction_views[d] = 0;
    }
}
