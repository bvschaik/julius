#include "empire.h"

#include "assets/assets.h"
#include "building/count.h"
#include "city/constants.h"
#include "city/population.h"
#include "city/resource.h"
#include "core/calc.h"
#include "core/config.h"
#include "core/file.h"
#include "core/image.h"
#include "core/image_group.h"
#include "core/image_group_editor.h"
#include "core/io.h"
#include "core/log.h"
#include "editor/editor.h"
#include "empire/city.h"
#include "empire/object.h"
#include "empire/trade_route.h"
#include "game/campaign.h"
#include "game/save_version.h"
#include "scenario/empire.h"

#include <string.h>

enum {
    EMPIRE_HEADER_SIZE = 1280,
    EMPIRE_DATA_SIZE = 12800
};

static struct {
    int initial_scroll_x;
    int initial_scroll_y;
    int scroll_x;
    int scroll_y;
    int selected_object;
    int viewport_width;
    int viewport_height;
    struct {
        int id;
        char path[FILE_NAME_MAX];
        int offset_x;
        int offset_y;
        int width;
        int height;
    } image;
    struct {
        int relative;
        int x_offset;
        int y_offset;
    } coordinates;
} data;

static void set_image_id(const char *path)
{
    if (!path || !*path) {
        data.image.id = image_group(editor_is_active() ? GROUP_EDITOR_EMPIRE_MAP : GROUP_EMPIRE_MAP);
        data.image.path[0] = 0;
        return;
    }
    char *paths[] = {
        CAMPAIGNS_DIRECTORY "/image",
        "image",
        0
    };
    for (int i = 0; paths[i]; i++) {
        char full_path[FILE_NAME_MAX];
        const char *found_path = 0;
        snprintf(full_path, FILE_NAME_MAX, "%s/%s", paths[i], path);
        if (game_campaign_has_file(full_path)) {
            found_path = full_path;
        } else {
            found_path = dir_get_file_at_location(full_path, PATH_LOCATION_COMMUNITY);
        }
        if (found_path) {
            data.image.id = assets_get_external_image(found_path, 1);
            snprintf(data.image.path, FILE_NAME_MAX, "%s", path);
            return;
        }
    }
    log_error("Unable to find map image file", path, 0);
    data.image.id = image_group(editor_is_active() ? GROUP_EDITOR_EMPIRE_MAP : GROUP_EMPIRE_MAP);
    data.image.path[0] = 0;
}

void empire_set_custom_map(const char *path, int offset_x, int offset_y, int width, int height)
{
    set_image_id(path);
    if (offset_x < 0) {
        offset_x = 0;
    }
    if (offset_y < 0) {
        offset_y = 0;
    }
    const image *img = image_get(data.image.id);
    if (width > 0) {
        width += offset_x;
        if (width > img->original.width - offset_x) {
            width = img->original.width - offset_x;
        }
    } else {
        width = img->original.width - offset_x;
    }
    if (height > 0) {
        height += offset_y;
        if (height > img->original.height - offset_y) {
            height = img->original.height - offset_y;
        }
    } else {
        height = img->original.height - offset_y;
    }

    data.image.offset_x = offset_x;
    data.image.offset_y = offset_y;
    data.image.width = width;
    data.image.height = height;
}

void empire_reset_map(void)
{
    data.image.id = image_group(editor_is_active() ? GROUP_EDITOR_EMPIRE_MAP : GROUP_EMPIRE_MAP);
    data.image.path[0] = 0;
    const image *img = image_get(data.image.id);
    data.image.offset_x = 0;
    data.image.offset_y = 0;
    data.image.width = img->original.width - data.image.offset_x;
    data.image.height = img->original.height - data.image.offset_y;
}

void empire_load(int is_custom_scenario, int empire_id)
{
    // empire has already been loaded from the scenario or save file at this point
    if (empire_id == SCENARIO_CUSTOM_EMPIRE) {
        return;
    }

    empire_reset_map();
    empire_set_coordinates(0, 0, 0);

    char raw_data[EMPIRE_DATA_SIZE];
    const char *filename = is_custom_scenario ? "c32.emp" : "c3.emp";

    // read header with scroll positions
    if (!io_read_file_part_into_buffer(filename, NOT_LOCALIZED, raw_data, 4, 32 * empire_id)) {
        memset(raw_data, 0, 4);
    }
    buffer buf;
    buffer_init(&buf, raw_data, 4);
    data.initial_scroll_x = buffer_read_i16(&buf);
    data.initial_scroll_y = buffer_read_i16(&buf);

    // read data section with objects
    int offset = EMPIRE_HEADER_SIZE + EMPIRE_DATA_SIZE * empire_id;
    int read_size = io_read_file_part_into_buffer(filename, NOT_LOCALIZED, raw_data, EMPIRE_DATA_SIZE, offset);
    if (read_size != EMPIRE_DATA_SIZE) {
        // load empty empire when loading fails
        log_error("Unable to load empire data from file", filename, 0);
        memset(raw_data, 0, EMPIRE_DATA_SIZE);
    }
    buffer_init(&buf, raw_data, EMPIRE_DATA_SIZE);
    empire_object_load(&buf, SCENARIO_LAST_UNVERSIONED);
}

