#include "route.h"

#include "map/routing.h"
#include "map/routing_path.h"

#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE_STEP 600
#define MAX_PATH_LENGTH 500

static struct {
    int *figure_ids;
    uint8_t (*direction_paths)[MAX_PATH_LENGTH];
    int array_size;
} data;

void figure_route_clear_all(void)
{
    for (int i = 0; i < data.array_size; i++) {
        data.figure_ids[i] = 0;
        for (int j = 0; j < MAX_PATH_LENGTH; j++) {
            data.direction_paths[i][j] = 0;
        }
    }
}

void figure_route_clean(void)
{
    for (int i = 0; i < data.array_size; i++) {
        int figure_id = data.figure_ids[i];
        if (figure_id > 0 && figure_id < figure_count()) {
            const figure *f = figure_get(figure_id);
            if (f->state != FIGURE_STATE_ALIVE || f->routing_path_id != i) {
                data.figure_ids[i] = 0;
            }
        }
    }
}

static void create_arrays(int size)
{
    free(data.figure_ids);
    free(data.direction_paths);
    data.array_size = size;
    data.figure_ids = malloc(size * sizeof(int));
    data.direction_paths = malloc(size * sizeof(uint8_t) * MAX_PATH_LENGTH);
    memset(data.figure_ids, 0, size * sizeof(int));
    memset(data.direction_paths, 0, size * sizeof(uint8_t) * MAX_PATH_LENGTH);
}

static int expand_arrays(void)
{
    int *new_ids = realloc(data.figure_ids, (data.array_size + ARRAY_SIZE_STEP) * sizeof(int));
    if (!new_ids) {
        return 0;
    }
    data.figure_ids = new_ids;

    uint8_t *new_paths = realloc(data.direction_paths, (data.array_size + ARRAY_SIZE_STEP) * sizeof(uint8_t) * MAX_PATH_LENGTH);
    if (!new_paths) {
        return 0;
    }
    data.direction_paths = new_paths;

    memset(data.figure_ids + data.array_size, 0, ARRAY_SIZE_STEP * sizeof(int));
    memset(&data.direction_paths[data.array_size], 0, ARRAY_SIZE_STEP * sizeof(uint8_t) * MAX_PATH_LENGTH);
    int size = data.array_size;
    data.array_size += ARRAY_SIZE_STEP;

    return size;
}


static int get_first_available(void)
{
    for (int i = 1; i < data.array_size; i++) {
        if (data.figure_ids[i] == 0) {
            return i;
        }
    }
    return expand_arrays();
}

