#ifndef BUILDING_MENU_H
#define BUILDING_MENU_H

#include "building/type.h"

void building_menu_enable_all();

void building_menu_update();

int building_menu_count_items(int submenu);

int building_menu_next_index(int submenu, int current_index);

building_type building_menu_type(int submenu, int item);

/**
 * Checks whether the building menu has changed.
 * Also marks the change as 'seen'.
 * @return True if the building menu has changed
 */
int building_menu_has_changed();

#endif // BUILDING_MENU_H
