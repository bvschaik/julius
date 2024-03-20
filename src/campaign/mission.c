#include "mission.h"

#include "core/array.h"

#define MISSIONS_ARRAY_SIZE_STEP 8
#define OPTIONS_ARRAY_SIZE_STEP 2

static struct {
    array(campaign_mission) missions;
    array(campaign_mission_option) options;
} data;

static void new_mission(campaign_mission *mission, int index)
{
    mission->id = index;
    mission->first_option = data.options.size;
    mission->last_option = mission->first_option - 1;
}

static void new_option(campaign_mission_option *option, int index)
{
    option->id = index;
}

campaign_mission *campaign_mission_new(void)
{
    campaign_mission *mission;
    array_new_item(data.missions, data.missions.size, mission);
    return mission;
}

campaign_mission *campaign_mission_next(int last_index)
{
    campaign_mission *mission;
    array_foreach(data.missions, mission) {
        if (mission->first_option > last_index) {
            return mission;
        }
    }
    return 0;
}

campaign_mission_option *campaign_mission_new_option(void)
{
    campaign_mission_option *option;
    array_new_item(data.options, data.options.size, option);
    return option;
}

campaign_mission_option *campaign_mission_get_option(int option_id)
{
    return option_id >= 0 && option_id < data.options.size ? array_item(data.options, option_id) : 0;
}

int campaign_mission_init(void)
{
    campaign_mission_clear();
    return array_init(data.missions, MISSIONS_ARRAY_SIZE_STEP, new_mission, 0) &&
        array_init(data.options, OPTIONS_ARRAY_SIZE_STEP, new_option, 0);
}

void campaign_mission_clear(void)
{
    campaign_mission *mission;
    array_foreach(data.missions, mission) {
        free((char *) mission->background_image);
    }
    array_clear(data.missions);
    campaign_mission_option *option;
    array_foreach(data.options, option) {
        free((char *) option->name);
        free((char *) option->description);
        free((char *) option->path);
        free((char *) option->image.path);
    }
    array_clear(data.options);
}
