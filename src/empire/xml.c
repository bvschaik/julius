#include "xml.h"

#include "assets/assets.h"
#include "core/buffer.h"
#include "core/file.h"
#include "core/image_group.h"
#include "core/log.h"
#include "core/string.h"
#include "core/xml_parser.h"
#include "core/zlib_helper.h"
#include "empire/city.h"
#include "empire/object.h"
#include "scenario/data.h"
#include "scenario/empire.h"

#include <stdio.h>
#include <string.h>

#define XML_TOTAL_ELEMENTS 11

typedef enum {
    LIST_NONE = -1,
    LIST_BUYS = 1,
    LIST_SELLS = 2,
    LIST_TRADE_WAYPOINTS = 3
} city_list;

static struct {
    int success;
    int version;
    int next_empire_obj_id;
    int current_city_id;
    int current_trade_route_id;
    city_list current_city_list;
    int current_invasion_path_id;
    int invasion_path_ids[10];
    int invasion_path_idx;
} data;

static int xml_start_empire(void);
static int xml_start_city(void);
static int xml_start_buys(void);
static int xml_start_sells(void);
static int xml_start_waypoints(void);
static int xml_start_resource(void);
static int xml_start_trade_point(void);
static int xml_start_invasion_path(void);
static int xml_start_battle(void);

static void xml_end_city(void);
static void xml_end_sells_buys_or_waypoints(void);
static void xml_end_invasion_path(void);

static const xml_parser_element xml_elements[XML_TOTAL_ELEMENTS] = {
    { "empire", xml_start_empire },
    { "cities", 0, 0, "empire" },
    { "city", xml_start_city, xml_end_city, "cities" },
    { "buys", xml_start_buys, xml_end_sells_buys_or_waypoints, "city" },
    { "sells", xml_start_sells, xml_end_sells_buys_or_waypoints, "city" },
    { "resource", xml_start_resource, 0, "buys|sells" },
    { "trade_points", xml_start_waypoints, xml_end_sells_buys_or_waypoints, "city" },
    { "point", xml_start_trade_point, 0, "trade_points" },
    { "invasion_paths", 0, 0, "empire" },
    { "path", xml_start_invasion_path, xml_end_invasion_path, "invasion_paths"},
    { "battle", xml_start_battle, 0, "path"},
};

static int xml_start_empire(void)
{
    data.version = xml_parser_get_attribute_int("version");
    if (!data.version) {
        data.success = 0;
        log_error("No version set", 0, 0);
        return 0;
    }
    return 1;
}

