#include "route.h"

#include "core/array.h"
#include "core/log.h"
#include "map/routing.h"
#include "map/routing_path.h"

#define ARRAY_SIZE_STEP 600
#define MAX_PATH_LENGTH 500

typedef struct {
    int id;
    int figure_id;
    uint8_t directions[MAX_PATH_LENGTH];
} figure_path_data;

static array(figure_path_data) paths;

static void create_new_path(figure_path_data *path, int position)
{
    path->id = position;
}

static int path_is_used(const figure_path_data *path)
{
    return path->figure_id != 0;
}

void figure_route_clear_all(void)
{
    paths.size = 0;
    array_trim(paths);
}

void figure_route_clean(void)
{
    figure_path_data *path;
    array_foreach(paths, path)
    {
        int figure_id = path->figure_id;
        if (figure_id > 0 && figure_id < figure_count()) {
            const figure *f = figure_get(figure_id);
            if (f->state != FIGURE_STATE_ALIVE || f->routing_path_id != i) {
                path->figure_id = 0;
            }
        }
    }
    array_trim(paths);
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
    if (!paths.blocks && !array_init(paths, ARRAY_SIZE_STEP, create_new_path, path_is_used)) {
        log_error("Unable to create paths array. The game will likely crash.", 0, 0);
        return;
    }
    figure_path_data *path;
    array_new_item(paths, 0, path);
    if (!path) {
        return;
    }
    int path_length;
    if (f->is_boat) {
        if (f->is_boat == 2) { // flotsam
            map_routing_calculate_distances_water_flotsam(f->x, f->y);
            path_length = map_routing_get_path_on_water(path->directions,
                f->destination_x, f->destination_y, 1);
        } else {
            map_routing_calculate_distances_water_boat(f->x, f->y);
            path_length = map_routing_get_path_on_water(path->directions,
                f->destination_x, f->destination_y, 0);
        }
    } else {
        // land figure
        int can_travel;
        switch (f->terrain_usage) {
            case TERRAIN_USAGE_ENEMY:
                // check to see if we can reach our destination by going around the city walls
                can_travel = map_routing_noncitizen_can_travel_over_land(f->x, f->y,
                    f->destination_x, f->destination_y, direction_limit, f->destination_building_id, 5000);
                if (!can_travel) {
                    can_travel = map_routing_noncitizen_can_travel_over_land(f->x, f->y,
                        f->destination_x, f->destination_y, direction_limit, 0, 25000);
                    if (!can_travel) {
                        can_travel = map_routing_noncitizen_can_travel_through_everything(
                            f->x, f->y, f->destination_x, f->destination_y, direction_limit);
                    }
                }
                break;
            case TERRAIN_USAGE_WALLS:
                can_travel = map_routing_can_travel_over_walls(f->x, f->y,
                    f->destination_x, f->destination_y, 4);
                break;
            case TERRAIN_USAGE_ANIMAL:
                can_travel = map_routing_noncitizen_can_travel_over_land(f->x, f->y,
                    f->destination_x, f->destination_y, direction_limit, -1, 5000);
                break;
            case TERRAIN_USAGE_PREFER_ROADS:
                can_travel = map_routing_citizen_can_travel_over_road_garden(f->x, f->y,
                    f->destination_x, f->destination_y, direction_limit);
                if (!can_travel) {
                    can_travel = map_routing_citizen_can_travel_over_land(f->x, f->y,
                        f->destination_x, f->destination_y, direction_limit);
                }
                break;
            case TERRAIN_USAGE_ROADS:
                can_travel = map_routing_citizen_can_travel_over_road_garden(f->x, f->y,
                    f->destination_x, f->destination_y, direction_limit);
                break;
            case TERRAIN_USAGE_PREFER_ROADS_HIGHWAY:
                can_travel = map_routing_citizen_can_travel_over_road_garden_highway(f->x, f->y,
                    f->destination_x, f->destination_y, direction_limit);
                if (!can_travel) {
                    can_travel = map_routing_citizen_can_travel_over_land(f->x, f->y,
                        f->destination_x, f->destination_y, direction_limit);
                }
                break;
            case TERRAIN_USAGE_ROADS_HIGHWAY:
                can_travel = map_routing_citizen_can_travel_over_road_garden_highway(f->x, f->y,
                    f->destination_x, f->destination_y, direction_limit);
                break;
            default:
                can_travel = map_routing_citizen_can_travel_over_land(f->x, f->y,
                    f->destination_x, f->destination_y, direction_limit);
                break;
        }
        if (can_travel) {
            if (f->terrain_usage == TERRAIN_USAGE_WALLS) {
                path_length = map_routing_get_path(path->directions, f->x, f->y,
                    f->destination_x, f->destination_y, 4);
                if (path_length <= 0) {
                    path_length = map_routing_get_path(path->directions, f->x, f->y,
                        f->destination_x, f->destination_y, direction_limit);
                }
            } else {
                path_length = map_routing_get_path(path->directions, f->x, f->y,
                    f->destination_x, f->destination_y, direction_limit);
            }
        } else { // cannot travel
            path_length = 0;
        }
    }
    if (path_length) {
        path->figure_id = f->id;
        f->routing_path_id = path->id;
        f->routing_path_length = path_length;
    }
}

void figure_route_remove(figure *f)
{
    if (f->routing_path_id > 0) {
        if (f->routing_path_id < paths.size && array_item(paths, f->routing_path_id)->figure_id == f->id) {
            array_item(paths, f->routing_path_id)->figure_id = 0;
        }
        f->routing_path_id = 0;
    }
    array_trim(paths);
}

int figure_route_get_direction(int path_id, int index)
{
    return array_item(paths, path_id)->directions[index];
}

void figure_route_save_state(buffer *figures, buffer *buf_paths)
{
    int size = paths.size * sizeof(int);
    uint8_t *buf_data = malloc(size);
    buffer_init(figures, buf_data, size);

    size = paths.size * sizeof(uint8_t) * MAX_PATH_LENGTH;
    buf_data = malloc(size);
    buffer_init(buf_paths, buf_data, size);

    figure_path_data *path;
    array_foreach(paths, path)
    {
        buffer_write_i16(figures, path->figure_id);
        buffer_write_raw(buf_paths, path->directions, MAX_PATH_LENGTH);
    }
}

void figure_route_load_state(buffer *figures, buffer *buf_paths)
{
    int elements_to_load = buf_paths->size / MAX_PATH_LENGTH;

    if (!array_init(paths, ARRAY_SIZE_STEP, create_new_path, path_is_used) ||
        !array_expand(paths, elements_to_load)) {
        log_error("Unable to create paths array. The game will likely crash.", 0, 0);
        return;
    }

    int highest_id_in_use = 0;

    for (int i = 0; i < elements_to_load; i++) {
        figure_path_data *path = array_next(paths);
        path->figure_id = buffer_read_i16(figures);
        buffer_read_raw(buf_paths, path->directions, MAX_PATH_LENGTH);
        if (path->figure_id) {
            highest_id_in_use = i;
        }
    }
    paths.size = highest_id_in_use + 1;
}
