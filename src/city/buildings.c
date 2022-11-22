#include "buildings.h"

#include "city/data_private.h"
#include "core/calc.h"

#define NUM_PLAGUE_BUILDINGS sizeof(PLAGUE_BUILDINGS) / sizeof(building_type)

static const building_type PLAGUE_BUILDINGS[] = { BUILDING_DOCK, BUILDING_WAREHOUSE, BUILDING_GRANARY };

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

int city_buildings_has_barracks(void)
{
    return city_data.building.barracks_placed;
}

void city_buildings_add_barracks(building *barracks)
{
    city_data.building.barracks_placed = 1;
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

int city_buildings_has_mess_hall(void)
{
    return city_data.building.mess_hall_building_id > 0;
}

void city_buildings_add_mess_hall(building *mess_hall)
{
    city_data.mess_hall.missing_mess_hall_warning_shown = 0;
    city_data.building.mess_hall_building_id = mess_hall->id;
}

void city_buildings_remove_mess_hall(void)
{
    city_data.building.mess_hall_building_id = 0;
}

int city_buildings_get_mess_hall(void)
{
    return city_data.building.mess_hall_building_id;
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

int city_buildings_has_caravanserai(void)
{
    return city_data.building.caravanserai_building_id > 0;
}

void city_buildings_add_caravanserai(building *caravanserai)
{
    city_data.building.caravanserai_building_id = caravanserai->id;
}

void city_buildings_remove_caravanserai(void)
{
    city_data.building.caravanserai_building_id = 0;
}

int city_buildings_get_caravanserai(void)
{
    return city_data.building.caravanserai_building_id;
}

int city_buildings_triumphal_arch_available(void)
{
    return city_data.building.triumphal_arches_available > city_data.building.triumphal_arches_placed;
}

void city_buildings_build_triumphal_arch(void)
{
    city_data.building.triumphal_arches_placed++;
}

void city_buildings_remove_triumphal_arch(void)
{
    if (city_data.building.triumphal_arches_placed > 0) {
        city_data.building.triumphal_arches_placed--;
    }
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

int city_buildings_get_closest_plague(int x, int y, int *distance)
{
    int min_free_building_id = 0;
    int min_occupied_building_id = 0;
    int min_occupied_dist = *distance = 10000;

    // Find closest in houses
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->has_plague && b->distance_from_entry) {
                int dist = calc_maximum_distance(x, y, b->x, b->y);
                if (b->figure_id4) {
                    if (dist < min_occupied_dist) {
                        min_occupied_dist = dist;
                        min_occupied_building_id = b->id;
                    }
                } else if (dist < *distance) {
                    *distance = dist;
                    min_free_building_id = b->id;
                }
            }
        }
    }

    // Find closest in buildings (docks, granaries or warehouses)
    for (int i = 0 ; i < NUM_PLAGUE_BUILDINGS; i++) {
        building_type type = PLAGUE_BUILDINGS[i];
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            if (b->has_plague && b->distance_from_entry) {
                int dist = calc_maximum_distance(x, y, b->x, b->y);
                if (b->figure_id4) {
                    if (dist < min_occupied_dist) {
                        min_occupied_dist = dist;
                        min_occupied_building_id = b->id;
                    }
                } else if (dist < *distance) {
                    *distance = dist;
                    min_free_building_id = b->id;
                }
            }
        }
    }

    if (!min_free_building_id && min_occupied_dist <= 2) {
        min_free_building_id = min_occupied_building_id;
        *distance = 2;
    }
    return min_free_building_id;
}

static void update_sickness_duration(int building_id)
{
    building *b = building_get(building_id);

    if (b->has_plague) {
        // Stop plague after time or if doctor heals it
        if (b->sickness_duration == 99) {
            b->sickness_duration = 0;
            b->has_plague = 0;
            b->sickness_level = 0;
            b->sickness_doctor_cure = 0;
            b->figure_id4 = 0;
            b->fumigation_frame = 0;
            b->fumigation_direction = 0;
        } else {
            b->sickness_duration += 1;
        }
    }
}

void city_buildings_update_plague(void)
{
    for (building_type type = BUILDING_HOUSE_SMALL_TENT; type <= BUILDING_HOUSE_LUXURY_PALACE; type++) {
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            update_sickness_duration(b->id);
        }
    }

    for (int i = 0 ; i < NUM_PLAGUE_BUILDINGS; i++) {
        building_type type = PLAGUE_BUILDINGS[i];
        for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
            update_sickness_duration(b->id);
        }
    }
}
