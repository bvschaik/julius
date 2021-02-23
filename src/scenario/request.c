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
#include "game/time.h"
#include "game/tutorial.h"
#include "scenario/data.h"

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
                        scenario.requests[i].months_to_comply = 24;
                        city_ratings_reduce_favor_missed_request(3);
                    }
                } else if (state == REQUEST_STATE_OVERDUE) {
                    if (scenario.requests[i].months_to_comply <= 0) {
                        city_message_post(1, MESSAGE_REQUEST_REFUSED_OVERDUE, i, 0);
                        scenario.requests[i].state = REQUEST_STATE_IGNORED;
                        scenario.requests[i].visible = 0;
                        city_ratings_reduce_favor_missed_request(5);
                    }
                }
                if (!scenario.requests[i].can_comply_dialog_shown) {
                    resource_type resource = scenario.requests[i].resource;
                    int resource_amount = city_resource_count(resource);
                    if (city_resource_is_food(resource)) {
                        resource_amount += city_resource_count_food_on_granaries(resource) / RESOURCE_GRANARY_ONE_LOAD;
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
                    scenario.requests[i].visible = 1;
                    if (city_resource_count(scenario.requests[i].resource) >= scenario.requests[i].amount) {
                        scenario.requests[i].can_comply_dialog_shown = 1;
                    }
                    if (scenario.requests[i].resource == RESOURCE_DENARII) {
                        city_message_post(1, MESSAGE_CAESAR_REQUESTS_MONEY, i, 0);
                    } else if (scenario.requests[i].resource == RESOURCE_TROOPS) {
                        city_message_post(1, MESSAGE_CAESAR_REQUESTS_ARMY, i, 0);
                    } else {
                        city_message_post(1, MESSAGE_CAESAR_REQUESTS_GOODS, i, 0);
                    }
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
        int amount_left = building_warehouses_remove_resource(scenario.requests[id].resource, amount);
        if (amount_left > 0 && city_resource_is_food(scenario.requests[id].resource)) {
            building_granaries_remove_resource(scenario.requests[id].resource, amount_left * RESOURCE_GRANARY_ONE_LOAD);
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
