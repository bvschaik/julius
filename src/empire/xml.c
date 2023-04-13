#include "xml.h"

#include "assets/assets.h"
#include "core/array.h"
#include "core/buffer.h"
#include "core/calc.h"
#include "core/file.h"
#include "core/image_group.h"
#include "core/log.h"
#include "core/string.h"
#include "core/xml_parser.h"
#include "core/zlib_helper.h"
#include "empire/city.h"
#include "empire/object.h"
#include "empire/trade_route.h"
#include "scenario/data.h"
#include "scenario/empire.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XML_TOTAL_ELEMENTS 17
#define BASE_BORDER_FLAG_IMAGE_ID 3323
#define BASE_ORNAMENT_IMAGE_ID 3356
#define BORDER_EDGE_DEFAULT_SPACING 50
#define ORIGINAL_ORNAMENTS 20

typedef enum {
    LIST_NONE = -1,
    LIST_BUYS = 1,
    LIST_SELLS = 2,
    LIST_TRADE_WAYPOINTS = 3
} city_list;

typedef enum {
    DISTANT_BATTLE_PATH_NONE,
    DISTANT_BATTLE_PATH_ROMAN,
    DISTANT_BATTLE_PATH_ENEMY
} distant_battle_path_type;

enum {
    BORDER_STATUS_NONE = 0,
    BORDER_STATUS_CREATING,
    BORDER_STATUS_DONE
};

typedef struct {
    int x;
    int y;
    int num_months;
} waypoint;

static const char *ORNAMENTS[] = {
    "The Stonehenge",
    "Gallic Wheat",
    "The Pyrenees",
    "Iberian Aqueduct",
    "Triumphal Arch",
    "West Desert Wheat",
    "Lighthouse of Alexandria",
    "West Desert Palm Trees",
    "Trade Ship",
    "Waterside Palm Trees",
    "Colosseum|The Colosseum",
    "The Alps",
    "Roman Tree",
    "Greek Mountain Range",
    "The Parthenon",
    "The Pyramids",
    "The Hagia Sophia",
    "East Desert Palm Trees",
    "East Desert Wheat",
    "Trade Camel",
    "Mount Etna",
    "Colossus of Rhodes"
};

#define TOTAL_ORNAMENTS (sizeof(ORNAMENTS) / sizeof(const char *))

map_point ORNAMENT_POSITIONS[TOTAL_ORNAMENTS] = {
    {  247,  81 }, {  361, 356 }, {  254, 428 }, {  199, 590 }, {  275, 791 },
    {  423, 802 }, { 1465, 883 }, {  518, 764 }, {  691, 618 }, {  742, 894 },
    {  726, 468 }, {  502, 280 }, {  855, 551 }, { 1014, 443 }, { 1158, 698 },
    { 1431, 961 }, { 1300, 500 }, { 1347, 648 }, { 1707, 783 }, { 1704, 876 },
    {  829, 720 }, { 1347, 745 }
};

static struct {
    int success;
    int version;
    int current_city_id;
    city_list current_city_list;
    int has_vulnerable_city;
    int current_invasion_path_id;
    array(int) invasion_path_ids;
    distant_battle_path_type distant_battle_path_type;
    array(waypoint) distant_battle_waypoints;
    int border_status;
    char added_ornaments[TOTAL_ORNAMENTS];
} data;

static int xml_start_empire(void);
static int xml_start_ornament(void);
static int xml_start_border(void);
static int xml_start_border_edge(void);
static int xml_start_city(void);
static int xml_start_buys(void);
static int xml_start_sells(void);
static int xml_start_waypoints(void);
static int xml_start_resource(void);
static int xml_start_trade_point(void);
static int xml_start_invasion_path(void);
static int xml_start_battle(void);
static int xml_start_distant_battle_path(void);
static int xml_start_distant_battle_waypoint(void);

