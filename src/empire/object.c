#include "object.h"

#include "core/calc.h"
#include "core/image.h"
#include "empire/city.h"
#include "empire/trade_route.h"
#include "empire/type.h"
#include "game/animation.h"
#include "game/file_io.h"
#include "scenario/empire.h"

#include <stdlib.h>
#include <string.h>

static full_empire_object objects[MAX_EMPIRE_OBJECTS];

static void fix_image_ids(void)
{
    int image_id = 0;
    for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
        if (objects[i].in_use
            && objects[i].obj.type == EMPIRE_OBJECT_CITY
            && objects[i].city_type == EMPIRE_CITY_OURS) {
            image_id = objects[i].obj.image_id;
            break;
        }
    }
    if (image_id > 0 && image_id != image_group(GROUP_EMPIRE_CITY)) {
        // empire map uses old version of graphics: increase every graphic id
        int offset = image_group(GROUP_EMPIRE_CITY) - image_id;
        for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
            if (!objects[i].in_use) {
                continue;
            }
            if (objects[i].obj.image_id) {
                objects[i].obj.image_id += offset;
                if (objects[i].obj.expanded.image_id) {
                    objects[i].obj.expanded.image_id += offset;
                }
            }
        }
    }
}

void empire_object_clear(void)
{
    memset(objects, 0, sizeof(objects));
}

void empire_object_load(buffer *buf, int version)
{
    empire_object_clear();
    // we're loading a scenario that does not have a custom empire
    if (buf->size == sizeof(int) && buffer_read_i32(buf) == 0) {
        return;
    }

    for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
        full_empire_object *full = &objects[i];
        empire_object *obj = &full->obj;
        obj->id = i;
        obj->type = buffer_read_u8(buf);
        full->in_use = buffer_read_u8(buf);
        if (version > SCENARIO_LAST_UNVERSIONED && !full->in_use) {
            continue;
        }

        obj->animation_index = buffer_read_u8(buf);
        if (version <= SCENARIO_LAST_EMPIRE_OBJECT_BUFFERS) {
            buffer_skip(buf, 1);
        }
        obj->x = buffer_read_i16(buf);
        obj->y = buffer_read_i16(buf);
        obj->width = buffer_read_i16(buf);
        obj->height = buffer_read_i16(buf);
        obj->image_id = buffer_read_i16(buf);
        obj->expanded.image_id = buffer_read_i16(buf);
        if (version <= SCENARIO_LAST_EMPIRE_OBJECT_BUFFERS) {
            buffer_skip(buf, 1);
        }
        obj->distant_battle_travel_months = buffer_read_u8(buf);
        if (version <= SCENARIO_LAST_EMPIRE_OBJECT_BUFFERS) {
            buffer_skip(buf, 2);
        }
        obj->expanded.x = buffer_read_i16(buf);
        obj->expanded.y = buffer_read_i16(buf);
        full->city_type = buffer_read_u8(buf);
        full->city_name_id = buffer_read_u8(buf);
        obj->trade_route_id = buffer_read_u8(buf);
        full->trade_route_open = buffer_read_u8(buf);
        full->trade_route_cost = buffer_read_i16(buf);
        int old_sells_resource[10];
        int old_buys_resource[8];
        if (version <= SCENARIO_LAST_UNVERSIONED) {
            for (int r = 0; r < 10; r++) {
                old_sells_resource[r] = buffer_read_u8(buf);
            }
            buffer_skip(buf, 2);
            for (int r = 0; r < 8; r++) {
                old_buys_resource[r] = buffer_read_u8(buf);
            }
        } else if (version <= SCENARIO_LAST_EMPIRE_RESOURCES_U8) {
            for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
                full->city_sells_resource[r] = buffer_read_u8(buf);
            }
            for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
                full->city_buys_resource[r] = buffer_read_u8(buf);
            }
        } else if (version <= SCENARIO_LAST_EMPIRE_RESOURCES_ALWAYS_WRITE) {
            for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
                full->city_sells_resource[r] = buffer_read_i32(buf);
            }
            for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
                full->city_buys_resource[r] = buffer_read_i32(buf);
            }
        } else if (obj->type == EMPIRE_OBJECT_CITY) {
            for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
                full->city_sells_resource[r] = buffer_read_i16(buf);
            }
            for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
                full->city_buys_resource[r] = buffer_read_i16(buf);
            }
        }
        obj->invasion_path_id = buffer_read_u8(buf);
        obj->invasion_years = buffer_read_u8(buf);
        if (version <= SCENARIO_LAST_UNVERSIONED) {
            int trade40 = buffer_read_u16(buf);
            int trade25 = buffer_read_u16(buf);
            int trade15 = buffer_read_u16(buf);
            for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
                int resource_flag = 1 << r;
                int amount = 0;
                if (trade40 & resource_flag) {
                    amount = 40;
                } else if (trade25 & resource_flag) {
                    amount = 25;
                } else if (trade15 & resource_flag) {
                    amount = 15;
                } else if (full->city_type == EMPIRE_CITY_OURS) {
                    // our city is special and won't actually have an amount for goods it sells, so we set it to 1
                    amount = 1;
                }
                for (int i = 0; i < 10; i++) {
                    if (old_sells_resource[i] == r) {
                        full->city_sells_resource[r] = amount;
                        break;
                    }
                }
                for (int i = 0; i < 8; i++) {
                    if (old_buys_resource[i] == r) {
                        full->city_buys_resource[r] = amount;
                        break;
                    }
                }
            }
        }
        if (version <= SCENARIO_LAST_EMPIRE_OBJECT_BUFFERS) {
            buffer_skip(buf, 6);
        }

        if (version > SCENARIO_LAST_UNVERSIONED) {
            buffer_read_raw(buf, full->city_custom_name, sizeof(full->city_custom_name));
        }
    }

    fix_image_ids();
}

