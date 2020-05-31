#include "city.h"

#include "city/figures.h"
#include "core/time.h"
#include "game/settings.h"
#include "sound/channel.h"
#include "sound/device.h"

#include <string.h>

#define MAX_CHANNELS 70

// for compatibility with the original game:
#define CITY_CHANNEL_OFFSET 15

enum {
    SOUND_CHANNEL_CITY_HOUSE_SLUM = 30,
    SOUND_CHANNEL_CITY_HOUSE_POOR = 34,
    SOUND_CHANNEL_CITY_HOUSE_MEDIUM = 38,
    SOUND_CHANNEL_CITY_HOUSE_GOOD = 42,
    SOUND_CHANNEL_CITY_HOUSE_POSH = 46,
    SOUND_CHANNEL_CITY_AMPHITHEATER = 50,
    SOUND_CHANNEL_CITY_THEATER = 51,
    SOUND_CHANNEL_CITY_HIPPODROME = 52,
    SOUND_CHANNEL_CITY_COLOSSEUM = 53,
    SOUND_CHANNEL_CITY_GLADIATOR_SCHOOL = 54,
    SOUND_CHANNEL_CITY_LION_PIT = 55,
    SOUND_CHANNEL_CITY_ACTOR_COLONY = 56,
    SOUND_CHANNEL_CITY_CHARIOT_MAKER = 57,
    SOUND_CHANNEL_CITY_GARDEN = 58,
    SOUND_CHANNEL_CITY_CLINIC = 62,
    SOUND_CHANNEL_CITY_HOSPITAL = 63,
    SOUND_CHANNEL_CITY_BATHHOUSE = 64,
    SOUND_CHANNEL_CITY_BARBER = 65,
    SOUND_CHANNEL_CITY_SCHOOL = 66,
    SOUND_CHANNEL_CITY_ACADEMY = 67,
    SOUND_CHANNEL_CITY_LIBRARY = 68,
    SOUND_CHANNEL_CITY_PREFECTURE = 69,
    SOUND_CHANNEL_CITY_FORT = 70,
    SOUND_CHANNEL_CITY_TOWER = 74,
    SOUND_CHANNEL_CITY_TEMPLE_CERES = 78,
    SOUND_CHANNEL_CITY_TEMPLE_NEPTUNE = 79,
    SOUND_CHANNEL_CITY_TEMPLE_MERCURY = 80,
    SOUND_CHANNEL_CITY_TEMPLE_MARS = 81,
    SOUND_CHANNEL_CITY_TEMPLE_VENUS = 82,
    SOUND_CHANNEL_CITY_MARKET = 83,
    SOUND_CHANNEL_CITY_GRANARY = 87,
    SOUND_CHANNEL_CITY_WAREHOUSE = 89,
    SOUND_CHANNEL_CITY_SHIPYARD = 91,
    SOUND_CHANNEL_CITY_DOCK = 93,
    SOUND_CHANNEL_CITY_WHARF = 95,
    SOUND_CHANNEL_CITY_PALACE = 97,
    SOUND_CHANNEL_CITY_ENGINEERS_POST = 98,
    SOUND_CHANNEL_CITY_SENATE = 99,
    SOUND_CHANNEL_CITY_FORUM = 100,
    SOUND_CHANNEL_CITY_RESERVOIR = 101,
    SOUND_CHANNEL_CITY_FOUNTAIN = 102,
    SOUND_CHANNEL_CITY_WELL = 106,
    SOUND_CHANNEL_CITY_MILITARY_ACADEMY = 110,
    SOUND_CHANNEL_CITY_ORACLE = 111,
    SOUND_CHANNEL_CITY_BURNING_RUIN = 112,
    SOUND_CHANNEL_CITY_WHEAT_FARM = 113,
    SOUND_CHANNEL_CITY_VEGETABLE_FARM = 114,
    SOUND_CHANNEL_CITY_FRUIT_FARM = 115,
    SOUND_CHANNEL_CITY_OLIVE_FARM = 116,
    SOUND_CHANNEL_CITY_VINE_FARM = 117,
    SOUND_CHANNEL_CITY_PIG_FARM = 118,
    SOUND_CHANNEL_CITY_QUARRY = 119,
    SOUND_CHANNEL_CITY_IRON_MINE = 120,
    SOUND_CHANNEL_CITY_TIMBER_YARD = 121,
    SOUND_CHANNEL_CITY_CLAY_PIT = 122,
    SOUND_CHANNEL_CITY_WINE_WORKSHOP = 123,
    SOUND_CHANNEL_CITY_OIL_WORKSHOP = 124,
    SOUND_CHANNEL_CITY_WEAPONS_WORKSHOP = 125,
    SOUND_CHANNEL_CITY_FURNITURE_WORKSHOP = 126,
    SOUND_CHANNEL_CITY_POTTERY_WORKSHOP = 127,
    SOUND_CHANNEL_CITY_EMPTY_LAND = 128,
    SOUND_CHANNEL_CITY_RIVER = 132,
    SOUND_CHANNEL_CITY_MISSION_POST = 133,
};

