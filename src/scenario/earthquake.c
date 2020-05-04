#include "earthquake.h"

#include "building/building.h"
#include "building/destruction.h"
#include "city/message.h"
#include "core/calc.h"
#include "core/random.h"
#include "figuretype/missile.h"
#include "game/time.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "scenario/data.h"
#include "sound/effect.h"

static struct {
    int game_year;
    int month;
    int state;
    int duration;
    int max_duration;
    int delay;
    int max_delay;
    struct {
        int x;
        int y;
    } expand[4];
} data;

void scenario_earthquake_init(void)
{
    data.game_year = scenario.start_year + scenario.earthquake.year;
    data.month = 2 + (random_byte() & 7);
    switch (scenario.earthquake.severity) {
        default:
            data.max_duration = 0;
            data.max_delay = 0;
            break;
        case EARTHQUAKE_SMALL:
            data.max_duration = 25 + (random_byte() & 0x1f);
            data.max_delay = 10;
            break;
        case EARTHQUAKE_MEDIUM:
            data.max_duration = 100 + (random_byte() & 0x3f);
            data.max_delay = 8;
            break;
        case EARTHQUAKE_LARGE:
            data.max_duration = 250 + random_byte();
            data.max_delay = 6;
            break;
    }
    data.state = EVENT_NOT_STARTED;
    for (int i = 0; i < 4; i++) {
        data.expand[i].x = scenario.earthquake_point.x;
        data.expand[i].y = scenario.earthquake_point.y;
    }
}

static int can_advance_earthquake_to_tile(int x, int y)
{
    if (map_terrain_is(map_grid_offset(x, y), TERRAIN_ELEVATION | TERRAIN_ROCK | TERRAIN_WATER)) {
        return 0;
    } else {
        return 1;
    }
}

static void advance_earthquake_to_tile(int x, int y)
{
    int grid_offset = map_grid_offset(x, y);
    int building_id = map_building_at(grid_offset);
    if (building_id) {
        building_destroy_by_fire(building_get(building_id));
        sound_effect_play(SOUND_EFFECT_EXPLOSION);
        int ruin_id = map_building_at(grid_offset);
        if (ruin_id) {
            building_get(ruin_id)->state = BUILDING_STATE_DELETED_BY_GAME;
            map_building_set(grid_offset, 0);
        }
    }
    map_terrain_set(grid_offset, 0);
    map_tiles_set_earthquake(x, y);
    map_tiles_update_all_gardens();
    map_tiles_update_all_roads();
    map_tiles_update_all_plazas();

    map_routing_update_land();
    map_routing_update_walls();

    figure_create_explosion_cloud(x, y, 1);
}

void scenario_earthquake_process(void)
{
    if (scenario.earthquake.severity == EARTHQUAKE_NONE ||
        scenario.earthquake_point.x == -1 || scenario.earthquake_point.y == -1) {
        return;
    }
    if (data.state == EVENT_NOT_STARTED) {
        if (game_time_year() == data.game_year &&
            game_time_month() == data.month) {
            data.state = EVENT_IN_PROGRESS;
            data.duration = 0;
            data.delay = 0;
            advance_earthquake_to_tile(data.expand[0].x, data.expand[0].y);
            city_message_post(1, MESSAGE_EARTHQUAKE, 0,
                map_grid_offset(data.expand[0].x, data.expand[0].y));
        }
    } else if (data.state == EVENT_IN_PROGRESS) {
        data.delay++;
        if (data.delay >= data.max_delay) {
            data.delay = 0;
            data.duration++;
            if (data.duration >= data.max_duration) {
                data.state = EVENT_FINISHED;
            }
            int dx, dy, index;
            switch (random_byte() & 0xf) {
                case 0: index = 0; dx = 0; dy = -1; break;
                case 1: index = 1; dx = 1; dy = 0; break;
                case 2: index = 2; dx = 0; dy = 1; break;
                case 3: index = 3; dx = -1; dy = 0; break;
                case 4: index = 0; dx = 0; dy = -1; break;
                case 5: index = 0; dx = -1; dy = 0; break;
                case 6: index = 0; dx = 1; dy = 0; break;
                case 7: index = 1; dx = 1; dy = 0; break;
                case 8: index = 1; dx = 0; dy = -1; break;
                case 9: index = 1; dx = 0; dy = 1; break;
                case 10: index = 2; dx = 0; dy = 1; break;
                case 11: index = 2; dx = -1; dy = 0; break;
                case 12: index = 2; dx = 1; dy = 0; break;
                case 13: index = 3; dx = -1; dy = 0; break;
                case 14: index = 3; dx = 0; dy = -1; break;
                case 15: index = 3; dx = 0; dy = 1; break;
                default: return;
            }
            int x = calc_bound(data.expand[index].x + dx, 0, scenario.map.width - 1);
            int y = calc_bound(data.expand[index].y + dy, 0, scenario.map.height - 1);
            if (can_advance_earthquake_to_tile(x, y)) {
                data.expand[index].x = x;
                data.expand[index].y = y;
                advance_earthquake_to_tile(x, y);
            }
        }
    }
}

int scenario_earthquake_is_in_progress(void)
{
    return data.state == EVENT_IN_PROGRESS;
}

void scenario_earthquake_save_state(buffer *buf)
{
    buffer_write_i32(buf, data.game_year);
    buffer_write_i32(buf, data.month);
    buffer_write_i32(buf, data.state);
    buffer_write_i32(buf, data.duration);
    buffer_write_i32(buf, data.max_duration);
    buffer_write_i32(buf, data.max_delay);
    buffer_write_i32(buf, data.delay);
    for (int i = 0; i < 4; i++) {
        buffer_write_i32(buf, data.expand[i].x);
        buffer_write_i32(buf, data.expand[i].y);
    }
}

void scenario_earthquake_load_state(buffer *buf)
{
    data.game_year = buffer_read_i32(buf);
    data.month = buffer_read_i32(buf);
    data.state = buffer_read_i32(buf);
    data.duration = buffer_read_i32(buf);
    data.max_duration = buffer_read_i32(buf);
    data.max_delay = buffer_read_i32(buf);
    data.delay = buffer_read_i32(buf);
    for (int i = 0; i < 4; i++) {
        data.expand[i].x = buffer_read_i32(buf);
        data.expand[i].y = buffer_read_i32(buf);
    }
}