void figure_route_add(figure *f)
{
    f->routing_path_id = 0;
    f->routing_path_current_tile = 0;
    f->routing_path_length = 0;
    int direction_limit = 8;
    if (f->disallow_diagonal) {
        direction_limit = 4;
    }
    int path_id = get_first_available();
    if (!path_id) {
        return;
    }
    int path_length;
    if (f->is_boat) {
        if (f->is_boat == 2) { // flotsam
            map_routing_calculate_distances_water_flotsam(f->x, f->y);
            path_length = map_routing_get_path_on_water(data.direction_paths[path_id],
                f->destination_x, f->destination_y, 1);
        } else {
            map_routing_calculate_distances_water_boat(f->x, f->y);
            path_length = map_routing_get_path_on_water(data.direction_paths[path_id],
                f->destination_x, f->destination_y, 0);
        }
    } else {
        // land figure
        int can_travel;
        switch (f->terrain_usage) {
            case TERRAIN_USAGE_ENEMY:
                can_travel = map_routing_noncitizen_can_travel_over_land(f->x, f->y,
                    f->destination_x, f->destination_y, f->destination_building_id, 5000);
                if (!can_travel) {
                    can_travel = map_routing_noncitizen_can_travel_over_land(f->x, f->y,
                        f->destination_x, f->destination_y, 0, 25000);
                    if (!can_travel) {
                        can_travel = map_routing_noncitizen_can_travel_through_everything(
                            f->x, f->y, f->destination_x, f->destination_y);
                    }
                }
                break;
            case TERRAIN_USAGE_WALLS:
                can_travel = map_routing_can_travel_over_walls(f->x, f->y,
                    f->destination_x, f->destination_y);
                break;
            case TERRAIN_USAGE_ANIMAL:
                can_travel = map_routing_noncitizen_can_travel_over_land(f->x, f->y,
                    f->destination_x, f->destination_y, -1, 5000);
                break;
            case TERRAIN_USAGE_PREFER_ROADS:
                can_travel = map_routing_citizen_can_travel_over_road_garden(f->x, f->y,
                    f->destination_x, f->destination_y);
                if (!can_travel) {
                    can_travel = map_routing_citizen_can_travel_over_land(f->x, f->y,
                        f->destination_x, f->destination_y);
                }
                break;
            case TERRAIN_USAGE_ROADS:
                can_travel = map_routing_citizen_can_travel_over_road_garden(f->x, f->y,
                    f->destination_x, f->destination_y);
                break;
            default:
                can_travel = map_routing_citizen_can_travel_over_land(f->x, f->y,
                    f->destination_x, f->destination_y);
                break;
        }
        if (can_travel) {
            if (f->terrain_usage == TERRAIN_USAGE_WALLS) {
                path_length = map_routing_get_path(data.direction_paths[path_id], f->x, f->y,
                    f->destination_x, f->destination_y, 4);
                if (path_length <= 0) {
                    path_length = map_routing_get_path(data.direction_paths[path_id], f->x, f->y,
                        f->destination_x, f->destination_y, direction_limit);
                }
            } else {
                path_length = map_routing_get_path(data.direction_paths[path_id], f->x, f->y,
                    f->destination_x, f->destination_y, direction_limit);
            }
        } else { // cannot travel
            path_length = 0;
        }
    }
    if (path_length) {
        data.figure_ids[path_id] = f->id;
        f->routing_path_id = path_id;
        f->routing_path_length = path_length;
    }
}

void figure_route_remove(figure *f)
{
    if (f->routing_path_id > 0) {
        if (data.figure_ids[f->routing_path_id] == f->id) {
            data.figure_ids[f->routing_path_id] = 0;
        }
        f->routing_path_id = 0;
    }
}

int figure_route_get_direction(int path_id, int index)
{
    return data.direction_paths[path_id][index];
}

void figure_route_save_state(buffer *figures, buffer *paths)
{
    int size = data.array_size * sizeof(int);
    uint8_t *buf_data = malloc(size);
    buffer_init(figures, buf_data, size);

    size = data.array_size * sizeof(uint8_t) * MAX_PATH_LENGTH;
    buf_data = malloc(size);
    buffer_init(paths, buf_data, size);

    for (int i = 0; i < data.array_size; i++) {
        buffer_write_i16(figures, data.figure_ids[i]);
        buffer_write_raw(paths, data.direction_paths[i], MAX_PATH_LENGTH);
    }
}

void figure_route_load_state(buffer *figures, buffer *paths)
{
    int elements_to_load = paths->size / MAX_PATH_LENGTH;

    create_arrays(elements_to_load);

    // Reduce number of used paths on old Augustus savefiles that were hardcoded to load 3000. Improves performance
    int highest_id_in_use = 0;
    int reduce_array_size = elements_to_load == 3000;

    for (int i = 0; i < data.array_size; i++) {
        data.figure_ids[i] = buffer_read_i16(figures);
        buffer_read_raw(paths, data.direction_paths[i], MAX_PATH_LENGTH);
        if (reduce_array_size && data.figure_ids[i]) {
            highest_id_in_use = i;
        }
    }
    if (reduce_array_size) {
        data.array_size = ARRAY_SIZE_STEP;
        while (highest_id_in_use > data.array_size) {
            data.array_size += ARRAY_SIZE_STEP;
        }
    }
}
