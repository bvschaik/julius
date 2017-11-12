#include "empire.h"

#include "building/count.h"
#include "city/message.h"
#include "core/calc.h"
#include "core/io.h"
#include "empire/city.h"
#include "empire/object.h"
#include "empire/trade_route.h"
#include "game/time.h"

#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Scenario.h"

enum {
    EMPIRE_WIDTH = 2000,
    EMPIRE_HEIGHT = 1000
};

static struct {
    int initial_scroll_x;
    int initial_scroll_y;
    int scroll_x;
    int scroll_y;
    int selected_object;
    int viewport_width;
    int viewport_height;
} data;

void empire_load(int is_custom_scenario, int empire_id)
{
    char raw_data[12800];
    const char *filename = is_custom_scenario ? "c32.emp" : "c3.emp";
    
    // read header with scroll positions
    io_read_file_part_into_buffer(filename, raw_data, 4, 32 * empire_id);
    buffer buf;
    buffer_init(&buf, raw_data, 4);
    data.initial_scroll_x = buffer_read_i16(&buf);
    data.initial_scroll_y = buffer_read_i16(&buf);

    // read data section with objects
    int offset = 1280 + 12800 * empire_id;
    io_read_file_part_into_buffer(filename, raw_data, 12800, offset);
    buffer_init(&buf, raw_data, 12800);
    empire_object_load(&buf);
}

void empire_init_scenario()
{
    data.scroll_x = data.initial_scroll_x;
    data.scroll_y = data.initial_scroll_y;
    data.viewport_width = EMPIRE_WIDTH;
    data.viewport_height = EMPIRE_HEIGHT;

    empire_object_init_cities();
}

static void check_scroll_boundaries()
{
    int max_x = EMPIRE_WIDTH - data.viewport_width;
    int max_y = EMPIRE_HEIGHT - data.viewport_height;

    data.scroll_x = calc_bound(data.scroll_x, 0, max_x - 1);
    data.scroll_y = calc_bound(data.scroll_y, 0, max_y - 1);
}

void empire_set_viewport(int width, int height)
{
    data.viewport_width = width;
    data.viewport_height = height;
    check_scroll_boundaries();
}

void empire_adjust_scroll(int *x_offset, int *y_offset)
{
    *x_offset = *x_offset - data.scroll_x;
    *y_offset = *y_offset - data.scroll_y;
}

void empire_scroll_map(int direction)
{
    if (direction == Dir_8_None) {
        return;
    }
    switch (direction) {
        case Dir_0_Top:
            data.scroll_y -= 20;
            break;
        case Dir_1_TopRight:
            data.scroll_x += 20;
            data.scroll_y -= 20;
            break;
        case Dir_2_Right:
            data.scroll_x += 20;
            break;
        case Dir_3_BottomRight:
            data.scroll_x += 20;
            data.scroll_y += 20;
            break;
        case Dir_4_Bottom:
            data.scroll_y += 20;
            break;
        case Dir_5_BottomLeft:
            data.scroll_x -= 20;
            data.scroll_y += 20;
            break;
        case Dir_6_Left:
            data.scroll_x -= 20;
            break;
        case Dir_7_TopLeft:
            data.scroll_x -= 20;
            data.scroll_y -= 20;
            break;
    };
    check_scroll_boundaries();
}

int empire_selected_object()
{
    return data.selected_object;
}

void empire_clear_selected_object()
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
    empire_city *city = empire_city_get(city_id);
    if (city_id && trade_route_limit_reached(city->route_id, resource)) {
        // quota reached
        return 0;
    }
    if (Data_CityInfo.resourceStored[resource] <= Data_CityInfo.resourceTradeExportOver[resource]) {
        // stocks too low
        return 0;
    }
    if (city_id == 0 || city->buys_resource[resource]) {
        return Data_CityInfo.resourceTradeStatus[resource] == TradeStatus_Export;
    } else {
        return 0;
    }
}

int empire_can_import_resource_from_city(int city_id, int resource)
{
    empire_city *city = empire_city_get(city_id);
    if (!city->sells_resource[resource]) {
        return 0;
    }
    if (Data_CityInfo.resourceTradeStatus[resource] != TradeStatus_Import) {
        return 0;
    }
    if (trade_route_limit_reached(city->route_id, resource)) {
        return 0;
    }

    int in_stock = Data_CityInfo.resourceStored[resource];
    int max_in_stock = 0;
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
            if (Data_CityInfo.population < 2000) {
                max_in_stock = 10;
            } else if (Data_CityInfo.population < 4000) {
                max_in_stock = 20;
            } else if (Data_CityInfo.population < 6000) {
                max_in_stock = 30;
            } else {
                max_in_stock = 40;
            }
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
    }
    return in_stock < max_in_stock ? 1 : 0;
}

void empire_handle_expand_event()
{
    if (Data_Scenario.empireHasExpanded || Data_Scenario.empireExpansionYear <= 0) {
        return;
    }
    if (game_time_year() < Data_Scenario.empireExpansionYear + Data_Scenario.startYear) {
        return;
    }

    empire_city_expand_empire();

    Data_Scenario.empireHasExpanded = 1;
    city_message_post(1, Message_77_EmpireHasExpanded, 0, 0);
}

void empire_determine_distant_battle_city()
{
    Data_CityInfo.distantBattleCityId = empire_city_get_vulnerable_roman();
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
