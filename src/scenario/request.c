#include "request.h"

#include "building/granary.h"
#include "building/warehouse.h"
#include "city/finance.h"
#include "city/message.h"
#include "city/population.h"
#include "city/ratings.h"
#include "city/resource.h"
#include "core/random.h"
#include "game/resource.h"
#include "game/save_version.h"
#include "game/time.h"
#include "game/tutorial.h"
#include "scenario/data.h"

static void make_request_visible_and_send_message(int id)
{
    scenario.requests[id].visible = 1;
    if (city_resource_count(scenario.requests[id].resource) >= scenario.requests[id].amount) {
        scenario.requests[id].can_comply_dialog_shown = 1;
    }
    if (scenario.requests[id].resource == RESOURCE_DENARII) {
        city_message_post(1, MESSAGE_CAESAR_REQUESTS_MONEY, id, 0);
    } else if (scenario.requests[id].resource == RESOURCE_TROOPS) {
        city_message_post(1, MESSAGE_CAESAR_REQUESTS_ARMY, id, 0);
    } else {
        city_message_post(1, MESSAGE_CAESAR_REQUESTS_GOODS, id, 0);
    }
}

void scenario_request_init(void)
{
    for (int i = 0; i < MAX_REQUESTS; i++) {
        random_generate_next();
        if (scenario.requests[i].resource) {
            scenario.requests[i].month = (random_byte() & 7) + 2;
            scenario.requests[i].months_to_comply = 12 * scenario.requests[i].deadline_years;
        }
    }
}

void scenario_request_process(void)
{
    for (int i = 0; i < MAX_REQUESTS; i++) {
        if (!scenario.requests[i].resource || scenario.requests[i].state > REQUEST_STATE_DISPATCHED_LATE) {
            continue;
        }
        int state = scenario.requests[i].state;
        if (state == REQUEST_STATE_DISPATCHED || state == REQUEST_STATE_DISPATCHED_LATE) {
            --scenario.requests[i].months_to_comply;
            if (scenario.requests[i].months_to_comply <= 0) {
                if (state == REQUEST_STATE_DISPATCHED) {
                    city_message_post(1, MESSAGE_REQUEST_RECEIVED, i, 0);
                    city_ratings_change_favor(scenario.requests[i].favor);
                } else {
                    city_message_post(1, MESSAGE_REQUEST_RECEIVED_LATE, i, 0);
                    city_ratings_change_favor(scenario.requests[i].favor / 2);
                }
                scenario.requests[i].state = REQUEST_STATE_RECEIVED;
                scenario.requests[i].visible = 0;
            }
        } else {
            // normal or overdue
            if (scenario.requests[i].visible) {
                --scenario.requests[i].months_to_comply;
                if (state == REQUEST_STATE_NORMAL) {
                    if (scenario.requests[i].months_to_comply == 12) {
                        // reminder
                        city_message_post(1, MESSAGE_REQUEST_REMINDER, i, 0);
                    } else if (scenario.requests[i].months_to_comply <= 0) {
                        city_message_post(1, MESSAGE_REQUEST_REFUSED, i, 0);
                        scenario.requests[i].state = REQUEST_STATE_OVERDUE;
                        scenario.requests[i].months_to_comply = scenario.requests[i].extension_months_to_comply;
                        city_ratings_reduce_favor_missed_request(scenario.requests[i].extension_disfavor);
                    }
                } else if (state == REQUEST_STATE_OVERDUE) {
                    if (scenario.requests[i].months_to_comply <= 0) {
                        city_message_post(1, MESSAGE_REQUEST_REFUSED_OVERDUE, i, 0);
                        scenario.requests[i].state = REQUEST_STATE_IGNORED;
                        scenario.requests[i].visible = 0;
                        city_ratings_reduce_favor_missed_request(scenario.requests[i].ignored_disfavor);
                    }
                }
                if (!scenario.requests[i].can_comply_dialog_shown) {
                    resource_type resource = scenario.requests[i].resource;
                    int resource_amount = city_resource_count(resource);
                    if (resource_is_food(resource)) {
                        resource_amount += city_resource_count_food_on_granaries(resource) / RESOURCE_ONE_LOAD;
                    }
                    if (resource_amount >= scenario.requests[i].amount) {
                        scenario.requests[i].can_comply_dialog_shown = 1;
                        city_message_post(1, MESSAGE_REQUEST_CAN_COMPLY, i, 0);
                    }
                }
            } else {
                // request is not visible
                int year = scenario.start_year;
                if (!tutorial_adjust_request_year(&year)) {
                    return;
                }
                if (game_time_year() == year + scenario.requests[i].year &&
                    game_time_month() == scenario.requests[i].month) {
                    make_request_visible_and_send_message(i);
                }
            }
        }
    }
}

