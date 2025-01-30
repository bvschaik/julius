#include "mission.h"

#include "core/array.h"

#define MISSIONS_ARRAY_SIZE_STEP 8
#define SCENARIOS_ARRAY_SIZE_STEP 16

static struct {
    array(campaign_mission) missions;
    array(campaign_scenario) scenarios;
} data;

static void new_mission(campaign_mission *mission, unsigned int index)
{
    mission->id = index;
    mission->first_scenario = data.scenarios.size;
    mission->last_scenario = mission->first_scenario - 1;
}

static void new_scenario(campaign_scenario *scenario, unsigned int index)
{
    scenario->id = index;
}

campaign_mission *campaign_mission_new(void)
{
    return array_advance(data.missions);
}

campaign_mission *campaign_mission_current(int index)
{
    campaign_mission *mission;
    array_foreach(data.missions, mission) {
        if (mission->first_scenario <= index && mission->last_scenario >= index) {
            return mission;
        }
    }
    return 0;
}

campaign_mission *campaign_mission_next(int last_index)
{
    campaign_mission *mission;
    array_foreach(data.missions, mission) {
        if (mission->first_scenario > last_index) {
            return mission;
        }
    }
    return 0;
}

campaign_scenario *campaign_mission_new_scenario(void)
{
    return array_advance(data.scenarios);
}

campaign_scenario *campaign_mission_get_scenario(int scenario_id)
{
    return scenario_id >= 0 && scenario_id < data.scenarios.size ? array_item(data.scenarios, scenario_id) : 0;
}

int campaign_mission_init(void)
{
    campaign_mission_clear();
    return array_init(data.missions, MISSIONS_ARRAY_SIZE_STEP, new_mission, 0) &&
        array_init(data.scenarios, SCENARIOS_ARRAY_SIZE_STEP, new_scenario, 0);
}

void campaign_mission_clear(void)
{
    if (game_campaign_is_custom()) {
        campaign_mission *mission;
        array_foreach(data.missions, mission) {
            free((uint8_t *) mission->title);
            free((char *) mission->background_image.path);
            free((char *) mission->intro_video);
        }
    }
    array_clear(data.missions);
    campaign_scenario *scenario;
    array_foreach(data.scenarios, scenario) {
        free((uint8_t *) scenario->name);
        if (game_campaign_is_custom()) {
            free((uint8_t *) scenario->description);
            free((char *) scenario->fanfare);
            free((char *) scenario->path);
        }
    }
    array_clear(data.scenarios);
}