static int xml_start_city(void)
{
    if (data.next_empire_obj_id + 1 >= MAX_EMPIRE_OBJECTS) {
        data.success = 0;
        log_error("Too many objects", 0, data.next_empire_obj_id);
        return 0;
    }

    if (xml_parser_get_total_attributes() < 2) {
        log_error("Wrong number of attributes for the city", 0, 0);
        return 0;
    }

    full_empire_object *city_obj = empire_object_get_full(data.next_empire_obj_id);
    city_obj->obj.id = data.next_empire_obj_id;
    data.current_city_id = data.next_empire_obj_id;
    data.next_empire_obj_id++;
    city_obj->in_use = 1;
    city_obj->obj.type = EMPIRE_OBJECT_CITY;
    city_obj->city_type = EMPIRE_CITY_TRADE;
    city_obj->obj.image_id = image_group(GROUP_EMPIRE_CITY_TRADE);
    city_obj->trade_route_cost = 500;
    city_obj->obj.width = 44;
    city_obj->obj.height = 36;

    static const char *city_types[5] = { "roman", "ours", "trade", 0, "distant" };
    static const char *trade_route_types[2] = { "land", "sea" };

    const char *name = xml_parser_get_attribute_string("name");
    if (name) {
        string_copy(string_from_ascii(name), city_obj->city_custom_name, sizeof(city_obj->city_custom_name));
    }

    city_obj->obj.x = xml_parser_get_attribute_int("x");
    city_obj->obj.y = xml_parser_get_attribute_int("y");

    int city_type = xml_parser_get_attribute_enum("type", city_types, 5, EMPIRE_CITY_DISTANT_ROMAN);
    if (city_type < EMPIRE_CITY_DISTANT_ROMAN) {
        city_obj->city_type = EMPIRE_CITY_TRADE;
    } else {
        city_obj->city_type = city_type;
    }
    switch (city_obj->city_type) {
        case EMPIRE_CITY_OURS:
            city_obj->obj.image_id = image_group(GROUP_EMPIRE_CITY);
            break;
        case EMPIRE_CITY_DISTANT_ROMAN:
            city_obj->obj.image_id = image_group(GROUP_EMPIRE_CITY_DISTANT_ROMAN);
            break;
        case EMPIRE_CITY_DISTANT_FOREIGN:
            city_obj->obj.image_id = image_group(GROUP_EMPIRE_FOREIGN_CITY);
            break;
        default:
            city_obj->obj.image_id = image_group(GROUP_EMPIRE_CITY_TRADE);
            break;
    }

    if (city_obj->city_type == EMPIRE_CITY_TRADE) {
        full_empire_object *route_obj = empire_object_get_full(data.next_empire_obj_id);
        route_obj->obj.id = data.next_empire_obj_id;
        city_obj->obj.trade_route_id = route_obj->obj.id;
        data.current_trade_route_id = route_obj->obj.id;
        data.next_empire_obj_id++;
        route_obj->in_use = 1;
        route_obj->obj.type = EMPIRE_OBJECT_LAND_TRADE_ROUTE;
        route_obj->obj.trade_route_id = city_obj->obj.trade_route_id;

        route_obj->obj.type = xml_parser_get_attribute_enum("trade_route_type",
            trade_route_types, 2, EMPIRE_OBJECT_LAND_TRADE_ROUTE);
        if (route_obj->obj.type < EMPIRE_OBJECT_LAND_TRADE_ROUTE) {
            route_obj->obj.type = EMPIRE_OBJECT_LAND_TRADE_ROUTE;
        }
        if (route_obj->obj.type == EMPIRE_OBJECT_SEA_TRADE_ROUTE) {
            route_obj->obj.image_id = image_group(GROUP_EMPIRE_TRADE_ROUTE_TYPE);
        } else {
            route_obj->obj.image_id = image_group(GROUP_EMPIRE_TRADE_ROUTE_TYPE) + 1;
        }

        city_obj->trade_route_cost = xml_parser_get_attribute_int("trade_route_cost");
        if (!city_obj->trade_route_cost) {
            city_obj->trade_route_cost = 500;
        }
    }

    return 1;
}

static int xml_start_buys(void)
{
    data.current_city_list = LIST_BUYS;
    return 1;
}

static int xml_start_sells(void)
{
    data.current_city_list = LIST_SELLS;
    return 1;
}

static int xml_start_waypoints(void)
{
    data.current_city_list = LIST_TRADE_WAYPOINTS;
    return 1;
}

static int xml_start_resource(void)
{
    if (data.current_city_id == -1) {
        data.success = 0;
        log_error("No active city when parsing resource", 0, 0);
        return 0;
    } else if (data.current_city_list != LIST_BUYS && data.current_city_list != LIST_SELLS) {
        data.success = 0;
        log_error("Resource not in buy or sell tag", 0, 0);
        return 0;
    }

    static const char *resource_types[] = {
        "wheat", "vegetables", "fruit", "olives", "vines", "meat|fish", "wine", "oil", "iron",
        "timber|wood", "clay", "marble", "weapons", "furniture", "pottery"
    };
    
    full_empire_object *city_obj = empire_object_get_full(data.current_city_id);

    if (!xml_parser_has_attribute("type")) {
        data.success = 0;
        log_error("Unable to find resource type attribute", 0, 0);
        return 0;
    }
    resource_type resource = xml_parser_get_attribute_enum("type", resource_types, 15, RESOURCE_WHEAT);
    if (resource == RESOURCE_NONE) {
        data.success = 0;
        log_error("Unable to determine resource type", xml_parser_get_attribute_string("type"), 0);
        return 0;
    }

    int amount = xml_parser_has_attribute("amount") ?
        xml_parser_get_attribute_int("amount") : 1;

    if (data.current_city_list == LIST_BUYS) {
        city_obj->city_buys_resource[resource] = amount;
    } else if (data.current_city_list == LIST_SELLS) {
        city_obj->city_sells_resource[resource] = amount;
    }

    return 1;
}