static void xml_end_border(void);
static void xml_end_city(void);
static void xml_end_sells_buys_or_waypoints(void);
static void xml_end_invasion_path(void);
static void xml_end_distant_battle_path(void);

static const xml_parser_element xml_elements[XML_TOTAL_ELEMENTS] = {
    { "empire", xml_start_empire },
    { "ornament", xml_start_ornament, 0, "empire" },
    { "border", xml_start_border, xml_end_border, "empire" },
    { "edge", xml_start_border_edge, 0, "border" },
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
    { "distant_battle_paths", 0, 0, "empire" },
    { "path", xml_start_distant_battle_path, xml_end_distant_battle_path, "distant_battle_paths" },
    { "waypoint", xml_start_distant_battle_waypoint, 0, "path" },
};

static resource_type get_resource_from_attr(const char *key)
{
    const char *value = xml_parser_get_attribute_string(key);
    if (!value) {
        return RESOURCE_NONE;
    }
    for (resource_type i = RESOURCE_MIN; i < RESOURCE_MAX; i++) {
        const char *resource_name = resource_get_data(i)->xml_attr_name;
        if (xml_parser_compare_multiple(resource_name, value)) {
            return i;
        }
    }
    return RESOURCE_NONE;
}

static int xml_start_empire(void)
{
    data.version = xml_parser_get_attribute_int("version");
    if (!data.version) {
        data.success = 0;
        log_error("No version set", 0, 0);
        return 0;
    }
    if (xml_parser_get_attribute_bool("show_ireland")) {
        full_empire_object *obj = empire_object_get_new();
        obj->in_use = 1;
        obj->obj.type = EMPIRE_OBJECT_ORNAMENT;
        obj->obj.image_id = -1;
    }
    return 1;
}

static void add_ornament(int ornament_id)
{
    if (data.added_ornaments[ornament_id]) {
        return;
    }
    data.added_ornaments[ornament_id] = 1;
    full_empire_object *obj = empire_object_get_new();
    if (!obj) {
        data.success = 0;
        log_error("Error creating new object - out of memory", 0, 0);
        return;
    }
    obj->in_use = 1;
    obj->obj.type = EMPIRE_OBJECT_ORNAMENT;
    if (ornament_id < ORIGINAL_ORNAMENTS) {
        obj->obj.image_id = BASE_ORNAMENT_IMAGE_ID + ornament_id;
    } else {
        obj->obj.image_id = ORIGINAL_ORNAMENTS - ornament_id - 2;
    }
    obj->obj.x = ORNAMENT_POSITIONS[ornament_id].x;
    obj->obj.y = ORNAMENT_POSITIONS[ornament_id].y;
}

static int xml_start_ornament(void)
{
    if (!xml_parser_has_attribute("type")) {
        log_info("No ornament type specified", 0, 0);
        return 1;
    }
    int ornament_id = xml_parser_get_attribute_enum("type", ORNAMENTS, TOTAL_ORNAMENTS, 0);
    if (ornament_id == -1) {
        if (strcmp("all", xml_parser_get_attribute_string("type")) == 0) {
            for (int i = 0; i < TOTAL_ORNAMENTS; i++) {
                add_ornament(i);
            }
        } else {
            log_info("Invalid ornament type specified", 0, 0);
        }
    } else {
        add_ornament(ornament_id);
    }
    return 1;
}

static int xml_start_border(void)
{
    if (data.border_status != BORDER_STATUS_NONE) {
        data.success = 0;
        log_error("Border is being set twice", 0, 0);
        return 0;
    }
    full_empire_object *obj = empire_object_get_new();
    if (!obj) {
        data.success = 0;
        log_error("Error creating new object - out of memory", 0, 0);
        return 0;
    }
    obj->in_use = 1;
    obj->obj.type = EMPIRE_OBJECT_BORDER;
    obj->obj.width = xml_parser_get_attribute_int("density");
    if (obj->obj.width == 0) {
        obj->obj.width = BORDER_EDGE_DEFAULT_SPACING;
    }
    data.border_status = BORDER_STATUS_CREATING;
    return 1;
}

