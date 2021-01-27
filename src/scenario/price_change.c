#include "price_change.h"

#include "city/message.h"
#include "core/random.h"
#include "empire/trade_prices.h"
#include "game/time.h"
#include "scenario/data.h"

void scenario_price_change_init(void)
{
    for (int i = 0; i < MAX_PRICE_CHANGES; i++) {
        random_generate_next();
        if (scenario.price_changes[i].year) {
            scenario.price_changes[i].month = (random_byte() & 7) + 2;
        }
    }
}

void scenario_price_change_process(void)
{
    for (int i = 0; i < MAX_PRICE_CHANGES; i++) {
        if (!scenario.price_changes[i].year) {
            continue;
        }
        if (game_time_year() != scenario.price_changes[i].year + scenario.start_year ||
            game_time_month() != scenario.price_changes[i].month) {
            continue;
        }
        int amount = scenario.price_changes[i].amount;
        int resource = scenario.price_changes[i].resource;
        if (scenario.price_changes[i].is_rise) {
            if (trade_price_change(resource, amount)) {
                city_message_post(1, MESSAGE_PRICE_INCREASED, amount, resource);
            }
        } else {
            if (trade_price_change(resource, -amount)) {
                city_message_post(1, MESSAGE_PRICE_DECREASED, amount, resource);
            }
        }
    }
}
