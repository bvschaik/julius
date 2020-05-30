#include "natives.h"

#include "building/building.h"
#include "building/list.h"
#include "city/buildings.h"
#include "city/military.h"
#include "core/calc.h"
#include "core/image.h"
#include "core/image_group_editor.h"
#include "map/building.h"
#include "map/building_tiles.h"
#include "map/data.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/property.h"
#include "map/random.h"
#include "map/terrain.h"
#include "scenario/building.h"

static void mark_native_land(int x, int y, int size, int radius)
{
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, size, radius, &x_min, &y_min, &x_max, &y_max);
    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            map_property_mark_native_land(map_grid_offset(xx, yy));
        }
    }
}

static int has_building_on_native_land(int x, int y, int size, int radius)
{
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, size, radius, &x_min, &y_min, &x_max, &y_max);
    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            int building_id = map_building_at(map_grid_offset(xx, yy));
            if (building_id > 0) {
                int type = building_get(building_id)->type;
                if (type != BUILDING_MISSION_POST &&
                    type != BUILDING_NATIVE_HUT &&
                    type != BUILDING_NATIVE_MEETING &&
                    type != BUILDING_NATIVE_CROPS &&
		    type != BUILDING_ROADBLOCK
		    ) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

static void determine_meeting_center(void)
{
    // gather list of meeting centers
    building_list_small_clear();
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_NATIVE_MEETING) {
            building_list_small_add(i);
        }
    }
    int total_meetings = building_list_small_size();
    if (total_meetings <= 0) {
        return;
    }
    const int *meetings = building_list_small_items();
    // determine closest meeting center for hut
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_NATIVE_HUT) {
            int min_dist = 1000;
            int min_meeting_id = 0;
            for (int n = 0; n < total_meetings; n++) {
                building *meeting = building_get(meetings[n]);
                int dist = calc_maximum_distance(b->x, b->y, meeting->x, meeting->y);
                if (dist < min_dist) {
                    min_dist = dist;
                    min_meeting_id = meetings[n];
                }
            }
            b->subtype.native_meeting_center_id = min_meeting_id;
        }
    }
}

void map_natives_init(void)
{
    int meeting_center_set = 0;
    int image_hut = scenario_building_image_native_hut();
    int image_meeting = scenario_building_image_native_meeting();
    int image_crops = scenario_building_image_native_crops();
    int native_image = image_group(GROUP_BUILDING_NATIVE);
    int grid_offset = map_data.start_offset;
    for (int y = 0; y < map_data.height; y++, grid_offset += map_data.border_size) {
        for (int x = 0; x < map_data.width; x++, grid_offset++) {
            if (!map_terrain_is(grid_offset, TERRAIN_BUILDING) || map_building_at(grid_offset)) {
                continue;
            }

            int random_bit = map_random_get(grid_offset) & 1;
            int type;
            int image_id = map_image_at(grid_offset);
            if (image_id == image_hut) {
                type = BUILDING_NATIVE_HUT;
                map_image_set(grid_offset, native_image);
            } else if (image_id == image_hut + 1) {
                type = BUILDING_NATIVE_HUT;
                map_image_set(grid_offset, native_image + 1);
            } else if (image_id == image_meeting) {
                type = BUILDING_NATIVE_MEETING;
                map_image_set(grid_offset, native_image + 2);
                map_image_set(grid_offset + map_grid_delta(1, 0), native_image + 2);
                map_image_set(grid_offset + map_grid_delta(0, 1), native_image + 2);
                map_image_set(grid_offset + map_grid_delta(1, 1), native_image + 2);
            } else if (image_id == image_crops) {
                type = BUILDING_NATIVE_CROPS;
                map_image_set(grid_offset, image_group(GROUP_BUILDING_FARM_CROPS) + random_bit);
            } else { //unknown building
                map_building_tiles_remove(0, x, y);
                continue;
            }
            building *b = building_create(type, x, y);
            map_building_set(grid_offset, b->id);
            b->state = BUILDING_STATE_IN_USE;
            switch (type) {
                case BUILDING_NATIVE_CROPS:
                    b->data.industry.progress = random_bit;
                    break;
                case BUILDING_NATIVE_MEETING:
                    b->sentiment.native_anger = 100;
                    map_building_set(grid_offset + map_grid_delta(1, 0), b->id);
                    map_building_set(grid_offset + map_grid_delta(0, 1), b->id);
                    map_building_set(grid_offset + map_grid_delta(1, 1), b->id);
                    mark_native_land(b->x, b->y, 2, 6);
                    if (!meeting_center_set) {
                        city_buildings_set_main_native_meeting_center(b->x, b->y);
                    }
                    break;
                case BUILDING_NATIVE_HUT:
                    b->sentiment.native_anger = 100;
                    b->figure_spawn_delay = random_bit;
                    mark_native_land(b->x, b->y, 1, 3);
                    break;
            }
        }
    }

    determine_meeting_center();
}