typedef struct {
    int in_use;
    int available;
    int total_views;
    int views_threshold;
    int direction_views[5];
    int channel;
    int times_played;
    time_millis last_played_time;
    time_millis delay_millis;
    int should_play;
} city_channel;

static city_channel channels[MAX_CHANNELS];

static const int BUILDING_TYPE_TO_CHANNEL_ID[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0-9
    1, 1, 1, 1, 1, 1, 2, 2, 2, 2, //10-19
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, //20-29
    6, 7, 8, 9, 10, 11, 12, 13, 0, 14, //30-39
    0, 0, 0, 0, 0, 0, 15, 16, 17, 18, //40-49
    0, 19, 20, 21, 0, 22, 0, 23, 24, 24, //50-59
    25, 26, 27, 28, 29, 25, 26, 27, 28, 29, //60-69
    30, 31, 32, 0, 33, 34, 35, 36, 36, 36, //70-79
    63, 37, 0, 0, 38, 38, 39, 39, 0, 0, // 80-89
    40, 0, 0, 0, 43, 0, 0, 0, 44, 45, //90-99
    46, 47, 48, 49, 50, 51, 52, 53, 54, 55, //100-109
    56, 57, 58, 59, 60, 0, 0, 0, 0, 0, //110-119
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //120-129
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //130-139
    0, 0, 0, 0, 0, 0 //140-145
};

static time_millis last_update_time;