void empire_object_save(buffer *buf)
{
    char *buf_data;
    if (scenario.empire.id != SCENARIO_CUSTOM_EMPIRE) {
        buf_data = malloc(sizeof(int));
        buffer_init(buf, buf_data, sizeof(int));
        buffer_write_i32(buf, 0);
        return;
    }
    int size_per_obj = 78;
    int size_per_city = 138;
    int total_size = 0;
    for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
        full_empire_object *full = &objects[i];
        if (full->in_use && full->obj.type == EMPIRE_OBJECT_CITY) {
            total_size += size_per_city;
        } else if (full->in_use) {
            total_size += size_per_obj;
        } else {
            total_size += 2;
        }
    }
    buf_data = malloc(total_size);
    buffer_init(buf, buf_data, total_size);

    for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
        full_empire_object *full = &objects[i];
        empire_object *obj = &full->obj;
        buffer_write_u8(buf, obj->type);
        buffer_write_u8(buf, full->in_use);
        if (!full->in_use) {
            continue;
        }

        buffer_write_u8(buf, obj->animation_index);
        buffer_write_i16(buf, obj->x);
        buffer_write_i16(buf, obj->y);
        buffer_write_i16(buf, obj->width);
        buffer_write_i16(buf, obj->height);
        buffer_write_i16(buf, obj->image_id);
        buffer_write_i16(buf, obj->expanded.image_id);
        buffer_write_u8(buf, obj->distant_battle_travel_months);
        buffer_write_i16(buf, obj->expanded.x);
        buffer_write_i16(buf, obj->expanded.y);
        buffer_write_u8(buf, full->city_type);
        buffer_write_u8(buf, full->city_name_id);
        buffer_write_u8(buf, obj->trade_route_id);
        buffer_write_u8(buf, full->trade_route_open);
        buffer_write_i16(buf, full->trade_route_cost);
        if (obj->type == EMPIRE_OBJECT_CITY) {
            for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
                buffer_write_i16(buf, full->city_sells_resource[r]);
            }
            for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
                buffer_write_i16(buf, full->city_buys_resource[r]);
            }
        }
        buffer_write_u8(buf, obj->invasion_path_id);
        buffer_write_u8(buf, obj->invasion_years);
        buffer_write_raw(buf, full->city_custom_name, sizeof(full->city_custom_name));
    }
}

