#ifndef CITY_MISSION_H
#define CITY_MISSION_H

void city_mission_reset_save_start();
int city_mission_should_save_start();

void city_mission_tutorial_set_fire_message_shown(int shown);
void city_mission_tutorial_set_disease_message_shown(int shown);
int city_mission_tutorial_show_disease_message();

void city_mission_tutorial_add_senate();
int city_mission_tutorial_has_senate();

#endif // CITY_MISSION_H