static int xml_start_border_edge(void)
{
    if (data.border_status != BORDER_STATUS_CREATING) {
        data.success = 0;
        log_error("Border edge is being wrongly added", 0, 0);
        return 0;
    }
    full_empire_object *obj = empire_object_get_new();
    if (!obj) {
        data.success = 0;
        log_error("Error creating new object - out of memory", 0, 0);
        return 0;
    }
    obj->in_use = 1;
    obj->obj.type = EMPIRE_OBJECT_BORDER_EDGE;
    obj->obj.x = xml_parser_get_attribute_int("x");
    obj->obj.y = xml_parser_get_attribute_int("y");
    obj->obj.image_id = xml_parser_get_attribute_bool("hidden") ? 0 : BASE_BORDER_FLAG_IMAGE_ID;

    return 1;
}

static int xml_start_city(void)
{
    full_empire_object *city_obj = empire_object_get_new();

    if (!city_obj) {
        data.success = 0;
        log_error("Error creating new object - out of memory", 0, 0);
        return 0;
    }

    if (xml_parser_get_total_attributes() < 2) {
        log_error("Wrong number of attributes for the city", 0, 0);
        return 0;
    }

    data.current_city_id = city_obj->obj.id;
    city_obj->in_use = 1;
    city_obj->obj.type = EMPIRE_OBJECT_CITY;
    city_obj->city_type = EMPIRE_CITY_TRADE;
    city_obj->obj.image_id = image_group(GROUP_EMPIRE_CITY_TRADE);
    city_obj->trade_route_cost = 500;

    static const char *city_types[6] = { "roman", "ours", "trade", "future_trade", "distant", "vulnerable" };
    static const char *trade_route_types[2] = { "land", "sea" };

    const char *name = xml_parser_get_attribute_string("name");
    if (name) {
        string_copy(string_from_ascii(name), city_obj->city_custom_name, sizeof(city_obj->city_custom_name));
    }

    int city_type = xml_parser_get_attribute_enum("type", city_types, 6, EMPIRE_CITY_DISTANT_ROMAN);
    if (city_type < EMPIRE_CITY_DISTANT_ROMAN) {
        city_obj->city_type = EMPIRE_CITY_TRADE;
    } else {
        city_obj->city_type = city_type;
    }
    switch (city_obj->city_type) {
        case EMPIRE_CITY_OURS:
            city_obj->obj.image_id = image_group(GROUP_EMPIRE_CITY);
            break;
        case EMPIRE_CITY_FUTURE_TRADE:
        case EMPIRE_CITY_DISTANT_ROMAN:
            city_obj->obj.image_id = image_group(GROUP_EMPIRE_CITY_DISTANT_ROMAN);
            break;
        case EMPIRE_CITY_DISTANT_FOREIGN:
            city_obj->obj.image_id = image_group(GROUP_EMPIRE_FOREIGN_CITY);
            break;
        case EMPIRE_CITY_VULNERABLE_ROMAN:
            city_obj->obj.image_id = image_group(GROUP_EMPIRE_CITY_DISTANT_ROMAN);
            data.has_vulnerable_city = 1;
            break;
        default:
            city_obj->obj.image_id = image_group(GROUP_EMPIRE_CITY_TRADE);
            break;
    }

    const image *img = image_get(city_obj->obj.image_id);

    city_obj->obj.width = img->width;
    city_obj->obj.height = img->height;

    city_obj->obj.x = xml_parser_get_attribute_int("x") - city_obj->obj.width / 2;
    city_obj->obj.y = xml_parser_get_attribute_int("y") - city_obj->obj.height / 2;

    if (city_obj->city_type == EMPIRE_CITY_TRADE || city_obj->city_type == EMPIRE_CITY_FUTURE_TRADE) {
        full_empire_object *route_obj = empire_object_get_new();
        if (!route_obj) {
            data.success = 0;
            log_error("Error creating new object - out of memory", 0, 0);
            return 0;
        }
        route_obj->in_use = 1;
        route_obj->obj.type = EMPIRE_OBJECT_LAND_TRADE_ROUTE;

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
    
    full_empire_object *city_obj = empire_object_get_full(data.current_city_id);

    if (!xml_parser_has_attribute("type")) {
        data.success = 0;
        log_error("Unable to find resource type attribute", 0, 0);
        return 0;
    }
    resource_type resource = get_resource_from_attr("type");
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
    } else if (!empire_object_get_full(data.current_city_id)->trade_route_cost) {
        data.success = 0;
        log_error("Attempting to parse trade point in a city that can't trade", 0, 0);
        return 0;
    }

    full_empire_object *obj = empire_object_get_new();
    if (!obj) {
        data.success = 0;
        log_error("Error creating new object - out of memory", 0, 0);
        return 0;
    }
    obj->in_use = 1;
    obj->obj.type = EMPIRE_OBJECT_TRADE_WAYPOINT;
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
    if (!data.current_invasion_path_id) {
        data.success = 0;
        log_error("Battle not in path tag", 0, 0);
        return 0;
    }
    int *battle_id = array_advance(data.invasion_path_ids);
    if (!battle_id) {
        data.success = 0;
        log_error("Error creating invasion path - out of memory", 0, 0);
        return 0;
    }

    full_empire_object *battle_obj = empire_object_get_new();
    if (!battle_obj) {
        data.invasion_path_ids.size--;
        data.success = 0;
        log_error("Error creating new object - out of memory", 0, 0);
        return 0;
    }
    *battle_id = battle_obj->obj.id;
    battle_obj->in_use = 1;
    battle_obj->obj.type = EMPIRE_OBJECT_BATTLE_ICON;
    battle_obj->obj.invasion_path_id = data.current_invasion_path_id;
    battle_obj->obj.image_id = image_group(GROUP_EMPIRE_BATTLE);
    battle_obj->obj.x = xml_parser_get_attribute_int("x");
    battle_obj->obj.y = xml_parser_get_attribute_int("y");

    return 1;
}

