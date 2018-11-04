#include "buildings.h"

#include "city/data_private.h"

int city_buildings_has_senate(void)
{
    return city_data.building.senate_placed;
}

void city_buildings_add_senate(building *senate)
{
    city_data.building.senate_placed = 1;
    if (!city_data.building.senate_grid_offset) {
        city_data.building.senate_building_id = senate->id;
        city_data.building.senate_x = senate->x;
        city_data.building.senate_y = senate->y;
        city_data.building.senate_grid_offset = senate->grid_offset;
    }
}

void city_buildings_remove_senate(building *senate)
{
    if (senate->grid_offset == city_data.building.senate_grid_offset) {
        city_data.building.senate_grid_offset = 0;
        city_data.building.senate_x = 0;
        city_data.building.senate_y = 0;
        city_data.building.senate_placed = 0;
    }
}

void city_buildings_add_barracks(building *barracks)
{
    if (!city_data.building.barracks_grid_offset) {
        city_data.building.barracks_building_id = barracks->id;
        city_data.building.barracks_x = barracks->x;
        city_data.building.barracks_y = barracks->y;
        city_data.building.barracks_grid_offset = barracks->grid_offset;
    }
}

void city_buildings_remove_barracks(building *barracks)
{
    if (barracks->grid_offset == city_data.building.barracks_grid_offset) {
        city_data.building.barracks_grid_offset = 0;
        city_data.building.barracks_x = 0;
        city_data.building.barracks_y = 0;
        city_data.building.barracks_placed = 0;
    }
}

int city_buildings_get_barracks(void)
{
    return city_data.building.barracks_building_id;
}

void city_buildings_set_barracks(int building_id)
{
    city_data.building.barracks_building_id = building_id;
}

int city_buildings_has_distribution_center(void)
{
    return city_data.building.distribution_center_placed;
}

void city_buildings_add_distribution_center(building *center)
{
    city_data.building.distribution_center_placed = 1;
    if (!city_data.building.distribution_center_grid_offset) {
        city_data.building.distribution_center_building_id = center->id;
        city_data.building.distribution_center_x = center->x;
        city_data.building.distribution_center_y = center->y;
        city_data.building.distribution_center_grid_offset = center->grid_offset;
    }
}

void city_buildings_remove_distribution_center(building *center)
{
    if (center->grid_offset == city_data.building.distribution_center_grid_offset) {
        city_data.building.distribution_center_grid_offset = 0;
        city_data.building.distribution_center_x = 0;
        city_data.building.distribution_center_y = 0;
        city_data.building.distribution_center_placed = 0;
    }
}

int city_buildings_get_trade_center(void)
{
    return city_data.building.trade_center_building_id;
}

void city_buildings_set_trade_center(int building_id)
{
    city_data.building.trade_center_building_id = building_id;
}

int city_buildings_has_hippodrome(void)
{
    return city_data.building.hippodrome_placed;
}

void city_buildings_add_hippodrome(void)
{
    city_data.building.hippodrome_placed = 1;
}

void city_buildings_remove_hippodrome(void)
{
    city_data.building.hippodrome_placed = 0;
}

int city_buildings_triumphal_arch_available(void)
{
    return city_data.building.triumphal_arches_available > city_data.building.triumphal_arches_placed;
}

void city_buildings_build_triumphal_arch(void)
{
    city_data.building.triumphal_arches_placed++;
}

void city_buildings_earn_triumphal_arch(void)
{
    city_data.building.triumphal_arches_available++;
}

void city_buildings_add_dock(void)
{
    city_data.building.working_docks++;
}

void city_buildings_remove_dock(void)
{
    city_data.building.working_docks--;
}

void city_buildings_reset_dock_wharf_counters(void)
{
    city_data.building.working_wharfs = 0;
    city_data.building.shipyard_boats_requested = 0;
    for (int i = 0; i < 8; i++) {
        city_data.building.working_dock_ids[i] = 0;
    }
    city_data.building.working_docks = 0;
}

void city_buildings_add_working_wharf(int needs_fishing_boat)
{
    ++city_data.building.working_wharfs;
    if (needs_fishing_boat) {
        ++city_data.building.shipyard_boats_requested;
    }
}

void city_buildings_add_working_dock(int building_id)
{
    if (city_data.building.working_docks < 10) {
        city_data.building.working_dock_ids[city_data.building.working_docks] = building_id;
    }
    ++city_data.building.working_docks;
}

int city_buildings_shipyard_boats_requested(void)
{
    return city_data.building.shipyard_boats_requested;
}

int city_buildings_has_working_dock(void)
{
    return city_data.building.working_docks > 0;
}

int city_buildings_get_working_dock(int index)
{
    return city_data.building.working_dock_ids[index];
}

void city_buildings_main_native_meeting_center(int *x, int *y)
{
    *x = city_data.building.main_native_meeting.x;
    *y = city_data.building.main_native_meeting.y;
}

void city_buildings_set_main_native_meeting_center(int x, int y)
{
    city_data.building.main_native_meeting.x = x;
    city_data.building.main_native_meeting.y = y;
}

int city_buildings_is_mission_post_operational(void)
{
    return city_data.building.mission_post_operational > 0;
}

void city_buildings_set_mission_post_operational(void)
{
    city_data.building.mission_post_operational = 1;
}

int city_buildings_unknown_value(void)
{
    return city_data.building.unknown_value;
}
