#include "request.h"

#include "city/message.h"
#include "core/random.h"
#include "game/resource.h"
#include "game/time.h"
#include "game/tutorial.h"

#include <data>
#include "cityinfo.h"
#include "resource.h"

#define MAX_REQUESTS 20

void scenario_request_init()
{
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        random_generate_next();
        if (Data_Scenario.requests.resourceId[i])
        {
            Data_Scenario.requests_month[i] = (random_byte() & 7) + 2;
            Data_Scenario.requests_monthsToComply[i] = 12 * Data_Scenario.requests.deadlineYears[i];
        }
    }
}

void scenario_request_process()
{
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        if (!Data_Scenario.requests.resourceId[i] || Data_Scenario.requests_state[i] > REQUEST_STATE_DISPATCHED_LATE)
        {
            continue;
        }
        int state = Data_Scenario.requests_state[i];
        if (state == REQUEST_STATE_DISPATCHED || state == REQUEST_STATE_DISPATCHED_LATE)
        {
            --Data_Scenario.requests_monthsToComply[i];
            if (Data_Scenario.requests_monthsToComply[i] <= 0)
            {
                if (state == REQUEST_STATE_DISPATCHED)
                {
                    city_message_post(1, MESSAGE_REQUEST_RECEIVED, i, 0);
                    CityInfo_Ratings_changeFavor(Data_Scenario.requests_favor[i]);
                }
                else
                {
                    city_message_post(1, MESSAGE_REQUEST_RECEIVED_LATE, i, 0);
                    CityInfo_Ratings_changeFavor(Data_Scenario.requests_favor[i] / 2);
                }
                Data_Scenario.requests_state[i] = REQUEST_STATE_RECEIVED;
                Data_Scenario.requests_isVisible[i] = 0;
            }
        }
        else
        {
            // normal or overdue
            if (Data_Scenario.requests_isVisible[i])
            {
                --Data_Scenario.requests_monthsToComply[i];
                if (state == REQUEST_STATE_NORMAL)
                {
                    if (Data_Scenario.requests_monthsToComply[i] == 12)
                    {
                        // reminder
                        city_message_post(1, MESSAGE_REQUEST_REMINDER, i, 0);
                    }
                    else if (Data_Scenario.requests_monthsToComply[i] <= 0)
                    {
                        city_message_post(1, MESSAGE_REQUEST_REFUSED, i, 0);
                        Data_Scenario.requests_state[i] = REQUEST_STATE_OVERDUE;
                        Data_Scenario.requests_monthsToComply[i] = 24;
                        CityInfo_Ratings_changeFavor(-3);
                        Data_CityInfo.ratingFavorIgnoredRequestPenalty = 3;
                    }
                }
                else if (state == REQUEST_STATE_OVERDUE)
                {
                    if (Data_Scenario.requests_monthsToComply[i] <= 0)
                    {
                        city_message_post(1, MESSAGE_REQUEST_REFUSED_OVERDUE, i, 0);
                        Data_Scenario.requests_state[i] = REQUEST_STATE_IGNORED;
                        Data_Scenario.requests_isVisible[i] = 0;
                        CityInfo_Ratings_changeFavor(-5);
                        Data_CityInfo.ratingFavorIgnoredRequestPenalty = 5;
                    }
                }
                if (!Data_Scenario.requests_canComplyDialogShown[i] &&
                        Data_CityInfo.resourceStored[Data_Scenario.requests.resourceId[i]] >= Data_Scenario.requests.amount[i])
                {
                    Data_Scenario.requests_canComplyDialogShown[i] = 1;
                    city_message_post(1, MESSAGE_REQUEST_CAN_COMPLY, i, 0);
                }
            }
            else
            {
                // request is not visible
                int year = Data_Scenario.startYear;
                if (!Tutorial::adjust_request_year(&year))
                {
                    return;
                }
                if (game_time_year() == year + Data_Scenario.requests.year[i] &&
                        game_time_month() == Data_Scenario.requests_month[i])
                {
                    Data_Scenario.requests_isVisible[i] = 1;
                    if (Data_CityInfo.resourceStored[Data_Scenario.requests.resourceId[i]] >= Data_Scenario.requests.amount[i])
                    {
                        Data_Scenario.requests_canComplyDialogShown[i] = 1;
                    }
                    if (Data_Scenario.requests.resourceId[i] == RESOURCE_DENARII)
                    {
                        city_message_post(1, MESSAGE_CAESAR_REQUESTS_MONEY, i, 0);
                    }
                    else if (Data_Scenario.requests.resourceId[i] == RESOURCE_TROOPS)
                    {
                        city_message_post(1, MESSAGE_CAESAR_REQUESTS_ARMY, i, 0);
                    }
                    else
                    {
                        city_message_post(1, MESSAGE_CAESAR_REQUESTS_GOODS, i, 0);
                    }
                }
            }
        }
    }
}

void scenario_request_dispatch(int id)
{
    if (Data_Scenario.requests_state[id] == REQUEST_STATE_NORMAL)
    {
        Data_Scenario.requests_state[id] = REQUEST_STATE_DISPATCHED;
    }
    else
    {
        Data_Scenario.requests_state[id] = REQUEST_STATE_DISPATCHED_LATE;
    }
    Data_Scenario.requests_monthsToComply[id] = (random_byte() & 3) + 1;
    Data_Scenario.requests_isVisible[id] = 0;
    int amount = Data_Scenario.requests.amount[id];
    if (Data_Scenario.requests.resourceId[id] == RESOURCE_DENARII)
    {
        Data_CityInfo.treasury -= amount;
        Data_CityInfo.financeSundriesThisYear += amount;
    }
    else if (Data_Scenario.requests.resourceId[id] == RESOURCE_TROOPS)
    {
        CityInfo_Population_removePeopleForTroopRequest(amount);
        Resource_removeFromCityWarehouses(RESOURCE_WEAPONS, amount);
    }
    else
    {
        Resource_removeFromCityWarehouses(Data_Scenario.requests.resourceId[id], amount);
    }
}

const scenario_request *scenario_request_get(int id)
{
    static scenario_request request;
    request.id = id;
    request.amount = Data_Scenario.requests.amount[id];
    request.resource = Data_Scenario.requests.resourceId[id];
    request.state = Data_Scenario.requests_state[id];
    request.months_to_comply = Data_Scenario.requests_monthsToComply[id];
    return &request;
}

int scenario_request_foreach_visible(int start_index, void (*callback)(int index, const scenario_request *request))
{
    int index = start_index;
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        if (Data_Scenario.requests.resourceId[i] && Data_Scenario.requests_isVisible[i])
        {
            callback(index, scenario_request_get(i));
            index++;
        }
    }
    return index;
}

const scenario_request *scenario_request_get_visible(int index)
{
    for (int i = 0; i < MAX_REQUESTS; i++)
    {
        if (Data_Scenario.requests.resourceId[i] && Data_Scenario.requests_isVisible[i] &&
                Data_Scenario.requests_state[i] <= 1)
        {
            if (index == 0)
            {
                return scenario_request_get(i);
            }
            index--;
        }
    }
    return 0;
}
