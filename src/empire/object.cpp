#include "object.h"

#include "core/buffer.h"
#include "core/calc.h"
#include "core/io.h"
#include "empire/city.h"
#include "empire/trade_route.h"
#include "empire/type.h"
#include "graphics/image.h"
#include "game/resource.h"

#include "data/constants.hpp"
#include "data/scenario.hpp"
#include "empire.h"

#define HEADER_SIZE 1280
#define DATA_SIZE 12800

#define MAX_OBJECTS 200

static empire_object objects[MAX_OBJECTS];

static int get_trade_amount_code(int index, int resource);
static void set_trade_amount_code(int index, int resource, int amount_code);
static int is_sea_trade_route(int route_id);

int Empire_citySellsResource(int, int);
int Empire_cityBuysResource(int, int);

static void fix_image_ids()
{
    int image_id = 0;
    for (int i = 0; i < MAX_OBJECTS; i++)
    {
        if (objects[i].in_use
                && objects[i].type == EMPIRE_OBJECT_CITY
                && objects[i].city_type == EMPIRE_CITY_OURS)
        {
            image_id = objects[i].image_id;
            break;
        }
    }
    if (image_id > 0 && image_id != image_group(ID_Graphic_EmpireCity))
    {
        // empire map uses old version of graphics: increase every graphic id
        int offset = image_group(ID_Graphic_EmpireCity) - image_id;
        for (int i = 0; i < MAX_OBJECTS; i++)
        {
            if (!objects[i].in_use)
            {
                continue;
            }
            if (objects[i].image_id)
            {
                objects[i].image_id += offset;
                if (objects[i].expanded.image_id)
                {
                    objects[i].expanded.image_id += offset;
                }
            }
        }
    }
}

void empire_object_load(buffer *buf)
{
    for (int i = 0; i < MAX_OBJECTS; i++)
    {
        empire_object *obj = &objects[i];
        obj->id = i;
        obj->type = buffer_read_u8(buf);
        obj->in_use = buffer_read_u8(buf);
        obj->animation_index = buffer_read_u8(buf);
        buffer_skip(buf, 1);
        obj->x = buffer_read_i16(buf);
        obj->y = buffer_read_i16(buf);
        obj->width = buffer_read_i16(buf);
        obj->height = buffer_read_i16(buf);
        obj->image_id = buffer_read_i16(buf);
        obj->expanded.image_id = buffer_read_i16(buf);
        buffer_skip(buf, 1);
        obj->distant_battle_travel_months = buffer_read_u8(buf);
        buffer_skip(buf, 2);
        obj->expanded.x = buffer_read_i16(buf);
        obj->expanded.y = buffer_read_i16(buf);
        obj->city_type = buffer_read_u8(buf);
        obj->city_name_id = buffer_read_u8(buf);
        obj->trade_route_id = buffer_read_u8(buf);
        obj->trade_route_open = buffer_read_u8(buf);
        obj->trade_route_cost = buffer_read_i16(buf);
        for (int r = 0; r < 10; r++)
        {
            obj->city_sells_resource[r] = buffer_read_u8(buf);
        }
        obj->owner_city_object = buffer_read_u8(buf);
        buffer_skip(buf, 1);
        for (int r = 0; r < 8; r++)
        {
            obj->city_buys_resource[r] = buffer_read_u8(buf);
        }
        obj->invasion_path_id = buffer_read_u8(buf);
        obj->invasion_years = buffer_read_u8(buf);
        obj->trade40 = buffer_read_u16(buf);
        obj->trade25 = buffer_read_u16(buf);
        obj->trade15 = buffer_read_u16(buf);
        buffer_skip(buf, 6);
    }

    fix_image_ids();
}