void sound_city_init(void)
{
    last_update_time = time_get_millis();
    memset(channels, 0, MAX_CHANNELS * sizeof(city_channel));
    for (int i = 0; i < MAX_CHANNELS; i++) {
        channels[i].last_played_time = last_update_time;
    }
    for (int i = 1; i < 63; i++) {
        channels[i].in_use = 1;
        channels[i].views_threshold = 200;
        channels[i].delay_millis = 30000;
    }
    channels[1].channel = SOUND_CHANNEL_CITY_HOUSE_SLUM;
    channels[2].channel = SOUND_CHANNEL_CITY_HOUSE_POOR;
    channels[3].channel = SOUND_CHANNEL_CITY_HOUSE_MEDIUM;
    channels[4].channel = SOUND_CHANNEL_CITY_HOUSE_GOOD;
    channels[5].channel = SOUND_CHANNEL_CITY_HOUSE_POSH;
    channels[6].channel = SOUND_CHANNEL_CITY_AMPHITHEATER;
    channels[7].channel = SOUND_CHANNEL_CITY_THEATER;
    channels[8].channel = SOUND_CHANNEL_CITY_HIPPODROME;
    channels[9].channel = SOUND_CHANNEL_CITY_COLOSSEUM;
    channels[10].channel = SOUND_CHANNEL_CITY_GLADIATOR_SCHOOL;
    channels[11].channel = SOUND_CHANNEL_CITY_LION_PIT;
    channels[12].channel = SOUND_CHANNEL_CITY_ACTOR_COLONY;
    channels[13].channel = SOUND_CHANNEL_CITY_CHARIOT_MAKER;
    channels[14].channel = SOUND_CHANNEL_CITY_GARDEN;
    channels[15].channel = SOUND_CHANNEL_CITY_CLINIC;
    channels[16].channel = SOUND_CHANNEL_CITY_HOSPITAL;
    channels[17].channel = SOUND_CHANNEL_CITY_BATHHOUSE;
    channels[18].channel = SOUND_CHANNEL_CITY_BARBER;
    channels[19].channel = SOUND_CHANNEL_CITY_SCHOOL;
    channels[20].channel = SOUND_CHANNEL_CITY_ACADEMY;
    channels[21].channel = SOUND_CHANNEL_CITY_LIBRARY;
    channels[22].channel = SOUND_CHANNEL_CITY_PREFECTURE;
    channels[23].channel = SOUND_CHANNEL_CITY_FORT;
    channels[24].channel = SOUND_CHANNEL_CITY_TOWER;
    channels[25].channel = SOUND_CHANNEL_CITY_TEMPLE_CERES;
    channels[26].channel = SOUND_CHANNEL_CITY_TEMPLE_NEPTUNE;
    channels[27].channel = SOUND_CHANNEL_CITY_TEMPLE_MERCURY;
    channels[28].channel = SOUND_CHANNEL_CITY_TEMPLE_MARS;
    channels[29].channel = SOUND_CHANNEL_CITY_TEMPLE_VENUS;
    channels[30].channel = SOUND_CHANNEL_CITY_MARKET;
    channels[31].channel = SOUND_CHANNEL_CITY_GRANARY;
    channels[32].channel = SOUND_CHANNEL_CITY_WAREHOUSE;
    channels[33].channel = SOUND_CHANNEL_CITY_SHIPYARD;
    channels[34].channel = SOUND_CHANNEL_CITY_DOCK;
    channels[35].channel = SOUND_CHANNEL_CITY_WHARF;
    channels[36].channel = SOUND_CHANNEL_CITY_PALACE;
    channels[37].channel = SOUND_CHANNEL_CITY_ENGINEERS_POST;
    channels[38].channel = SOUND_CHANNEL_CITY_SENATE;
    channels[39].channel = SOUND_CHANNEL_CITY_FORUM;
    channels[40].channel = SOUND_CHANNEL_CITY_RESERVOIR;
    channels[41].channel = SOUND_CHANNEL_CITY_FOUNTAIN;
    channels[42].channel = SOUND_CHANNEL_CITY_WELL;
    channels[43].channel = SOUND_CHANNEL_CITY_MILITARY_ACADEMY;
    channels[44].channel = SOUND_CHANNEL_CITY_ORACLE;
    channels[45].channel = SOUND_CHANNEL_CITY_BURNING_RUIN;
    channels[46].channel = SOUND_CHANNEL_CITY_WHEAT_FARM;
    channels[47].channel = SOUND_CHANNEL_CITY_VEGETABLE_FARM;
    channels[48].channel = SOUND_CHANNEL_CITY_FRUIT_FARM;
    channels[49].channel = SOUND_CHANNEL_CITY_OLIVE_FARM;
    channels[50].channel = SOUND_CHANNEL_CITY_VINE_FARM;
    channels[51].channel = SOUND_CHANNEL_CITY_PIG_FARM;
    channels[52].channel = SOUND_CHANNEL_CITY_QUARRY;
    channels[53].channel = SOUND_CHANNEL_CITY_IRON_MINE;
    channels[54].channel = SOUND_CHANNEL_CITY_TIMBER_YARD;
    channels[55].channel = SOUND_CHANNEL_CITY_CLAY_PIT;
    channels[56].channel = SOUND_CHANNEL_CITY_WINE_WORKSHOP;
    channels[57].channel = SOUND_CHANNEL_CITY_OIL_WORKSHOP;
    channels[58].channel = SOUND_CHANNEL_CITY_WEAPONS_WORKSHOP;
    channels[59].channel = SOUND_CHANNEL_CITY_FURNITURE_WORKSHOP;
    channels[60].channel = SOUND_CHANNEL_CITY_POTTERY_WORKSHOP;
    channels[61].channel = SOUND_CHANNEL_CITY_EMPTY_LAND;
    channels[62].channel = SOUND_CHANNEL_CITY_RIVER;
    channels[63].channel = SOUND_CHANNEL_CITY_MISSION_POST;
}

void sound_city_set_volume(int percentage)
{
    for (int i = SOUND_CHANNEL_CITY_MIN; i <= SOUND_CHANNEL_CITY_MAX; i++) {
        sound_device_set_channel_volume(i, percentage);
    }
}

void sound_city_mark_building_view(building *b, int direction)
{
    if (b->state == BUILDING_STATE_UNUSED) {
        return;
    }
    int type = b->type;
    int channel = BUILDING_TYPE_TO_CHANNEL_ID[type];
    if (!channel) {
        return;
    }
    if (type == BUILDING_THEATER || type == BUILDING_AMPHITHEATER ||
        type == BUILDING_GLADIATOR_SCHOOL || type == BUILDING_HIPPODROME) {
        // entertainment is shut off when caesar invades
        if (b->num_workers <= 0 || city_figures_imperial_soldiers() > 0) {
            return;
        }
    }

    channels[channel].available = 1;
    ++channels[channel].total_views;
    ++channels[channel].direction_views[direction];
}

void sound_city_decay_views(void)
{
    for (int i = 0; i < MAX_CHANNELS; i++) {
        for (int d = 0; d < 5; d++) {
            channels[i].direction_views[d] = 0;
        }
        channels[i].total_views /= 2;
    }
}