static int xml_start_trade_point(void)
{
    if (data.current_city_id == -1) {
        data.success = 0;
        log_error("No active city when parsing trade point", 0, 0);
        return 0;
    } else if (data.current_city_list != LIST_TRADE_WAYPOINTS) {
        data.success = 0;
        log_error("Trade point not trade_points tag", 0, 0);
        return 0;
    } else if (data.current_trade_route_id == -1) {
        data.success = 0;
        log_error("Attempting to parse trade point in a city that can't trade", 0, 0);
        return 0;
    }

    full_empire_object *obj = empire_object_get_full(data.next_empire_obj_id);
    obj->obj.id = data.next_empire_obj_id;
    data.next_empire_obj_id++;
    obj->in_use = 1;
    obj->obj.type = EMPIRE_OBJECT_TRADE_WAYPOINT;
    obj->obj.trade_route_id = data.current_trade_route_id;
    obj->obj.x = xml_parser_get_attribute_int("x");
    obj->obj.y = xml_parser_get_attribute_int("y");

    return 1;
}

static int xml_start_invasion_path(void)
{
    data.current_invasion_path_id++;
    return 1;
}

static int xml_start_battle(void)
{
    if (data.next_empire_obj_id >= MAX_EMPIRE_OBJECTS) {
        data.success = 0;
        log_error("Too many objects", 0, data.next_empire_obj_id);
        return 0;
    } else if (!data.current_invasion_path_id) {
        data.success = 0;
        log_error("Battle not in path tag", 0, 0);
        return 0;
    } else if (data.invasion_path_idx >= sizeof(data.invasion_path_ids) / sizeof(data.invasion_path_ids[0])) {
        data.success = 0;
        log_error("Invasion path too long", 0, 0);
        return 0;
    }

    full_empire_object *battle_obj = empire_object_get_full(data.next_empire_obj_id);
    battle_obj->obj.id = data.next_empire_obj_id;
    data.next_empire_obj_id++;
    battle_obj->in_use = 1;
    battle_obj->obj.type = EMPIRE_OBJECT_BATTLE_ICON;
    battle_obj->obj.invasion_path_id = data.current_invasion_path_id;
    battle_obj->obj.image_id = image_group(GROUP_EMPIRE_BATTLE);
    battle_obj->obj.x = xml_parser_get_attribute_int("x");
    battle_obj->obj.y = xml_parser_get_attribute_int("y");
    data.invasion_path_ids[data.invasion_path_idx] = battle_obj->obj.id;
    data.invasion_path_idx++;

    return 1;
}

static void xml_end_city(void)
{
    data.current_city_id = -1;
    data.current_trade_route_id = -1;
    data.current_city_list = LIST_NONE;
}

static void xml_end_sells_buys_or_waypoints(void)
{
    data.current_city_list = LIST_NONE;
}

static void xml_end_invasion_path(void)
{
    for (int i = 0; i < data.invasion_path_idx; i++) {
        int idx = data.invasion_path_idx - i - 1;
        full_empire_object *battle = empire_object_get_full(data.invasion_path_ids[idx]);
        battle->obj.invasion_years = i + 1;
    }
    memset(data.invasion_path_ids, 0, sizeof(data.invasion_path_ids));
    data.invasion_path_idx = 0;
}

