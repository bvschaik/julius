#include "city.h"

#include "core/time.h"
#include "game/settings.h"
#include "sound/channel.h"
#include "sound/device.h"

#include "Data/CityInfo.h"

#include <string.h>

#define MAX_CHANNELS 70

// for compatibility with the original game:
#define CITY_CHANNEL_OFFSET 15

enum {
    SoundChannel_City_HouseSlum = 30,
    SoundChannel_City_HousePoor = 34,
    SoundChannel_City_HouseMedium = 38,
    SoundChannel_City_HouseGood = 42,
    SoundChannel_City_HousePosh = 46,
    SoundChannel_City_Amphitheater = 50,
    SoundChannel_City_Theater = 51,
    SoundChannel_City_Hippodrome = 52,
    SoundChannel_City_Colosseum = 53,
    SoundChannel_City_GladiatorSchool = 54,
    SoundChannel_City_LionPit = 55,
    SoundChannel_City_ActorColony = 56,
    SoundChannel_City_ChariotMaker = 57,
    SoundChannel_City_Garden = 58,
    SoundChannel_City_Clinic = 62,
    SoundChannel_City_Hospital = 63,
    SoundChannel_City_Bathhouse = 64,
    SoundChannel_City_Barber = 65,
    SoundChannel_City_School = 66,
    SoundChannel_City_Academy = 67,
    SoundChannel_City_Library = 68,
    SoundChannel_City_Prefecture = 69,
    SoundChannel_City_Fort = 70,
    SoundChannel_City_Tower = 74,
    SoundChannel_City_TempleCeres = 78,
    SoundChannel_City_TempleNeptune = 79,
    SoundChannel_City_TempleMercury = 80,
    SoundChannel_City_TempleMars = 81,
    SoundChannel_City_TempleVenus = 82,
    SoundChannel_City_Market = 83,
    SoundChannel_City_Granary = 87,
    SoundChannel_City_Warehouse = 89,
    SoundChannel_City_Shipyard = 91,
    SoundChannel_City_Dock = 93,
    SoundChannel_City_Wharf = 95,
    SoundChannel_City_Palace = 97,
    SoundChannel_City_EngineersPost = 98,
    SoundChannel_City_Senate = 99,
    SoundChannel_City_Forum = 100,
    SoundChannel_City_Reservoir = 101,
    SoundChannel_City_Fountain = 102,
    SoundChannel_City_Well = 106,
    SoundChannel_City_MilitaryAcademy = 110,
    SoundChannel_City_Oracle = 111,
    SoundChannel_City_BurningRuin = 112,
    SoundChannel_City_WheatFarm = 113,
    SoundChannel_City_VegetableFarm = 114,
    SoundChannel_City_FruitFarm = 115,
    SoundChannel_City_OliveFarm = 116,
    SoundChannel_City_VineFarm = 117,
    SoundChannel_City_PigFarm = 118,
    SoundChannel_City_Quarry = 119,
    SoundChannel_City_IronMine = 120,
    SoundChannel_City_TimberYard = 121,
    SoundChannel_City_ClayPit = 122,
    SoundChannel_City_WineWorkshop = 123,
    SoundChannel_City_OilWorkshop = 124,
    SoundChannel_City_WeaponsWorkshop = 125,
    SoundChannel_City_FurnitureWorkshop = 126,
    SoundChannel_City_PotteryWorkshop = 127,
    SoundChannel_City_EmptyLand = 128,
    SoundChannel_City_River = 132,
    SoundChannel_City_MissionPost = 133,
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

void sound_city_init()
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
    channels[1].channel = SoundChannel_City_HouseSlum;
    channels[2].channel = SoundChannel_City_HousePoor;
    channels[3].channel = SoundChannel_City_HouseMedium;
    channels[4].channel = SoundChannel_City_HouseGood;
    channels[5].channel = SoundChannel_City_HousePosh;
    channels[6].channel = SoundChannel_City_Amphitheater;
    channels[7].channel = SoundChannel_City_Theater;
    channels[8].channel = SoundChannel_City_Hippodrome;
    channels[9].channel = SoundChannel_City_Colosseum;
    channels[10].channel = SoundChannel_City_GladiatorSchool;
    channels[11].channel = SoundChannel_City_LionPit;
    channels[12].channel = SoundChannel_City_ActorColony;
    channels[13].channel = SoundChannel_City_ChariotMaker;
    channels[14].channel = SoundChannel_City_Garden;
    channels[15].channel = SoundChannel_City_Clinic;
    channels[16].channel = SoundChannel_City_Hospital;
    channels[17].channel = SoundChannel_City_Bathhouse;
    channels[18].channel = SoundChannel_City_Barber;
    channels[19].channel = SoundChannel_City_School;
    channels[20].channel = SoundChannel_City_Academy;
    channels[21].channel = SoundChannel_City_Library;
    channels[22].channel = SoundChannel_City_Prefecture;
    channels[23].channel = SoundChannel_City_Fort;
    channels[24].channel = SoundChannel_City_Tower;
    channels[25].channel = SoundChannel_City_TempleCeres;
    channels[26].channel = SoundChannel_City_TempleNeptune;
    channels[27].channel = SoundChannel_City_TempleMercury;
    channels[28].channel = SoundChannel_City_TempleMars;
    channels[29].channel = SoundChannel_City_TempleVenus;
    channels[30].channel = SoundChannel_City_Market;
    channels[31].channel = SoundChannel_City_Granary;
    channels[32].channel = SoundChannel_City_Warehouse;
    channels[33].channel = SoundChannel_City_Shipyard;
    channels[34].channel = SoundChannel_City_Dock;
    channels[35].channel = SoundChannel_City_Wharf;
    channels[36].channel = SoundChannel_City_Palace;
    channels[37].channel = SoundChannel_City_EngineersPost;
    channels[38].channel = SoundChannel_City_Senate;
    channels[39].channel = SoundChannel_City_Forum;
    channels[40].channel = SoundChannel_City_Reservoir;
    channels[41].channel = SoundChannel_City_Fountain;
    channels[42].channel = SoundChannel_City_Well;
    channels[43].channel = SoundChannel_City_MilitaryAcademy;
    channels[44].channel = SoundChannel_City_Oracle;
    channels[45].channel = SoundChannel_City_BurningRuin;
    channels[46].channel = SoundChannel_City_WheatFarm;
    channels[47].channel = SoundChannel_City_VegetableFarm;
    channels[48].channel = SoundChannel_City_FruitFarm;
    channels[49].channel = SoundChannel_City_OliveFarm;
    channels[50].channel = SoundChannel_City_VineFarm;
    channels[51].channel = SoundChannel_City_PigFarm;
    channels[52].channel = SoundChannel_City_Quarry;
    channels[53].channel = SoundChannel_City_IronMine;
    channels[54].channel = SoundChannel_City_TimberYard;
    channels[55].channel = SoundChannel_City_ClayPit;
    channels[56].channel = SoundChannel_City_WineWorkshop;
    channels[57].channel = SoundChannel_City_OilWorkshop;
    channels[58].channel = SoundChannel_City_WeaponsWorkshop;
    channels[59].channel = SoundChannel_City_FurnitureWorkshop;
    channels[60].channel = SoundChannel_City_PotteryWorkshop;
    channels[61].channel = SoundChannel_City_EmptyLand;
    channels[62].channel = SoundChannel_City_River;
    channels[63].channel = SoundChannel_City_MissionPost;
}

void sound_city_set_volume(int percentage)
{
    for (int i = SOUND_CHANNEL_CITY_MIN; i <= SOUND_CHANNEL_CITY_MAX; i++) {
        if (sound_device_has_channel(i)) {
            sound_device_set_channel_volume(i, percentage);
        }
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
        if (b->numWorkers <= 0 || Data_CityInfo.numImperialSoldiersInCity > 0) {
            return;
        }
    }

    channels[channel].available = 1;
    ++channels[channel].total_views;
    ++channels[channel].direction_views[direction];
}

void sound_city_decay_views()
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
    if (!sound_device_has_channel(channel) || sound_device_is_channel_playing(channel)) {
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
    sound_device_set_channel_panning(channel, left_pan, right_pan);
    sound_device_play_channel(channel);
}

void sound_city_play()
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
