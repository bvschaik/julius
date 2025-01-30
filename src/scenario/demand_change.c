#include "demand_change.h"

#include "city/message.h"
#include "core/array.h"
#include "core/log.h"
#include "core/random.h"
#include "empire/city.h"
#include "empire/trade_route.h"
#include "game/time.h"
#include "scenario/property.h"

#define DEMAND_CHANGES_ARRAY_SIZE_STEP 16

#define DEMAND_CHANGES_STRUCT_SIZE_CURRENT (1 * sizeof(int32_t) + 1 * sizeof(int16_t) + 3 * sizeof(uint8_t))

static array(demand_change_t) demand_changes;

static void new_demand_change(demand_change_t *demand_change, unsigned int index)
{
    demand_change->id = index;
}

static int demand_change_in_use(const demand_change_t *demand_change)
{
    return demand_change->year != 0;
}

void scenario_demand_change_clear_all(void)
{
    if (!array_init(demand_changes, DEMAND_CHANGES_ARRAY_SIZE_STEP, new_demand_change, demand_change_in_use)) {
        log_error("Problem creating demand changes array - memory full. The gane will now crash.", 0, 0);
    }
}

void scenario_demand_change_init(void)
{
    demand_change_t *demand_change;
    array_foreach(demand_changes, demand_change) {
        random_generate_next();
        if (demand_change->year) {
            demand_change->month = (random_byte() & 7) + 2;
        }
    }
}

int scenario_demand_change_new(void)
{
    demand_change_t *demand_change;
    array_new_item(demand_changes, demand_change);
    return demand_change ? demand_change->id : -1;
}

static void process_demand_change(demand_change_t *demand_change)
{
    if (!demand_change->year) {
        return;
    }
    if (game_time_year() != demand_change->year + scenario_property_start_year() ||
        game_time_month() != demand_change->month) {
        return;
    }
    int route = demand_change->route_id;
    int resource = demand_change->resource;
    int city_id = empire_city_get_for_trade_route(route);
    if (city_id < 0) {
        city_id = 0;
    }

    int last_amount = trade_route_limit(route, resource);
    int amount = demand_change->amount;
    if (amount == DEMAND_CHANGE_LEGACY_IS_RISE) {
        amount = trade_route_legacy_increase_limit(route, resource);
    } else if (amount == DEMAND_CHANGE_LEGACY_IS_FALL) {
        amount = trade_route_legacy_decrease_limit(route, resource);
    } else {
        trade_route_set_limit(route, resource, amount);
    }
    if (empire_city_is_trade_route_open(route)) {
        int change = amount - last_amount;
        if (amount > 0 && change > 0) {
            city_message_post(1, MESSAGE_INCREASED_TRADING, city_id, resource);
        } else if (amount > 0 && change < 0) {
            city_message_post(1, MESSAGE_DECREASED_TRADING, city_id, resource);
        } else if (amount <= 0) {
            city_message_post(1, MESSAGE_TRADE_STOPPED, city_id, resource);
        }
    }
}

void scenario_demand_change_process(void)
{
    array_foreach_callback(demand_changes, process_demand_change);
}

const demand_change_t *scenario_demand_change_get(int id)
{
    return array_item(demand_changes, id);
}

void scenario_demand_change_update(const demand_change_t *demand_change)
{
    demand_change_t *base_demand_change = array_item(demand_changes, demand_change->id);
    *base_demand_change = *demand_change;
    array_trim(demand_changes);
}

void scenario_demand_change_delete(int id)
{
    demand_change_t *demand_change = array_item(demand_changes, id);
    memset(demand_change, 0, sizeof(demand_change_t));
    array_trim(demand_changes);
}

void scenario_demand_change_remap_resource(void)
{
    demand_change_t *demand_change;
    array_foreach(demand_changes, demand_change) {
        demand_change->resource = resource_remap(demand_change->resource);
    }
}

int scenario_demand_change_count_total(void)
{
    return demand_changes.size;
}

void scenario_demand_change_save_state(buffer *buf)
{
    buffer_init_dynamic_array(buf, demand_changes.size, DEMAND_CHANGES_STRUCT_SIZE_CURRENT);

    const demand_change_t *demand_change;
    array_foreach(demand_changes, demand_change) {
        buffer_write_i16(buf, demand_change->year);
        buffer_write_u8(buf, demand_change->month);
        buffer_write_u8(buf, demand_change->resource);
        buffer_write_u8(buf, demand_change->route_id);
        buffer_write_i32(buf, demand_change->amount);
    }
}

void scenario_demand_change_load_state(buffer *buf)
{
    unsigned int size = buffer_load_dynamic_array(buf);

    if (!array_init(demand_changes, DEMAND_CHANGES_ARRAY_SIZE_STEP, new_demand_change, demand_change_in_use) ||
        !array_expand(demand_changes, size)) {
        log_error("Problem creating demand changes array - memory full. The gane will now crash.", 0, 0);
    }

    for (unsigned int i = 0; i < size; i++) {
        demand_change_t *demand_change = array_next(demand_changes);
        demand_change->year = buffer_read_i16(buf);
        demand_change->month = buffer_read_u8(buf);
        demand_change->resource = buffer_read_u8(buf);
        demand_change->route_id = buffer_read_u8(buf);
        demand_change->amount = buffer_read_i32(buf);
    }

    array_trim(demand_changes);
}

void scenario_demand_change_load_state_old_version(buffer *buf, int is_legacy_change)
{
    if (!array_init(demand_changes, DEMAND_CHANGES_ARRAY_SIZE_STEP, new_demand_change, demand_change_in_use) ||
        !array_expand(demand_changes, MAX_ORIGINAL_DEMAND_CHANGES)) {
        log_error("Problem creating demand changes array - memory full. The gane will now crash.", 0, 0);
    }
    for (size_t i = 0; i < MAX_ORIGINAL_DEMAND_CHANGES; i++) {
        array_advance(demand_changes);
    }
    demand_change_t *demand_change;
    array_foreach(demand_changes, demand_change) {
        demand_change->year = buffer_read_i16(buf);
    }
    array_foreach(demand_changes, demand_change) {
        demand_change->month = buffer_read_u8(buf);
    }
    array_foreach(demand_changes, demand_change) {
        demand_change->resource = buffer_read_u8(buf);
    }
    array_foreach(demand_changes, demand_change) {
        demand_change->route_id = buffer_read_u8(buf);
    }
    if (is_legacy_change) {
        array_foreach(demand_changes, demand_change) {
            int is_rise = buffer_read_u8(buf);
            int amount = is_rise ? DEMAND_CHANGE_LEGACY_IS_RISE : DEMAND_CHANGE_LEGACY_IS_FALL;
            demand_change->amount = amount;
        }
    } else {
        array_foreach(demand_changes, demand_change) {
            demand_change->amount = buffer_read_i32(buf);
        }
    }
    array_trim(demand_changes);
}