static void reset_data(void)
{
    data.success = 1;
    data.next_empire_obj_id = 0;
    data.current_city_id = -1;
    data.current_trade_route_id = -1;
    data.current_city_list = LIST_NONE;
    data.current_invasion_path_id = 0;
    memset(data.invasion_path_ids, 0, sizeof(data.invasion_path_ids));
    data.invasion_path_idx = 0;
}

static void set_trade_coords(const empire_object *our_city)
{
    for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
        full_empire_object *trade_city = empire_object_get_full(i);
        if (
            !trade_city->in_use ||
            trade_city->obj.type != EMPIRE_OBJECT_CITY ||
            trade_city->city_type == EMPIRE_CITY_OURS ||
            !trade_city->obj.trade_route_id
            ) {
            continue;
        }
        empire_object *trade_route = empire_object_get(trade_city->obj.trade_route_id);
        if (!trade_route) {
            continue;
        }

        int num_waypoints = 0;
        for (int j = 0; j < MAX_EMPIRE_OBJECTS; j++) {
            empire_object *obj = empire_object_get(j);
            if (obj->type != EMPIRE_OBJECT_TRADE_WAYPOINT || obj->trade_route_id != trade_route->trade_route_id) {
                continue;
            }
            num_waypoints++;
        }

        int last_x = our_city->x + 25;
        int last_y = our_city->y + 25;
        int next_x = trade_city->obj.x + 25;
        int next_y = trade_city->obj.y + 25;
        int crossed_waypoints = 0;
        for (int j = 0; j < MAX_EMPIRE_OBJECTS && crossed_waypoints < num_waypoints / 2 + 1; j++) {
            empire_object *obj = empire_object_get(j);
            if (obj->type != EMPIRE_OBJECT_TRADE_WAYPOINT || obj->trade_route_id != trade_route->trade_route_id) {
                continue;
            }
            last_x = next_x;
            last_y = next_y;
            next_x = obj->x;
            next_y = obj->y;
            crossed_waypoints++;
        }

        trade_route->x = (next_x + last_x) / 2 - 16;
        trade_route->y = (next_y + last_y) / 2 - 10;
    }
}

static int parse_xml(char *buffer, int buffer_length)
{
    reset_data();
    empire_object_clear();
    if (!xml_parser_init(xml_elements, XML_TOTAL_ELEMENTS)) {
        return 0;
    }
    if (!xml_parser_parse(buffer, buffer_length, 1)) {
        data.success = 0;
    }
    xml_parser_free();
    if (!data.success) {
        return 0;
    }

    const empire_object *our_city = empire_object_get_our_city();
    if (!our_city) {
        log_error("No home city specified", 0, 0);
        return 0;
    }

    set_trade_coords(our_city);
    empire_object_init_cities(SCENARIO_CUSTOM_EMPIRE);

    return data.success;
}

static char *file_to_buffer(const char *filename, int *output_length)
{
    FILE *file = file_open(filename, "r");
    if (!file) {
        log_error("Error opening empire file", filename, 0);
        return 0;
    }
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);

    char *buffer = malloc(size);
    memset(buffer, 0, size);
    if (!buffer) {
        log_error("Unable to allocate buffer to read XML file", filename, 0);
        free(buffer);
        file_close(file);
        return 0;
    }
    *output_length = fread(buffer, 1, size, file);
    if (*output_length > size) {
        log_error("Unable to read file into buffer", filename, 0);
        free(buffer);
        file_close(file);
        *output_length = 0;
        return 0;
    }
    file_close(file);
    return buffer;
}

int empire_xml_parse_file(const char *filename)
{
    int output_length = 0;
    char *xml_contents = file_to_buffer(filename, &output_length);
    if (!xml_contents) {
        return 0;
    }
    int success = parse_xml(xml_contents, output_length);
    free(xml_contents);
    if (!success) {
        log_error("Error parsing file", filename, 0);
    }
    return success;
}
