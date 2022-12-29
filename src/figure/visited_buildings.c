#include "visited_buildings.h"

#include "building/dock.h"
#include "city/data_private.h"
#include "core/array.h"
#include "core/log.h"
#include "figure/figure.h"

#define VISITED_BUILDINGS_ARRAY_SIZE_STEP 100
#define VISITED_BUILDINGS_BUFFER_SIZE (sizeof(int32_t) * 2)

static array(visited_building) visited_buildings;

static void visited_building_create(visited_building *visited, int index)
{
    visited->index = index;
}

static int visited_building_in_use(const visited_building *visited)
{
    return visited->building_id != 0;
}

void figure_visited_buildings_init(void)
{
    if (!array_init(visited_buildings, VISITED_BUILDINGS_ARRAY_SIZE_STEP, visited_building_create, visited_building_in_use)) {
        log_error("Unable to allocate enough memory for the visited docks array. The game will now crash.", 0, 0);
    }
}

int figure_visited_building_in_list(int index, int building_id)
{
    while (index) {
        const visited_building *visited = array_item(visited_buildings, index);
        if (visited->building_id == building_id) {
            return 1;
        }
        index = visited->prev_index;
    }
    return 0;
}

int figure_visited_buildings_add(int index, int building_id)
{
    if (figure_visited_building_in_list(index, building_id)) {
        return index;
    }
    visited_building *visited;
    array_new_item(visited_buildings, 1, visited);
    visited->building_id = building_id;
    visited->prev_index = index;
    return visited->index;
}

void figure_visited_buildings_remove_list(int index)
{
    while (index) {
        visited_building *visited = array_item(visited_buildings, index);
        index = visited->prev_index;
        visited->prev_index = 0;
        visited->building_id = 0;
    }
    array_trim(visited_buildings);
}

void figure_visited_buildings_save_state(buffer *buf)
{
    int buf_size = sizeof(int32_t) + visited_buildings.size * VISITED_BUILDINGS_BUFFER_SIZE;
    uint8_t *buf_data = malloc(buf_size);
    buffer_init(buf, buf_data, buf_size);
    buffer_write_i32(buf, VISITED_BUILDINGS_BUFFER_SIZE);
    const visited_building *visited;
    array_foreach(visited_buildings, visited)
    {
        buffer_write_i32(buf, visited->building_id);
        buffer_write_i32(buf, visited->prev_index);
    }
}

void figure_visited_buildings_load_state(buffer *buf)
{
    int visited_buildings_to_load = (buf->size - sizeof(int32_t)) / buffer_read_i32(buf);

    if (!array_init(visited_buildings, VISITED_BUILDINGS_ARRAY_SIZE_STEP, visited_building_create, visited_building_in_use) ||
        !array_expand(visited_buildings, visited_buildings_to_load)) {
        log_error("Unable to allocate enough memory for the visited docks array. The game will now crash.", 0, 0);
    }
    for (int i = 0; i < visited_buildings_to_load; i++) {
        visited_building *visited = array_next(visited_buildings);
        visited->building_id = buffer_read_i32(buf);
        visited->prev_index = buffer_read_i32(buf);
    }
}

void figure_visited_buildings_migrate(void)
{
    if (!array_init(visited_buildings, VISITED_BUILDINGS_ARRAY_SIZE_STEP, visited_building_create, visited_building_in_use)) {
        log_error("Unable to allocate enough memory for the visited docks array. The game will now crash.", 0, 0);
    }
    for (int i = 0; i < figure_count(); i++) {
        figure *f = figure_get(i);
        if (f->type != FIGURE_TRADE_SHIP || f->state == FIGURE_STATE_DEAD || !f->building_id) {
            continue;
        }
        for (int j = 0; j < 10; j++) { // Magic number: 10 is the original allowed maximum number of docks
            int dock_id = city_data.building.legacy_working_dock_ids[j];
            if (!dock_id) {
                continue;
            }
            if (f->building_id & (1 << j)) {
                visited_building *visited;
                array_new_item(visited_buildings, 1, visited);
                visited->building_id = dock_id;
                visited->prev_index = f->last_visited_index;
                f->last_visited_index = visited->index;
            }
        }
        f->building_id = 0;
    }
}