void empire_object_init_cities(int empire_id)
{
    empire_city_clear_all();
    int route_index = 1;
    for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
        if (!objects[i].in_use || objects[i].obj.type != EMPIRE_OBJECT_CITY) {
            continue;
        }
        full_empire_object *obj = &objects[i];
        empire_city *city = empire_city_get(route_index++);
        city->in_use = 1;
        city->type = obj->city_type;
        city->name_id = obj->city_name_id;
        if (obj->obj.trade_route_id < 0) {
            obj->obj.trade_route_id = 0;
        }
        if (obj->obj.trade_route_id >= 20 && empire_id != SCENARIO_CUSTOM_EMPIRE) {
            obj->obj.trade_route_id = 19;
        }
        city->route_id = obj->obj.trade_route_id;
        city->is_open = obj->trade_route_open;
        city->cost_to_open = obj->trade_route_cost;
        city->is_sea_trade = is_sea_trade_route(obj->obj.trade_route_id);

        for (int resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
            city->sells_resource[resource] = 0;
            city->buys_resource[resource] = 0;
            if (city->type == EMPIRE_CITY_DISTANT_ROMAN
                || city->type == EMPIRE_CITY_DISTANT_FOREIGN
                || city->type == EMPIRE_CITY_VULNERABLE_ROMAN
                || city->type == EMPIRE_CITY_FUTURE_ROMAN) {
                continue;
            }
            if (empire_object_city_sells_resource(i, resource)) {
                city->sells_resource[resource] = 1;
            }
            if (empire_object_city_buys_resource(i, resource)) {
                city->buys_resource[resource] = 1;
            }
            int amount = 0;
            if (obj->city_buys_resource[resource]) {
                amount = obj->city_buys_resource[resource];
            } else if (obj->city_sells_resource[resource]) {
                amount = obj->city_sells_resource[resource];
            }
            trade_route_init(city->route_id, resource, amount);
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
    for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
        if (objects[i].in_use && objects[i].obj.type == object_type) {
            month++;
            objects[i].obj.distant_battle_travel_months = month;
        }
    }
    return month;
}

full_empire_object *empire_object_get_full(int object_id)
{
    return &objects[object_id];
}

empire_object *empire_object_get(int object_id)
{
    return &objects[object_id].obj;
}

const empire_object *empire_object_get_our_city(void)
{
    for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
        if (objects[i].in_use) {
            const empire_object *obj = &objects[i].obj;
            if (obj->type == EMPIRE_OBJECT_CITY && objects[i].city_type == EMPIRE_CITY_OURS) {
                return obj;
            }
        }
    }
    return 0;
}

const empire_object *empire_object_get_trade_city(int trade_route_id)
{
    for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
        if (objects[i].in_use) {
            const empire_object *obj = &objects[i].obj;
            if (obj->type == EMPIRE_OBJECT_CITY && obj->trade_route_id == trade_route_id) {
                return obj;
            }
        }
    }
    return 0;
}

void empire_object_foreach(void (*callback)(const empire_object *))
{
    for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
        if (objects[i].in_use) {
            callback(&objects[i].obj);
        }
    }
}

const empire_object *empire_object_get_battle_icon(int path_id, int year)
{
    for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
        if (objects[i].in_use) {
            empire_object *obj = &objects[i].obj;
            if (obj->type == EMPIRE_OBJECT_BATTLE_ICON &&
                obj->invasion_path_id == path_id && obj->invasion_years == year) {
                return obj;
            }
        }
    }
    return 0;
}