void empire_object_init_trade_amounts()
{
    for (int i = 0; i < MAX_OBJECTS; i++)
    {
        if (!objects[i].in_use || objects[i].type != EMPIRE_OBJECT_CITY)
        {
            continue;
        }
        int total_amount = 0;
        for (int res = RESOURCE_MIN; res < RESOURCE_MAX; res++)
        {
            total_amount += get_trade_amount_code(i, res);
        }
        if (total_amount)
        {
            for (int res = RESOURCE_MIN; res < RESOURCE_MAX; res++)
            {
                if (!Empire_citySellsResource(i, res) && !Empire_cityBuysResource(i, res))
                {
                    set_trade_amount_code(i, res, 0);
                }
            }
        }
        else
        {
            // reset everything to 25
            for (int res = RESOURCE_MIN; res < RESOURCE_MAX; res++)
            {
                set_trade_amount_code(i, res, 2);
            }
        }
    }
}

void empire_object_init_cities()
{
    empire_city_clear_all();
    int routeIndex = 1;
    for (int i = 0; i < MAX_OBJECTS; i++)
    {
        if (!objects[i].in_use || objects[i].type != EMPIRE_OBJECT_CITY)
        {
            continue;
        }
        empire_object *obj = &objects[i];
        empire_city *city = empire_city_get(routeIndex++);
        city->in_use = 1;
        city->type = obj->city_type;
        city->name_id = obj->city_name_id;
        if (obj->trade_route_id < 0)
        {
            obj->trade_route_id = 0;
        }
        if (obj->trade_route_id >= 20)
        {
            obj->trade_route_id = 19;
        }
        city->route_id = obj->trade_route_id;
        city->is_open = obj->trade_route_open;
        city->cost_to_open = obj->trade_route_cost;
        city->is_sea_trade = is_sea_trade_route(obj->trade_route_id);

        for (int resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++)
        {
            city->sells_resource[resource] = 0;
            city->buys_resource[resource] = 0;
            if (city->type == EMPIRE_CITY_DISTANT_ROMAN
                    || city->type == EMPIRE_CITY_DISTANT_FOREIGN
                    || city->type== EMPIRE_CITY_VULNERABLE_ROMAN
                    || city->type== EMPIRE_CITY_FUTURE_ROMAN)
            {
                continue;
            }
            if (Empire_citySellsResource(i, resource))
            {
                city->sells_resource[resource] = 1;
            }
            if (Empire_cityBuysResource(i, resource))
            {
                city->buys_resource[resource] = 1;
            }
            int amountCode = get_trade_amount_code(i, resource);
            int routeId = city->route_id;
            int amount;
            switch (amountCode)
            {
            case 1:
                amount = 15;
                break;
            case 2:
                amount = 25;
                break;
            case 3:
                amount = 40;
                break;
            default:
                amount = 0;
                break;
            }
            trade_route_init(routeId, resource, amount);
        }
        city->trader_entry_delay = 4;
        city->trader_figure_ids[0] = 0;
        city->trader_figure_ids[1] = 0;
        city->trader_figure_ids[2] = 0;
        city->empire_object_id = i;
    }
}


int empire_object_init_distant_battle_travel_months(int object_type)
{
    int month = 0;
    for (int i = 0; i < MAX_OBJECTS; i++)
    {
        if (objects[i].in_use && objects[i].type == object_type)
        {
            month++;
            objects[i].distant_battle_travel_months = month;
        }
    }
    return month;
}

const empire_object *empire_object_get(int object_id)
{
    return &objects[object_id];
}

void empire_object_foreach(void (*callback)(const empire_object *))
{
    for (int i = 0; i < MAX_OBJECTS; i++)
    {
        if (objects[i].in_use)
        {
            callback(&objects[i]);
        }
    }
}

const empire_object *empire_object_get_battle_icon(int path_id, int year)
{
    for (int i = 0; i < MAX_OBJECTS; i++)
    {
        empire_object *obj = &objects[i];
        if (obj->in_use && obj->type == EMPIRE_OBJECT_BATTLE_ICON &&
                obj->invasion_path_id == path_id && obj->invasion_years == year)
        {
            return obj;
        }
    }
    return 0;
}