static int xml_start_distant_battle_path(void)
{
    if (!data.has_vulnerable_city) {
        data.success = 0;
        log_error("Must have a vulnerable city to set up distant battle paths", 0, 0);
        return 0;
    } else if (!xml_parser_has_attribute("type")) {
        data.success = 0;
        log_error("Unable to find type attribute on distant battle path", 0, 0);
        return 0;
    } else if (!xml_parser_has_attribute("start_x")) {
        data.success = 0;
        log_error("Unable to find start_x attribute on distant battle path", 0, 0);
        return 0;
    } else if (!xml_parser_has_attribute("start_y")) {
        data.success = 0;
        log_error("Unable to find start_y attribute on distant battle path", 0, 0);
        return 0;
    }

    const char *type = xml_parser_get_attribute_string("type");
    if (strcmp(type, "roman") == 0) {
        data.distant_battle_path_type = DISTANT_BATTLE_PATH_ROMAN;
    } else if (strcmp(type, "enemy") == 0) {
        data.distant_battle_path_type = DISTANT_BATTLE_PATH_ENEMY;
    } else {
        data.success = 0;
        log_error("Distant battle path type must be \"roman\" or \"enemy\"", type, 0);
        return 0;
    }

    waypoint *w = array_advance(data.distant_battle_waypoints);
    if (!w) {
        data.success = 0;
        log_error("Error creating new object - out of memory", 0, 0);
        return 0;
    }
    w->x = xml_parser_get_attribute_int("start_x");
    w->y = xml_parser_get_attribute_int("start_y");
    w->num_months = 0;
    return 1;
}