void scenario_request_dispatch(int id)
{
    if (scenario.requests[id].state == REQUEST_STATE_NORMAL) {
        scenario.requests[id].state = REQUEST_STATE_DISPATCHED;
    } else {
        scenario.requests[id].state = REQUEST_STATE_DISPATCHED_LATE;
    }
    scenario.requests[id].months_to_comply = (random_byte() & 3) + 1;
    scenario.requests[id].visible = 0;
    int amount = scenario.requests[id].amount;
    if (scenario.requests[id].resource == RESOURCE_DENARII) {
        city_finance_process_sundry(amount);
    } else if (scenario.requests[id].resource == RESOURCE_TROOPS) {
        city_population_remove_for_troop_request(amount);
        building_warehouses_remove_resource(RESOURCE_WEAPONS, amount);
    } else {
        int amount_left = building_warehouses_send_resources_to_rome(scenario.requests[id].resource, amount);
        if (amount_left > 0 && resource_is_food(scenario.requests[id].resource)) {
            building_granaries_send_resources_to_rome(scenario.requests[id].resource, amount_left * RESOURCE_ONE_LOAD);
        }
    }
}

const scenario_request *scenario_request_get(int id)
{
    static scenario_request request;
    request.id = id;
    request.amount = scenario.requests[id].amount;
    request.resource = scenario.requests[id].resource;
    request.state = scenario.requests[id].state;
    request.months_to_comply = scenario.requests[id].months_to_comply;
    return &request;
}

int scenario_request_count_visible(void)
{
    int count = 0;
    for (int i = 0; i < MAX_REQUESTS; i++) {
        if (scenario.requests[i].resource && scenario.requests[i].visible) {
            count++;
        }
    }
    return count;
}

int scenario_request_foreach_visible(int start_index, void (*callback)(int index, const scenario_request *request))
{
    int index = start_index;
    for (int i = 0; i < MAX_REQUESTS; i++) {
        if (scenario.requests[i].resource && scenario.requests[i].visible) {
            callback(index, scenario_request_get(i));
            index++;
        }
    }
    return index;
}

const scenario_request *scenario_request_get_visible(int index)
{
    for (int i = 0; i < MAX_REQUESTS; i++) {
        if (scenario.requests[i].resource && scenario.requests[i].visible &&
            scenario.requests[i].state <= 1) {
            if (index == 0) {
                return scenario_request_get(i);
            }
            index--;
        }
    }
    return 0;
}

int scenario_request_is_ongoing(int id)
{
    if (id < 0 || id >= MAX_REQUESTS) {
        return 0;
    }

    if (!scenario.requests[id].resource) {
        return 0;
    }
    
    if (scenario.requests[id].visible
        && (scenario.requests[id].state == REQUEST_STATE_NORMAL
            || scenario.requests[id].state == REQUEST_STATE_OVERDUE)
        ) {
        return 1;
    }

    if (!scenario.requests[id].visible
        && (scenario.requests[id].state == REQUEST_STATE_DISPATCHED
            || scenario.requests[id].state == REQUEST_STATE_DISPATCHED_LATE)
        ) {
        return 1;
    }

    return 0;
}

int scenario_request_force_start(int id)
{
    if (id < 0 || id >= MAX_REQUESTS) {
        return 0;
    }
    
    if (!scenario.requests[id].resource) {
        return 0;
    }

    if (scenario_request_is_ongoing(id)) {
        return 0;
    }

    scenario.requests[id].state = REQUEST_STATE_NORMAL;
    scenario.requests[id].months_to_comply = 12 * scenario.requests[id].deadline_years;
    scenario.requests[id].year = game_time_year();
    scenario.requests[id].month = game_time_month();
    scenario.requests[id].can_comply_dialog_shown = 0;
    
    make_request_visible_and_send_message(id);

    return 1;
}

static void request_save(buffer *list, int index)
{
    buffer_write_i16(list, scenario.requests[index].year);
    buffer_write_i16(list, scenario.requests[index].resource);
    buffer_write_i16(list, scenario.requests[index].amount);
    buffer_write_i16(list, scenario.requests[index].deadline_years);

    buffer_write_u8(list, scenario.requests[index].can_comply_dialog_shown);

    buffer_write_u8(list, scenario.requests[index].favor);
    buffer_write_u8(list, scenario.requests[index].month);
    buffer_write_u8(list, scenario.requests[index].state);
    buffer_write_u8(list, scenario.requests[index].visible);
    buffer_write_u8(list, scenario.requests[index].months_to_comply);

    buffer_write_i16(list, scenario.requests[index].extension_months_to_comply);
    buffer_write_i16(list, scenario.requests[index].extension_disfavor);
    buffer_write_i16(list, scenario.requests[index].ignored_disfavor);
}

