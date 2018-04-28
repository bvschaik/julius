#include "mission.h"

#include "city/data_private.h"

void city_mission_reset_save_start()
{
    city_data.mission.start_saved_game_written = 0;
}

int city_mission_should_save_start()
{
    if (!city_data.mission.start_saved_game_written) {
        city_data.mission.start_saved_game_written = 1;
        return 1;
    } else {
        return 0;
    }
}

void city_mission_tutorial_set_fire_message_shown(int shown)
{
    city_data.mission.tutorial_fire_message_shown = shown;
}

void city_mission_tutorial_set_disease_message_shown(int shown)
{
    city_data.mission.tutorial_disease_message_shown = shown;
}

int city_mission_tutorial_show_disease_message()
{
    if (!city_data.mission.tutorial_disease_message_shown) {
        city_data.mission.tutorial_disease_message_shown = 1;
        return 1;
    } else {
        return 0;
    }
}

void city_mission_tutorial_add_senate()
{
    city_data.mission.tutorial_senate_built++;
}

int city_mission_tutorial_has_senate()
{
    return city_data.mission.tutorial_senate_built > 0;
}
