#include "empire.h"

#include "city/message.h"
#include "empire/city.h"
#include "game/time.h"
#include "scenario/data.h"

int scenario_empire_id(void)
{
    return scenario.empire.id;
}

int scenario_empire_is_expanded(void)
{
    return scenario.empire.is_expanded;
}

void scenario_empire_process_expansion(void)
{
    if (scenario.empire.is_expanded || scenario.empire.expansion_year <= 0) {
        return;
    }
    if (game_time_year() <= scenario.empire.expansion_year + scenario.start_year) {
        return;
    }

    empire_city_expand_empire();

    scenario.empire.is_expanded = 1;
    city_message_post(1, MESSAGE_EMPIRE_HAS_EXPANDED, 0, 0);
}
