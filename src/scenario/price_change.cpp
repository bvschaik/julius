#include "price_change.h"

#include "city/message.h"
#include "core/random.h"
#include "empire/trade_prices.h"
#include "game/time.h"

#include <data>
#include <scenario>

void scenario_price_change_init()
{
    for (int i = 0; i < MAX_PRICE_CHANGES; i++)
    {
        random_generate_next();
        if (Data_Scenario.priceChanges.year[i])
        {
            Data_Scenario.priceChanges.month[i] = (random_byte() & 7) + 2;
        }
    }
}

void scenario_price_change_process()
{
    for (int i = 0; i < MAX_PRICE_CHANGES; i++)
    {
        if (!Data_Scenario.priceChanges.year[i])
        {
            continue;
        }
        if (game_time_year() != Data_Scenario.priceChanges.year[i] + scenario_property_start_year() ||
                game_time_month() != Data_Scenario.priceChanges.month[i])
        {
            continue;
        }
        int amount = Data_Scenario.priceChanges.amount[i];
        int resource = Data_Scenario.priceChanges.resourceId[i];
        if (Data_Scenario.priceChanges.isRise[i])
        {
            if (trade_price_change(resource, amount))
            {
                city_message_post(1, MESSAGE_PRICE_INCREASED, amount, resource);
            }
        }
        else
        {
            if (trade_price_change(resource, -amount))
            {
                city_message_post(1, MESSAGE_PRICE_DECREASED, amount, resource);
            }
        }
    }
}
