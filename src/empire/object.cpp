#include "object.h"

#include "core/buffer.h"
#include "core/calc.h"
#include "core/io.h"
#include "empire/city.h"
#include "empire/trade_route.h"
#include "empire/type.h"
#include "graphics/image.h"

#include <game>
#include <data>
#include <scenario>

#define HEADER_SIZE 1280
#define DATA_SIZE 12800

#define MAX_OBJECTS 200

typedef struct
{
    int in_use;
    int city_type;
    int city_name_id;
    int trade_route_open;
    int trade_route_cost;
    int city_sells_resource[10];
    int city_buys_resource[8];
    int trade40;
    int trade25;
    int trade15;
    empire_object obj;
} full_empire_object;

static full_empire_object objects[MAX_OBJECTS];

static int get_trade_amount_code(int index, int resource);
static int is_sea_trade_route(int route_id);


static void fix_image_ids()
{
    int image_id = 0;
    for (int i = 0; i < MAX_OBJECTS; i++)
    {
        if (objects[i].in_use
                && objects[i].obj.type == EMPIRE_OBJECT_CITY
                && objects[i].city_type == EMPIRE_CITY_OURS)
        {
            image_id = objects[i].obj.image_id;
            break;
        }
    }
    if (image_id > 0 && image_id != image_group(GROUP_EMPIRE_CITY))
    {
        // empire map uses old version of graphics: increase every graphic id
        int offset = image_group(GROUP_EMPIRE_CITY) - image_id;
        for (int i = 0; i < MAX_OBJECTS; i++)
        {
            if (!objects[i].in_use)
            {
                continue;
            }
            if (objects[i].obj.image_id)
            {
                objects[i].obj.image_id += offset;
                if (objects[i].obj.expanded.image_id)
                {
                    objects[i].obj.expanded.image_id += offset;
                }
            }
        }
    }
}

void empire_object_load(buffer *buf)
{
    for (int i = 0; i < MAX_OBJECTS; i++)
    {
        full_empire_object *full = &objects[i];
        empire_object *obj = &full->obj;
        obj->id = i;
        obj->type = buffer_read_u8(buf);
        full->in_use = buffer_read_u8(buf);
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
        full->city_type = buffer_read_u8(buf);
        full->city_name_id = buffer_read_u8(buf);
        obj->trade_route_id = buffer_read_u8(buf);
        full->trade_route_open = buffer_read_u8(buf);
        full->trade_route_cost = buffer_read_i16(buf);
        for (int r = 0; r < 10; r++)
        {
            full->city_sells_resource[r] = buffer_read_u8(buf);
        }
        buffer_skip(buf, 2);
        for (int r = 0; r < 8; r++)
        {
            full->city_buys_resource[r] = buffer_read_u8(buf);
        }
        obj->invasion_path_id = buffer_read_u8(buf);
        obj->invasion_years = buffer_read_u8(buf);
        full->trade40 = buffer_read_u16(buf);
        full->trade25 = buffer_read_u16(buf);
        full->trade15 = buffer_read_u16(buf);
        buffer_skip(buf, 6);
    }

    fix_image_ids();
}

void empire_object_init_cities()
{
    empire_city_clear_all();
    int routeIndex = 1;
    for (int i = 0; i < MAX_OBJECTS; i++)
    {
        if (!objects[i].in_use || objects[i].obj.type != EMPIRE_OBJECT_CITY)
        {
            continue;
        }
        full_empire_object *obj = &objects[i];
        empire_city *city = empire_city_get(routeIndex++);
        city->in_use = 1;
        city->type = obj->city_type;
        city->name_id = obj->city_name_id;
        if (obj->obj.trade_route_id < 0)
        {
            obj->obj.trade_route_id = 0;
        }
        if (obj->obj.trade_route_id >= 20)
        {
            obj->obj.trade_route_id = 19;
        }
        city->route_id = obj->obj.trade_route_id;
        city->is_open = obj->trade_route_open;
        city->cost_to_open = obj->trade_route_cost;
        city->is_sea_trade = is_sea_trade_route(obj->obj.trade_route_id);

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
            if (empire_object_city_sells_resource(i, resource))
            {
                city->sells_resource[resource] = 1;
            }
            if (empire_object_city_buys_resource(i, resource))
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
        if (objects[i].in_use && objects[i].obj.type == object_type)
        {
            month++;
            objects[i].obj.distant_battle_travel_months = month;
        }
    }
    return month;
}

const empire_object *empire_object_get(int object_id)
{
    return &objects[object_id].obj;
}

void empire_object_foreach(void (*callback)(const empire_object *))
{
    for (int i = 0; i < MAX_OBJECTS; i++)
    {
        if (objects[i].in_use)
        {
            callback(&objects[i].obj);
        }
    }
}

const empire_object *empire_object_get_battle_icon(int path_id, int year)
{
    for (int i = 0; i < MAX_OBJECTS; i++)
    {
        if (objects[i].in_use)
        {
            empire_object *obj = &objects[i].obj;
            if (obj->type == EMPIRE_OBJECT_BATTLE_ICON &&
                    obj->invasion_path_id == path_id && obj->invasion_years == year)
            {
                return obj;
            }
        }
    }
    return 0;
}

int empire_object_get_max_invasion_path()
{
    int max_path = 0;
    for (int i = 0; i < MAX_OBJECTS; i++)
    {
        if (objects[i].in_use && objects[i].obj.type == EMPIRE_OBJECT_BATTLE_ICON)
        {
            if (objects[i].obj.invasion_path_id > max_path)
            {
                max_path = objects[i].obj.invasion_path_id;
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
        const empire_object *obj = &objects[i].obj;
        int obj_x, obj_y;
        if (scenario_empire_is_expanded)
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
        objects[object_id].obj.expanded.image_id = image_group(GROUP_EMPIRE_CITYTrade);
    }
    else if (new_city_type == EMPIRE_CITY_DISTANT_ROMAN)
    {
        objects[object_id].obj.expanded.image_id = image_group(GROUP_EMPIRE_CITYDistantRoman);
    }
}

int empire_object_city_buys_resource(int object_id, int resource)
{
    const full_empire_object *object = &objects[object_id];
    for (int i = 0; i < 8; i++)
    {
        if (object->city_buys_resource[i] == resource)
        {
            return 1;
        }
    }
    return 0;
}

int empire_object_city_sells_resource(int object_id, int resource)
{
    const full_empire_object *object = &objects[object_id];
    for (int i = 0; i < 10; i++)
    {
        if (object->city_sells_resource[i] == resource)
        {
            return 1;
        }
    }
    return 0;
}

static int is_trade_city(int index)
{
    if (objects[index].obj.type != EMPIRE_OBJECT_CITY)
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

static int is_sea_trade_route(int route_id)
{
    for (int i = 0; i < MAX_OBJECTS; i++)
    {
        if (objects[i].in_use && objects[i].obj.trade_route_id == route_id)
        {
            if (objects[i].obj.type == EMPIRE_OBJECT_SEA_TRADE_ROUTE)
            {
                return 1;
            }
            if (objects[i].obj.type == EMPIRE_OBJECT_LAND_TRADE_ROUTE)
            {
                return 0;
            }
        }
    }
    return 0;
}

void empire_object_update_animation(int object_id, int new_animation_index)
{
    objects[object_id].obj.animation_index = new_animation_index;
}