static void check_scroll_boundaries(void)
{
    int max_x = data.image.width + data.image.offset_x - data.viewport_width;
    int max_y = data.image.height + data.image.offset_y - data.viewport_height;

    data.scroll_x = calc_bound(data.scroll_x, data.image.offset_x, max_x);
    data.scroll_y = calc_bound(data.scroll_y, data.image.offset_y, max_y);
}

void empire_center_on_our_city(int viewport_width, int viewport_height)
{
    const empire_object *our_city = empire_object_get_our_city();

    data.viewport_width = viewport_width;
    data.viewport_height = viewport_height;
    if (our_city) {
        data.scroll_x = our_city->x - data.viewport_width / 2;
        data.scroll_y = our_city->y - data.viewport_height / 2;
    } else {
        data.scroll_x = data.initial_scroll_x;
        data.scroll_y = data.initial_scroll_y;
    }
    check_scroll_boundaries();
}

void empire_load_editor(int empire_id, int viewport_width, int viewport_height)
{
    empire_load(1, empire_id);
    empire_set_coordinates(0, 0, 0);
    empire_object_init_cities(empire_id);
    empire_clear_selected_object();
    empire_center_on_our_city(viewport_width, viewport_height);
}

void empire_init_scenario(void)
{
    data.scroll_x = data.initial_scroll_x;
    data.scroll_y = data.initial_scroll_y;
    data.viewport_width = data.image.width;
    data.viewport_height = data.image.height;

    empire_object_init_cities(scenario_empire_id());
}

void empire_set_viewport(int width, int height)
{
    data.viewport_width = width;
    data.viewport_height = height;
    check_scroll_boundaries();
}

int empire_get_image_id(void)
{
    return data.image.id;
}

void empire_get_map_size(int *width, int *height)
{
    *width = data.image.width;
    *height = data.image.height;
}

void empire_set_coordinates(int relative, int x_offset, int y_offset)
{
    data.coordinates.relative = relative;
    data.coordinates.x_offset = x_offset;
    data.coordinates.y_offset = y_offset;
}

void empire_transform_coordinates(int *x_coord, int *y_coord)
{
    if (data.coordinates.relative) {
        *x_coord += data.image.offset_x;
        *y_coord += data.image.offset_y;
    }
    *x_coord += data.coordinates.x_offset;
    *y_coord += data.coordinates.y_offset;
}

void empire_restore_coordinates(int *x_coord, int *y_coord)
{
    if (data.coordinates.relative) {
        *x_coord -= data.image.offset_x;
        *y_coord -= data.image.offset_y;
    }
    *x_coord -= data.coordinates.x_offset;
    *y_coord -= data.coordinates.y_offset;
}

void empire_adjust_scroll(int *x_offset, int *y_offset)
{
    *x_offset = *x_offset - data.scroll_x;
    *y_offset = *y_offset - data.scroll_y;
}

void empire_scroll_map(int x, int y)
{
    data.scroll_x += x;
    data.scroll_y += y;
    check_scroll_boundaries();
}

int empire_selected_object(void)
{
    return data.selected_object;
}

void empire_clear_selected_object(void)
{
    data.selected_object = 0;
}

void empire_select_object(int x, int y)
{
    int map_x = x + data.scroll_x;
    int map_y = y + data.scroll_y;

    data.selected_object = empire_object_get_closest(map_x, map_y);
}

