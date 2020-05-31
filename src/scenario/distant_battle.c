#include "distant_battle.h"

#include "building/menu.h"
#include "city/buildings.h"
#include "city/map.h"
#include "city/message.h"
#include "city/military.h"
#include "city/ratings.h"
#include "core/calc.h"
#include "empire/city.h"
#include "empire/object.h"
#include "empire/type.h"
#include "figure/formation_legion.h"
#include "game/time.h"
#include "scenario/data.h"

int scenario_distant_battle_roman_travel_months(void)
{
    return scenario.empire.distant_battle_roman_travel_months;
}

int scenario_distant_battle_enemy_travel_months(void)
{
    return scenario.empire.distant_battle_enemy_travel_months;
}

void scenario_distant_battle_set_roman_travel_months(void)
{
    scenario.empire.distant_battle_roman_travel_months =
        empire_object_init_distant_battle_travel_months(EMPIRE_OBJECT_ROMAN_ARMY);
}

void scenario_distant_battle_set_enemy_travel_months(void)
{
    scenario.empire.distant_battle_enemy_travel_months =
        empire_object_init_distant_battle_travel_months(EMPIRE_OBJECT_ENEMY_ARMY);
}

void scenario_distant_battle_process(void)
{
    for (int i = 0; i < MAX_INVASIONS; i++) {
        if (scenario.invasions[i].type == INVASION_TYPE_DISTANT_BATTLE &&
            game_time_year() == scenario.invasions[i].year + scenario.start_year &&
            game_time_month() == scenario.invasions[i].month &&
            scenario.empire.distant_battle_enemy_travel_months > 4 &&
            scenario.empire.distant_battle_roman_travel_months > 4 &&
            !city_military_has_distant_battle()) {

            city_message_post(1, MESSAGE_CAESAR_REQUESTS_ARMY, 0, 0);
            city_military_init_distant_battle(scenario.invasions[i].amount);
            return;
        }
    }

    city_military_process_distant_battle();
}
