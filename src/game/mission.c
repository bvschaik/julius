#include "mission.h"

#include "game/campaign.h"
#include "scenario/property.h"

int game_mission_has_choice(void)
{
    if (!game_campaign_is_active()) {
        return 0;
    }
    int mission = scenario_campaign_mission();
    const campaign_mission_info *info = game_campaign_get_current_mission(mission);
    return info && info->total_scenarios > 1;
}
