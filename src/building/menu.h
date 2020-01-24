#ifndef BUILDING_MENU_H
#define BUILDING_MENU_H

#include "building/type.h"

typedef enum {
    BUILDING_MENU_VACANT_HOUSES = 0,
    BUILDING_MENU_CLEAR_LAND = 1,
    BUILDING_MENU_ROAD = 2,
    BUILDING_MENU_WATER_STRUCTURES = 3,
    BUILDING_MENU_HEALTH = 4,
    BUILDING_MENU_TEMPLES = 5,
    BUILDING_MENU_EDUCATION = 6,
    BUILDING_MENU_ENTERTAINMENT = 7,
    BUILDING_MENU_ADMINISTRATION = 8,
    BUILDING_MENU_ENGINEERING = 9,
    BUILDING_MENU_SECURITY = 10,
    BUILDING_MENU_INDUSTRIAL = 11,
    BUILDING_MENU_ALL_HOUSES = 12,
    BUILDING_MENU_BRUSH_SIZE = 13,
    BUILDING_MENU_ENTRY_EXIT = 14,
    BUILDING_MENU_ELEVATION = 15,
    BUILDING_MENU_INVASION_POINTS = 16,
    BUILDING_MENU_RIVER_POINTS = 17,
    BUILDING_MENU_NATIVE = 18,
    BUILDING_MENU_ALL_FARMS = 19,
    BUILDING_MENU_ALL_RAW_MATERIALS = 20,
    BUILDING_MENU_ALL_WORKSHOPS = 21,
    BUILDING_MENU_ALL_SMALL_TEMPLES = 22,
    BUILDING_MENU_ALL_LARGE_TEMPLES = 23,
    BUILDING_MENU_ALL_FORTS = 24,
    BUILDING_MENU_HERDING_POINTS = 25,
    BUILDING_MENU_UNUSED_1 = 26,
    BUILDING_MENU_UNUSED_2 = 27,
    BUILDING_MENU_UNUSED_3 = 28,
    BUILDING_MENU_UNUSED_4 = 29,
    BUILDING_MENU_MAX = 30
} building_menu_group;

void building_menu_enable_all(void);

void building_menu_update(void);

int building_menu_count_items(int submenu);

int building_menu_next_index(int submenu, int current_index);

building_type building_menu_type(int submenu, int item);

/**
 * Checks whether the building menu has changed.
 * Also marks the change as 'seen'.
 * @return True if the building menu has changed
 */
int building_menu_has_changed(void);

#endif // BUILDING_MENU_H