int empire_can_export_resource_to_city(int city_id, int resource)
{
    if (!resource_is_storable(resource)) {
        return 0;
    }
    empire_city *city = empire_city_get(city_id);
    if (city_id && trade_route_limit_reached(city->route_id, resource)) {
        // quota reached
        return 0;
    }
    int in_stock = city_resource_count(resource);
    if (resource_is_food(resource) && config_get(CONFIG_GP_CH_ALLOW_EXPORTING_FROM_GRANARIES)) {
        in_stock += city_resource_count_food_on_granaries(resource) / RESOURCE_ONE_LOAD;
    }

    if (in_stock <= city_resource_export_over(resource)) {
        // stocks too low
        return 0;
    }
    if (city_id == 0 || city->buys_resource[resource]) {
        return (city_resource_trade_status(resource) & TRADE_STATUS_EXPORT) == TRADE_STATUS_EXPORT;
    } else {
        return 0;
    }
}

/**static int get_max_stock_for_population(void)
{
    int population = city_population();
    if (population < 2000) {
        return 10;
    } else if (population < 4000) {
        return 20;
    } else if (population < 6000) {
        return 30;
    } else {
        return 40;
    }
}**/

int empire_can_import_resource_from_city(int city_id, int resource)
{
    if (!resource_is_storable(resource)) {
        return 0;
    }
    empire_city *city = empire_city_get(city_id);
    if (!city->sells_resource[resource]) {
        return 0;
    }
    if (!(city_resource_trade_status(resource) & TRADE_STATUS_IMPORT)) {
        return 0;
    }
    if (trade_route_limit_reached(city->route_id, resource)) {
        return 0;
    }

    int in_stock = city_resource_count(resource);
    if (resource_is_food(resource)) {
        in_stock += city_resource_count_food_on_granaries(resource) / RESOURCE_ONE_LOAD;
    }
    int max_in_stock = 0;
    /* NOTE: don't forget to uncomment function get_max_stock_for_population
    
    int finished_good = RESOURCE_NONE; 
    switch (resource) {
        // food and finished materials
        case RESOURCE_WHEAT:
        case RESOURCE_VEGETABLES:
        case RESOURCE_FRUIT:
        case RESOURCE_MEAT:
        case RESOURCE_POTTERY:
        case RESOURCE_FURNITURE:
        case RESOURCE_OIL:
        case RESOURCE_WINE:
            max_in_stock = get_max_stock_for_population();
            break;

        case RESOURCE_MARBLE:
        case RESOURCE_WEAPONS:
            max_in_stock = 10;
            break;

        case RESOURCE_CLAY:
            finished_good = RESOURCE_POTTERY;
            break;
        case RESOURCE_TIMBER:
            finished_good = RESOURCE_FURNITURE;
            break;
        case RESOURCE_OLIVES:
            finished_good = RESOURCE_OIL;
            break;
        case RESOURCE_VINES:
            finished_good = RESOURCE_WINE;
            break;
        case RESOURCE_IRON:
            finished_good = RESOURCE_WEAPONS;
            break;
    }
    if (finished_good) {
        max_in_stock = 2 + 2 * building_count_industry_active(finished_good);
    }*/
    max_in_stock = city_resource_import_over(resource);
    return (max_in_stock == 0 || in_stock < max_in_stock) ? 1 : 0;
}

void empire_clear(void)
{
    data.selected_object = 0;
    empire_reset_map();
    empire_set_coordinates(0, 0, 0);
}

void empire_save_state(buffer *buf)
{
    buffer_write_i32(buf, data.scroll_x);
    buffer_write_i32(buf, data.scroll_y);
    buffer_write_i32(buf, data.selected_object);
}

void empire_load_state(buffer *buf)
{
    data.scroll_x = buffer_read_i32(buf);
    data.scroll_y = buffer_read_i32(buf);
    data.selected_object = buffer_read_i32(buf);
}

void empire_save_custom_map(buffer *buf)
{
    int path_length = (int) strlen(data.image.path) + 1;
    int buf_size = sizeof(int32_t) + 16 + path_length;
    uint8_t *buf_data = malloc(buf_size);

    buffer_init(buf, buf_data, buf_size);
    buffer_write_i32(buf, path_length);

    buffer_write_i32(buf, data.image.offset_x);
    buffer_write_i32(buf, data.image.offset_y);
    buffer_write_i32(buf, data.image.width);
    buffer_write_i32(buf, data.image.height);

    buffer_write_raw(buf, data.image.path, path_length);
}

void empire_load_custom_map(buffer *buf)
{
    int path_length = buffer_read_i32(buf);
    int offset_x = buffer_read_i32(buf);
    int offset_y = buffer_read_i32(buf);
    int width = buffer_read_i32(buf);
    int height = buffer_read_i32(buf);

    char path[FILE_NAME_MAX] = { 0 };
    buffer_read_raw(buf, path, path_length);

    empire_set_custom_map(path, offset_x, offset_y, width, height);
}
