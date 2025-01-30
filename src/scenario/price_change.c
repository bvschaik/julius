#include "price_change.h"

#include "city/message.h"
#include "core/array.h"
#include "core/log.h"
#include "core/random.h"
#include "empire/trade_prices.h"
#include "game/time.h"
#include "scenario/property.h"

#define PRICE_CHANGES_ARRAY_SIZE_STEP 16

#define PRICE_CHANGES_STRUCT_SIZE_CURRENT (1 * sizeof(int16_t) + 4 * sizeof(uint8_t))

static array(price_change_t) price_changes;

static void new_price_change(price_change_t *price_change, unsigned int index)
{
    price_change->id = index;
}

static int price_change_in_use(const price_change_t *price_change)
{
    return price_change->year != 0;
}

void scenario_price_change_clear_all(void)
{
    if (!array_init(price_changes, PRICE_CHANGES_ARRAY_SIZE_STEP, new_price_change, price_change_in_use)) {
        log_error("Problem creating price changes array - memory full. The gane will now crash.", 0, 0);
    }
}

void scenario_price_change_init(void)
{
    price_change_t *price_change;
    array_foreach(price_changes, price_change)
    {
        random_generate_next();
        if (price_change->year) {
            price_change->month = (random_byte() & 7) + 2;
        }
    }
}

int scenario_price_change_new(void)
{
    price_change_t *price_change;
    array_new_item(price_changes, price_change);
    return price_change ? price_change->id : -1;
}

static void process_price_change(price_change_t *price_change)
{
    if (!price_change->year) {
        return;
    }
    if (game_time_year() != price_change->year + scenario_property_start_year() ||
        game_time_month() != price_change->month) {
        return;
    }
    if (price_change->is_rise) {
        if (trade_price_change(price_change->resource, price_change->amount)) {
            city_message_post(1, MESSAGE_PRICE_INCREASED, price_change->amount, price_change->resource);
        }
    } else {
        if (trade_price_change(price_change->resource, -price_change->amount)) {
            city_message_post(1, MESSAGE_PRICE_DECREASED, price_change->amount, price_change->resource);
        }
    }
}

void scenario_price_change_process(void)
{
    array_foreach_callback(price_changes, process_price_change);
}

const price_change_t *scenario_price_change_get(int id)
{
    return array_item(price_changes, id);
}

void scenario_price_change_update(const price_change_t *price_change)
{
    price_change_t *base_price_change = array_item(price_changes, price_change->id);
    *base_price_change = *price_change;
    array_trim(price_changes);
}

void scenario_price_change_delete(int id)
{
    price_change_t *price_change = array_item(price_changes, id);
    memset(price_change, 0, sizeof(price_change_t));
    array_trim(price_changes);
}

void scenario_price_change_remap_resource(void)
{
    price_change_t *price_change;
    array_foreach(price_changes, price_change) {
        price_change->resource = resource_remap(price_change->resource);
    }
}

int scenario_price_change_count_total(void)
{
    return price_changes.size;
}

void scenario_price_change_save_state(buffer *buf)
{
    buffer_init_dynamic_array(buf, price_changes.size, PRICE_CHANGES_STRUCT_SIZE_CURRENT);

    const price_change_t *price_change;
    array_foreach(price_changes, price_change) {
        buffer_write_i16(buf, price_change->year);
        buffer_write_u8(buf, price_change->month);
        buffer_write_u8(buf, price_change->resource);
        buffer_write_u8(buf, price_change->amount);
        buffer_write_u8(buf, price_change->is_rise);
    }
}

void scenario_price_change_load_state(buffer *buf)
{
    unsigned int size = buffer_load_dynamic_array(buf);

    if (!array_init(price_changes, PRICE_CHANGES_ARRAY_SIZE_STEP, new_price_change, price_change_in_use) ||
        !array_expand(price_changes, size)) {
        log_error("Problem creating price changes array - memory full. The gane will now crash.", 0, 0);
    }

    for (unsigned int i = 0; i < size; i++) {
        price_change_t *price_change = array_next(price_changes);
        price_change->year = buffer_read_i16(buf);
        price_change->month = buffer_read_u8(buf);
        price_change->resource = buffer_read_u8(buf);
        price_change->amount = buffer_read_u8(buf);
        price_change->is_rise = buffer_read_u8(buf);
    }

    array_trim(price_changes);
}

void scenario_price_change_load_state_old_version(buffer *buf)
{
    if (!array_init(price_changes, PRICE_CHANGES_ARRAY_SIZE_STEP, new_price_change, price_change_in_use) ||
        !array_expand(price_changes, MAX_ORIGINAL_PRICE_CHANGES)) {
        log_error("Problem creating price changes array - memory full. The gane will now crash.", 0, 0);
    }
    for (size_t i = 0; i < MAX_ORIGINAL_PRICE_CHANGES; i++) {
        array_advance(price_changes);
    }
    price_change_t *price_change;
    array_foreach(price_changes, price_change) {
        price_change->year = buffer_read_i16(buf);
    }
    array_foreach(price_changes, price_change) {
        price_change->month = buffer_read_u8(buf);
    }
    array_foreach(price_changes, price_change) {
        price_change->resource = buffer_read_u8(buf);
    }
    array_foreach(price_changes, price_change) {
        price_change->amount = buffer_read_u8(buf);
    }
    array_foreach(price_changes, price_change) {
        price_change->is_rise = buffer_read_u8(buf);
    }
    array_trim(price_changes);
}