static void play_channel(int channel, int direction)
{
    channel += CITY_CHANNEL_OFFSET;
    if (!setting_sound(SOUND_CITY)->enabled) {
        return;
    }
    if (sound_device_is_channel_playing(channel)) {
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
    sound_device_play_channel_panned(channel, setting_sound(SOUND_CITY)->volume, left_pan, right_pan);
}

void sound_city_play(void)
{
    time_millis now = time_get_millis();
    for (int i = 1; i < MAX_CHANNELS; i++) {
        channels[i].should_play = 0;
        if (channels[i].available) {
            channels[i].available = 0;
            if (channels[i].total_views >= channels[i].views_threshold) {
                if (now - channels[i].last_played_time >= channels[i].delay_millis) {
                    channels[i].should_play = 1;
                }
            }
        } else {
            channels[i].total_views = 0;
            for (int d = 0; d < 5; d++) {
                channels[i].direction_views[d] = 0;
            }
        }
    }

    if (now - last_update_time < 2000) {
        // Only play 1 sound every 2 seconds
        return;
    }
    time_millis max_delay = 0;
    int max_sound_id = 0;
    for (int i = 1; i < MAX_CHANNELS; i++) {
        if (channels[i].should_play) {
            if (now - channels[i].last_played_time > max_delay) {
                max_delay = now - channels[i].last_played_time;
                max_sound_id = i;
            }
        }
    }
    if (!max_sound_id) {
        return;
    }

    // always only one channel available... use it
    int channel = channels[max_sound_id].channel;
    int direction;
    if (channels[max_sound_id].direction_views[SOUND_DIRECTION_CENTER] > 10) {
        direction = SOUND_DIRECTION_CENTER;
    } else if (channels[max_sound_id].direction_views[SOUND_DIRECTION_LEFT] > 10) {
        direction = SOUND_DIRECTION_LEFT;
    } else if (channels[max_sound_id].direction_views[SOUND_DIRECTION_RIGHT] > 10) {
        direction = SOUND_DIRECTION_RIGHT;
    } else {
        direction = SOUND_DIRECTION_CENTER;
    }

    play_channel(channel, direction);
    last_update_time = now;
    channels[max_sound_id].last_played_time = now;
    channels[max_sound_id].total_views = 0;
    for (int d = 0; d < 5; d++) {
        channels[max_sound_id].direction_views[d] = 0;
    }
    channels[max_sound_id].times_played++;
}

void sound_city_save_state(buffer *buf)
{
    for (int i = 0; i < MAX_CHANNELS; i++) {
        const city_channel *ch = &channels[i];
        buffer_write_i32(buf, ch->available);
        buffer_write_i32(buf, ch->total_views);
        buffer_write_i32(buf, ch->views_threshold);
        for (int d = 0; d < 5; d++) {
            buffer_write_i32(buf, ch->direction_views[d]);
        }
        buffer_write_i32(buf, 0); // current channel, always 0
        buffer_write_i32(buf, ch->in_use ? 1 : 0); // num channels, max 1
        buffer_write_i32(buf, ch->channel);
        for (int c = 1; c < 8; c++) {
            buffer_write_i32(buf, 0); // channels 1-7: never used
        }
        buffer_write_i32(buf, ch->in_use);
        buffer_write_i32(buf, ch->times_played);
        buffer_write_u32(buf, ch->last_played_time);
        buffer_write_u32(buf, ch->delay_millis);
        buffer_write_i32(buf, ch->should_play);
        for (int x = 0; x < 9; x++) {
            buffer_write_i32(buf, 0);
        }
    }
}

void sound_city_load_state(buffer *buf)
{
    for (int i = 0; i < MAX_CHANNELS; i++) {
        city_channel *ch = &channels[i];
        ch->available = buffer_read_i32(buf);
        ch->total_views = buffer_read_i32(buf);
        ch->views_threshold = buffer_read_i32(buf);
        for (int d = 0; d < 5; d++) {
            ch->direction_views[d] = buffer_read_i32(buf);
        }
        buffer_skip(buf, 4); // current channel
        buffer_skip(buf, 4); // num channels
        ch->channel = buffer_read_i32(buf);
        buffer_skip(buf, 28);
        ch->in_use = buffer_read_i32(buf);
        ch->times_played = buffer_read_i32(buf);
        ch->last_played_time = buffer_read_u32(buf);
        ch->delay_millis = buffer_read_u32(buf);
        ch->should_play = buffer_read_i32(buf);
        buffer_skip(buf, 36);
    }
}