void map_natives_init_editor(void)
{
    int image_hut = scenario_building_image_native_hut();
    int image_meeting = scenario_building_image_native_meeting();
    int image_crops = scenario_building_image_native_crops();
    int native_image = image_group(GROUP_EDITOR_BUILDING_NATIVE);
    int grid_offset = map_data.start_offset;
    for (int y = 0; y < map_data.height; y++, grid_offset += map_data.border_size) {
        for (int x = 0; x < map_data.width; x++, grid_offset++) {
            if (!map_terrain_is(grid_offset, TERRAIN_BUILDING) || map_building_at(grid_offset)) {
                continue;
            }

            int type;
            int image_id = map_image_at(grid_offset);
            if (image_id == image_hut) {
                type = BUILDING_NATIVE_HUT;
                map_image_set(grid_offset, native_image);
            } else if (image_id == image_hut + 1) {
                type = BUILDING_NATIVE_HUT;
                map_image_set(grid_offset, native_image + 1);
            } else if (image_id == image_meeting) {
                type = BUILDING_NATIVE_MEETING;
                map_image_set(grid_offset, native_image + 2);
                map_image_set(grid_offset + map_grid_delta(1, 0), native_image + 2);
                map_image_set(grid_offset + map_grid_delta(0, 1), native_image + 2);
                map_image_set(grid_offset + map_grid_delta(1, 1), native_image + 2);
            } else if (image_id == image_crops) {
                type = BUILDING_NATIVE_CROPS;
                map_image_set(grid_offset, image_group(GROUP_EDITOR_BUILDING_CROPS));
            } else { //unknown building
                map_building_tiles_remove(0, x, y);
                continue;
            }
            building *b = building_create(type, x, y);
            b->state = BUILDING_STATE_IN_USE;
            map_building_set(grid_offset, b->id);
            if (type == BUILDING_NATIVE_MEETING) {
                map_building_set(grid_offset + map_grid_delta(1, 0), b->id);
                map_building_set(grid_offset + map_grid_delta(0, 1), b->id);
                map_building_set(grid_offset + map_grid_delta(1, 1), b->id);
            }
        }
    }
}

void map_natives_check_land(void)
{
    map_property_clear_all_native_land();
    city_military_decrease_native_attack_duration();

    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        int size, radius;
        if (b->type == BUILDING_NATIVE_HUT) {
            size = 1;
            radius = 3;
        } else if (b->type == BUILDING_NATIVE_MEETING) {
            size = 2;
            radius = 6;
        } else {
            continue;
        }
        if (b->sentiment.native_anger >= 100) {
            mark_native_land(b->x, b->y, size, radius);
            if (has_building_on_native_land(b->x, b->y, size, radius)) {
                city_military_start_native_attack();
            }
        } else {
            b->sentiment.native_anger++;
        }
    }
}