int empire_object_get_max_invasion_path(void)
{
    int max_path = 0;
    for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
        if (objects[i].in_use && objects[i].obj.type == EMPIRE_OBJECT_BATTLE_ICON) {
            if (objects[i].obj.invasion_path_id > max_path) {
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
    for (int i = 0; i < MAX_EMPIRE_OBJECTS && objects[i].in_use; i++) {
        const empire_object *obj = &objects[i].obj;
        int obj_x, obj_y;
        if (scenario_empire_is_expanded()) {
            obj_x = obj->expanded.x;
            obj_y = obj->expanded.y;
        } else {
            obj_x = obj->x;
            obj_y = obj->y;
        }
        if (obj_x - 8 > x || obj_x + obj->width + 8 <= x) {
            continue;
        }
        if (obj_y - 8 > y || obj_y + obj->height + 8 <= y) {
            continue;
        }
        int dist = calc_maximum_distance(x, y, obj_x + obj->width / 2, obj_y + obj->height / 2);
        if (dist < min_dist) {
            min_dist = dist;
            min_obj_id = i + 1;
        }
    }
    return min_obj_id;
}

void empire_object_set_expanded(int object_id, int new_city_type)
{
    objects[object_id].city_type = new_city_type;
    if (new_city_type == EMPIRE_CITY_TRADE) {
        objects[object_id].obj.expanded.image_id = image_group(GROUP_EMPIRE_CITY_TRADE);
    } else if (new_city_type == EMPIRE_CITY_DISTANT_ROMAN) {
        objects[object_id].obj.expanded.image_id = image_group(GROUP_EMPIRE_CITY_DISTANT_ROMAN);
    }
}

int empire_object_city_buys_resource(int object_id, int resource)
{
    const full_empire_object *object = &objects[object_id];
    if (object->city_buys_resource[resource]) {
        return 1;
    }
    return 0;
}

int empire_object_city_sells_resource(int object_id, int resource)
{
    const full_empire_object *object = &objects[object_id];
    if (object->city_sells_resource[resource]) {
        return 1;
    }
    return 0;
}

void empire_object_city_force_sell_resource(int object_id, int resource)
{
    full_empire_object* object = &objects[object_id];
    for (int i = 0;i < 10;++i) {
        if (object->city_sells_resource[i] == 0) {
            object->city_sells_resource[i] = resource;
        }
    }
    
}

int is_sea_trade_route(int route_id)
{
    for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
        if (objects[i].in_use && objects[i].obj.trade_route_id == route_id) {
            if (objects[i].obj.type == EMPIRE_OBJECT_SEA_TRADE_ROUTE) {
                return 1;
            }
            if (objects[i].obj.type == EMPIRE_OBJECT_LAND_TRADE_ROUTE) {
                return 0;
            }
        }
    }
    return 0;
}

static int get_animation_offset(int image_id, int current_index)
{
    if (current_index <= 0) {
        current_index = 1;
    }
    const image *img = image_get(image_id);
    if (!img->animation) {
        return 0;
    }
    int animation_speed = img->animation->speed_id;
    if (!game_animation_should_advance(animation_speed)) {
        return current_index;
    }
    if (img->animation->can_reverse) {
        int is_reverse = 0;
        if (current_index & 0x80) {
            is_reverse = 1;
        }
        int current_sprite = current_index & 0x7f;
        if (is_reverse) {
            current_index = current_sprite - 1;
            if (current_index < 1) {
                current_index = 1;
                is_reverse = 0;
            }
        } else {
            current_index = current_sprite + 1;
            if (current_index > img->animation->num_sprites) {
                current_index = img->animation->num_sprites;
                is_reverse = 1;
            }
        }
        if (is_reverse) {
            current_index = current_index | 0x80;
        }
    } else {
        // Absolutely normal case
        current_index++;
        if (current_index > img->animation->num_sprites) {
            current_index = 1;
        }
    }
    return current_index;
}

int empire_object_update_animation(const empire_object *obj, int image_id)
{
    return objects[obj->id].obj.animation_index = get_animation_offset(image_id, obj->animation_index);
}
