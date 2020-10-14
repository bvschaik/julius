#ifndef WINDOW_BUILDING_CULTURE_H
#define WINDOW_BUILDING_CULTURE_H

#include "common.h"
#include "input/mouse.h"


void window_building_draw_clinic(building_info_context *c);

void window_building_draw_hospital(building_info_context *c);

void window_building_draw_bathhouse(building_info_context *c);

void window_building_draw_barber(building_info_context *c);

void window_building_draw_school(building_info_context *c);

void window_building_draw_academy(building_info_context *c);

void window_building_draw_library(building_info_context *c);

void window_building_draw_temple_ceres(building_info_context *c);

void window_building_draw_temple_neptune(building_info_context *c);

void window_building_draw_temple_mercury(building_info_context *c);

void window_building_draw_temple_mars(building_info_context *c);

void window_building_draw_temple_venus(building_info_context *c);

void window_building_draw_oracle(building_info_context *c);

void window_building_draw_theater(building_info_context *c);

void window_building_draw_amphitheater(building_info_context *c);

void window_building_draw_colosseum(building_info_context *c);

void window_building_draw_hippodrome(building_info_context *c);

void window_building_draw_actor_colony(building_info_context *c);

void window_building_draw_gladiator_school(building_info_context *c);

void window_building_draw_lion_house(building_info_context *c);

void window_building_draw_chariot_maker(building_info_context *c);

void window_building_draw_grand_temple_foreground(building_info_context* c);

void window_building_draw_grand_temple_ceres(building_info_context* c);

void window_building_draw_grand_temple_neptune(building_info_context* c);

void window_building_draw_grand_temple_mercury(building_info_context* c);

void window_building_draw_grand_temple_mars(building_info_context* c);

void window_building_draw_grand_temple_venus(building_info_context* c);

void window_building_draw_pantheon(building_info_context* c);

void window_building_draw_work_camp(building_info_context* c);

void window_building_draw_engineer_guild(building_info_context* c);

void window_building_draw_mess_hall(building_info_context* c, int mess_hall_fulfillment);

int window_building_handle_mouse_grand_temple(const mouse* m, building_info_context* c);

#endif // WINDOW_BUILDING_CULTURE_H