void scenario_request_save_state(buffer *list)
{
    int32_t array_size = MAX_REQUESTS;
    int32_t struct_size = (7 * sizeof(int16_t)) + (6 * sizeof(uint8_t));
    buffer_init_dynamic_piece(list,
        SCENARIO_CURRENT_VERSION,
        array_size,
        struct_size);

    for (int i = 0; i < array_size; i++) {
        request_save(list, i);
    }
}

static void request_load(buffer *list, int index, int version)
{
    scenario.requests[index].year = buffer_read_i16(list);
    scenario.requests[index].resource = buffer_read_i16(list);
    scenario.requests[index].amount = buffer_read_i16(list);
    scenario.requests[index].deadline_years = buffer_read_i16(list);
    scenario.requests[index].can_comply_dialog_shown = buffer_read_u8(list);
    scenario.requests[index].favor = buffer_read_u8(list);
    scenario.requests[index].month = buffer_read_u8(list);
    scenario.requests[index].state = buffer_read_u8(list);
    scenario.requests[index].visible = buffer_read_u8(list);
    scenario.requests[index].months_to_comply = buffer_read_u8(list);
    scenario.requests[index].extension_months_to_comply = buffer_read_i16(list);
    scenario.requests[index].extension_disfavor = buffer_read_i16(list);
    scenario.requests[index].ignored_disfavor = buffer_read_i16(list);
}

void scenario_request_load_state(buffer *list)
{
    int buffer_size, version, array_size, struct_size;

    buffer_load_dynamic_piece_header_data(list,
        &buffer_size,
        &version,
        &array_size,
        &struct_size);

    for (int i = 0; i < array_size; i++) {
        request_load(list, i, version);
    }
}

void scenario_request_load_state_old_version(buffer *list, int state_version, requests_old_state_sections section)
{
    // Old savegames had request data split out into multiple chunks,
    // and saved as multiple arrays of variables, rather than an array of struct approach.
    // So here we need to load in a similar section / varaible array manner when dealing with old versions.
    if (state_version <= SCENARIO_LAST_NO_EXTENDED_REQUESTS) {
        if (section == REQUESTS_OLD_STATE_SECTIONS_TARGET) {
            for (int i = 0; i < MAX_REQUESTS; i++) {
                scenario.requests[i].year = buffer_read_i16(list);
            }
            for (int i = 0; i < MAX_REQUESTS; i++) {
                scenario.requests[i].resource = buffer_read_i16(list);
            }
            for (int i = 0; i < MAX_REQUESTS; i++) {
                scenario.requests[i].amount = buffer_read_i16(list);
            }
            for (int i = 0; i < MAX_REQUESTS; i++) {
                scenario.requests[i].deadline_years = buffer_read_i16(list);
            }
        } else if (section == REQUESTS_OLD_STATE_SECTIONS_CAN_COMPLY) {
            for (int i = 0; i < MAX_REQUESTS; i++) {
                scenario.requests[i].can_comply_dialog_shown = buffer_read_u8(list);
            }
        } else if (section == REQUESTS_OLD_STATE_SECTIONS_FAVOR_REWARD) {
            for (int i = 0; i < MAX_REQUESTS; i++) {
                scenario.requests[i].favor = buffer_read_u8(list);
            }
        } else if (section == REQUESTS_OLD_STATE_SECTIONS_ONGOING_INFO) {
            for (int i = 0; i < MAX_REQUESTS; i++) {
                scenario.requests[i].month = buffer_read_u8(list);
            }
            for (int i = 0; i < MAX_REQUESTS; i++) {
                scenario.requests[i].state = buffer_read_u8(list);
            }
            for (int i = 0; i < MAX_REQUESTS; i++) {
                scenario.requests[i].visible = buffer_read_u8(list);
            }
            for (int i = 0; i < MAX_REQUESTS; i++) {
                scenario.requests[i].months_to_comply = buffer_read_u8(list);
            }
            // Setup any default values we need for values that didn't exist in old versions.
            for (int i = 0; i < MAX_REQUESTS; i++) {
                scenario.requests[i].extension_months_to_comply = REQUESTS_DEFAULT_MONTHS_TO_COMPLY;
                scenario.requests[i].extension_disfavor = REQUESTS_DEFAULT_EXTENSION_DISFAVOUR;
                scenario.requests[i].ignored_disfavor = REQUESTS_DEFAULT_IGNORED_DISFAVOUR;
            }
        }
    }
}