static int xml_start_distant_battle_waypoint(void)
{
    if (!xml_parser_has_attribute("num_months")) {
        data.success = 0;
        log_error("Unable to find num_months attribute on distant battle path", 0, 0);
        return 0;
    } else if (!xml_parser_has_attribute("x")) {
        data.success = 0;
        log_error("Unable to find x attribute on distant battle path", 0, 0);
        return 0;
    } else if (!xml_parser_has_attribute("y")) {
        data.success = 0;
        log_error("Unable to find y attribute on distant battle path", 0, 0);
        return 0;
    }

    waypoint *w = array_advance(data.distant_battle_waypoints);
    if (!w) {
        data.success = 0;
        log_error("Error creating new object - out of memory", 0, 0);
        return 0;
    }    w->x = xml_parser_get_attribute_int("x");
    w->y = xml_parser_get_attribute_int("y");
    w->num_months = xml_parser_get_attribute_int("num_months");
    return 1;
}

static void xml_end_border(void)
{
    if (data.border_status == BORDER_STATUS_CREATING) {
        data.border_status = BORDER_STATUS_DONE;
    }
}

static void xml_end_city(void)
{
    data.current_city_id = -1;
    data.current_city_list = LIST_NONE;
}

static void xml_end_sells_buys_or_waypoints(void)
{
    data.current_city_list = LIST_NONE;
}

static void xml_end_invasion_path(void)
{
    for (int i = data.invasion_path_ids.size - 1; i >= 0; i--) {
        full_empire_object *battle = empire_object_get_full(*array_item(data.invasion_path_ids, i));
        battle->obj.invasion_years = i + 1;
    }
    data.invasion_path_ids.size = 0;
}

static void xml_end_distant_battle_path(void)
{
    empire_object_type obj_type = 0;
    int image_id = 0;
    if (data.distant_battle_path_type == DISTANT_BATTLE_PATH_ROMAN) {
        obj_type = EMPIRE_OBJECT_ROMAN_ARMY;
        image_id = GROUP_EMPIRE_ROMAN_ARMY;
    } else if (data.distant_battle_path_type == DISTANT_BATTLE_PATH_ENEMY) {
        obj_type = EMPIRE_OBJECT_ENEMY_ARMY;
        image_id = GROUP_EMPIRE_ENEMY_ARMY;
    } else {
        data.success = 0;
        log_error("Invalid distant battle path type", 0, data.distant_battle_path_type);
        return;
    }

    int month = 1;
    for (int i = 1; i < data.distant_battle_waypoints.size; i++) {
        waypoint *last = array_item(data.distant_battle_waypoints, i - 1);
        waypoint *current = array_item(data.distant_battle_waypoints, i);
        int x_diff = current->x - last->x;
        int y_diff = current->y - last->y;
        for (int j = 0; j < current->num_months; j++) {
            full_empire_object *army_obj = empire_object_get_new();
            if (!army_obj) {
                data.success = 0;
                log_error("Error creating new object - out of memory", 0, 0);
                return;
            }
            army_obj->in_use = 1;
            army_obj->obj.type = obj_type;
            army_obj->obj.image_id = image_group(image_id);
            army_obj->obj.x = (double)j / current->num_months * x_diff + last->x;
            army_obj->obj.y = (double)j / current->num_months * y_diff + last->y;
            army_obj->obj.distant_battle_travel_months = month;
            month++;
        }
    }
    data.distant_battle_path_type = DISTANT_BATTLE_PATH_NONE;
    data.distant_battle_waypoints.size = 0;
}

static void reset_data(void)
{
    data.success = 1;
    data.current_city_id = -1;
    data.current_city_list = LIST_NONE;
    data.has_vulnerable_city = 0;
    data.current_invasion_path_id = 0;
    array_init(data.invasion_path_ids, 10, 0, 0);
    data.distant_battle_path_type = DISTANT_BATTLE_PATH_NONE;
    array_init(data.distant_battle_waypoints, 50, 0, 0);
    data.border_status = BORDER_STATUS_NONE;
    memset(data.added_ornaments, 0, sizeof(data.added_ornaments));
}

