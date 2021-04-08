#include "request.h"

#include "city/finance.h"
#include "city/military.h"
#include "city/resource.h"
#include "scenario/request.h"

int city_request_has_troop_request(void)
{
    return city_military_months_until_distant_battle() > 0
        && !city_military_distant_battle_roman_army_is_traveling_forth();
}

int city_request_get_status(int index)
{
    int num_requests = 0;
    if (city_request_has_troop_request()) {
        num_requests = 1;
        if (index == 0) {
            if (city_military_total_legions() <= 0) {
                return CITY_REQUEST_STATUS_NO_LEGIONS_AVAILABLE;
            } else if (city_military_empire_service_legions() <= 0) {
                return CITY_REQUEST_STATUS_NO_LEGIONS_SELECTED;
            } else {
                return CITY_REQUEST_STATUS_CONFIRM_SEND_LEGIONS;
            }
        }
    }
    const scenario_request *request = scenario_request_get_visible(index - num_requests);
    if (request) {
        if (request->resource == RESOURCE_DENARII) {
            if (city_finance_treasury() <= request->amount) {
                return CITY_REQUEST_STATUS_NOT_ENOUGH_RESOURCES;
            }
        } else {
            int using_granaries;
            int amount = city_resource_get_amount_including_granaries(request->resource,
                request->amount, &using_granaries);
            if (amount < request->amount) {
                return CITY_REQUEST_STATUS_NOT_ENOUGH_RESOURCES;
            } else if (using_granaries) {
                return CITY_REQUEST_STATUS_RESOURCES_FROM_GRANARY | (request->id + CITY_REQUEST_STATUS_MAX);
            }
        }
        return request->id + CITY_REQUEST_STATUS_MAX;
    }
    return 0;
}