int empire_object_get_max_invasion_path()
{
    int max_path = 0;
    for (int i = 0; i < MAX_OBJECTS; i++)
    {
        if (objects[i].in_use && objects[i].type == EMPIRE_OBJECT_BATTLE_ICON)
        {
            if (objects[i].invasion_path_id > max_path)
            {
                max_path = objects[i].invasion_path_id;
            }
        }
    }
    return max_path;
}


int empire_object_get_closest(int x, int y)
{
    int min_dist = 10000;
    int min_obj_id = 0;
    for (int i = 0; i < MAX_OBJECTS && objects[i].in_use; i++)
    {
        const empire_object *obj = &objects[i];
        int obj_x, obj_y;
        if (Data_Scenario.empireHasExpanded)
        {
            obj_x = obj->expanded.x;
            obj_y = obj->expanded.y;
        }
        else
        {
            obj_x = obj->x;
            obj_y = obj->y;
        }
        if (obj_x - 8 > x || obj_x + obj->width + 8 <= x)
        {
            continue;
        }
        if (obj_y - 8 > y || obj_y + obj->height + 8 <= y)
        {
            continue;
        }
        int dist = calc_maximum_distance(x, y, obj_x + obj->width / 2, obj_y + obj->height / 2);
        if (dist < min_dist)
        {
            min_dist = dist;
            min_obj_id = i + 1;
        }
    }
    return min_obj_id;
}

void empire_object_set_expanded(int object_id, int new_city_type)
{
    objects[object_id].city_type = new_city_type;
    if (new_city_type == EMPIRE_CITY_TRADE)
    {
        objects[object_id].expanded.image_id = image_group(ID_Graphic_EmpireCityTrade);
    }
    else if (new_city_type == EMPIRE_CITY_DISTANT_ROMAN)
    {
        objects[object_id].expanded.image_id = image_group(ID_Graphic_EmpireCityDistantRoman);
    }
}

static int is_trade_city(int index)
{
    if (objects[index].type != EMPIRE_OBJECT_CITY)
    {
        return 0;
    }
    return objects[index].city_type > EMPIRE_CITY_OURS && objects[index].city_type < EMPIRE_CITY_FUTURE_ROMAN;
}

static int get_trade_amount_code(int index, int resource)
{
    if (!is_trade_city(index))
    {
        return 0;
    }
    int resourceFlag = 1 << resource;
    if (objects[index].trade40 & resourceFlag)
    {
        return 3;
    }
    if (objects[index].trade25 & resourceFlag)
    {
        return 2;
    }
    if (objects[index].trade15 & resourceFlag)
    {
        return 1;
    }
    return 0;
}

static void set_trade_amount_code(int index, int resource, int amount_code)
{
    if (!is_trade_city(index))
    {
        return;
    }
    int resource_flag = 1 << resource;
    // clear flags
    objects[index].trade40 &= ~resource_flag;
    objects[index].trade25 &= ~resource_flag;
    objects[index].trade15 &= ~resource_flag;
    // set correct flag
    if (amount_code == 1)
    {
        objects[index].trade15 |= resource_flag;
    }
    else if (amount_code == 2)
    {
        objects[index].trade25 |= resource_flag;
    }
    else if (amount_code == 3)
    {
        objects[index].trade40 |= resource_flag;
    }
}

static int is_sea_trade_route(int route_id)
{
    for (int i = 0; i < MAX_OBJECTS; i++)
    {
        if (objects[i].in_use && objects[i].trade_route_id == route_id)
        {
            if (objects[i].type == EMPIRE_OBJECT_SEA_TRADE_ROUTE)
            {
                return 1;
            }
            if (objects[i].type == EMPIRE_OBJECT_LAND_TRADE_ROUTE)
            {
                return 0;
            }
        }
    }
    return 0;
}

void empire_object_update_animation(int object_id, int new_animation_index)
{
    objects[object_id].animation_index = new_animation_index;
}