static void set_trade_coords(const empire_object *our_city)
{
    int *section_distances = 0;
    for (int i = 0; i < empire_object_count(); i++) {
        full_empire_object *trade_city = empire_object_get_full(i);
        if (
            !trade_city->in_use ||
            trade_city->obj.type != EMPIRE_OBJECT_CITY ||
            trade_city->city_type == EMPIRE_CITY_OURS ||
            (trade_city->city_type != EMPIRE_CITY_TRADE && trade_city->city_type != EMPIRE_CITY_FUTURE_TRADE)
            ) {
            continue;
        }
        empire_object *trade_route = empire_object_get(i + 1);

        if (!section_distances) {
            section_distances = malloc(sizeof(int) * (empire_object_count() - 1));
        }
        int sections = 0;
        int distance = 0;
        int last_x = our_city->x + 25;
        int last_y = our_city->y + 25;
        int x_diff, y_diff;
        for (int j = i + 2; j < empire_object_count(); j++) {
            empire_object *obj = empire_object_get(j);
            if (obj->type != EMPIRE_OBJECT_TRADE_WAYPOINT) {
                break;
            }
            x_diff = obj->x - last_x;
            y_diff = obj->y - last_y;
            section_distances[sections] = (int) sqrt(x_diff * x_diff + y_diff * y_diff);
            distance += section_distances[sections];
            last_x = obj->x;
            last_y = obj->y;
            sections++;
        }
        x_diff = trade_city->obj.x + 25 - last_x;
        y_diff = trade_city->obj.y + 25 - last_y;
        section_distances[sections] = (int) sqrt(x_diff * x_diff + y_diff * y_diff);
        distance += section_distances[sections];
        sections++;
    
        last_x = our_city->x + 25;
        last_y = our_city->y + 25;
        int next_x = trade_city->obj.x + 25;
        int next_y = trade_city->obj.y + 25;

        if (sections == 1) {
            trade_route->x = (next_x + last_x) / 2 - 16;
            trade_route->y = (next_y + last_y) / 2 - 10;
            continue;
        }
        int crossed_distance = 0;
        int current_section = 0;
        int remaining_distance = 0;
        while (current_section < sections) {
            if (current_section == sections - 1) {
                next_x = trade_city->obj.x + 25;
                next_y = trade_city->obj.y + 25;
            } else {
                empire_object *obj = empire_object_get(current_section + i + 2);
                next_x = obj->x;
                next_y = obj->y;
            }
            if (section_distances[current_section] + crossed_distance > distance / 2) {
                remaining_distance = distance / 2 - crossed_distance;
                break;
            }
            last_x = next_x;
            last_y = next_y;
            crossed_distance += section_distances[current_section];
            current_section++;
        }
        x_diff = next_x - last_x;
        y_diff = next_y - last_y;
        int x_factor = calc_percentage(x_diff, section_distances[current_section]);
        int y_factor = calc_percentage(y_diff, section_distances[current_section]);
        trade_route->x = calc_adjust_with_percentage(remaining_distance, x_factor) + last_x - 16;
        trade_route->y = calc_adjust_with_percentage(remaining_distance, y_factor) + last_y - 10;

        i += sections; // We know the following objects are waypoints so we skip them
    }
    free(section_distances);
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
    if (!buffer) {
        log_error("Error opening empire file", filename, 0);
        return 0;
    }
    memset(buffer, 0, size);
    if (!buffer) {
        log_error("Unable to allocate buffer to read XML file", filename, 0);
        free(buffer);
        file_close(file);
        return 0;
    }
    *output_length = (int) fread(buffer, 1, size, file);
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
