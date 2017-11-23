#include "empire.h"

#include "city/message.h"
#include "empire/city.h"
#include "game/time.h"

#include "data/scenario.hpp"

int scenario_empire_id()
{
    return Data_Scenario.empireId;
}

int scenario_empire_is_expanded()
{
    return Data_Scenario.empireHasExpanded;
}

void scenario_empire_process_expansion()
{
    if (Data_Scenario.empireHasExpanded || Data_Scenario.empireExpansionYear <= 0)
    {
        return;
    }
    if (game_time_year() < Data_Scenario.empireExpansionYear + Data_Scenario.startYear)
    {
        return;
    }

    empire_city_expand_empire();

    Data_Scenario.empireHasExpanded = 1;
    city_message_post(1, MESSAGE_EMPIRE_HAS_EXPANDED, 0, 0);
}
