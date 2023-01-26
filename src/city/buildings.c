#include "buildings.h"

#include "city/data_private.h"
#include "core/calc.h"

#define NUM_PLAGUE_BUILDINGS sizeof(PLAGUE_BUILDINGS) / sizeof(building_type)

static const building_type PLAGUE_BUILDINGS[] = { BUILDING_DOCK, BUILDING_WAREHOUSE, BUILDING_GRANARY };

static const building DUMMY_BUILDING;

static const building *get_first_working_building(building_type type)
{
    for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
        if (b->state == BUILDING_STATE_IN_USE || b->state == BUILDING_STATE_CREATED) {
            return b;
        }
    }
    return &DUMMY_BUILDING;
}

int city_buildings_has_senate(void)
{
    const building *senate = get_first_working_building(BUILDING_SENATE);
    if (!senate->id) {
        senate = get_first_working_building(BUILDING_SENATE_UPGRADED);
    }
    return senate->id != 0;
}

int city_buildings_has_governor_house(void)
{
    return get_first_working_building(BUILDING_GOVERNORS_HOUSE)->id != 0 ||
        get_first_working_building(BUILDING_GOVERNORS_VILLA)->id != 0 ||
        get_first_working_building(BUILDING_GOVERNORS_PALACE)->id != 0;
}

int city_buildings_has_barracks(void)
{
    return city_buildings_get_barracks() != 0;
}

int city_buildings_get_barracks(void)
{
    return get_first_working_building(BUILDING_BARRACKS)->id;
}

int city_buildings_has_mess_hall(void)
{
    return city_buildings_get_mess_hall() != 0;
}

int city_buildings_has_city_mint(void)
{
    return get_first_working_building(BUILDING_CITY_MINT)->id != 0;
}

int city_buildings_get_mess_hall(void)
{
    return get_first_working_building(BUILDING_MESS_HALL)->id;
}

int city_buildings_has_hippodrome(void)
{
    return get_first_working_building(BUILDING_HIPPODROME)->id != 0;
}

int city_buildings_has_lighthouse(void)
{
    return get_first_working_building(BUILDING_LIGHTHOUSE)->id != 0;
}

int city_buildings_has_caravanserai(void)
{
    return city_buildings_get_caravanserai() != 0;
}

int city_buildings_get_caravanserai(void)
{
    return get_first_working_building(BUILDING_CARAVANSERAI)->id;
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

int city_buildings_has_working_dock(void)
{
    for (const building *dock = building_first_of_type(BUILDING_DOCK); dock; dock = dock->next_of_type) {
        if (building_is_active(dock)) {
            return 1;
        }
    }
    return 0;
}

void city_buildings_main_native_meeting_center(int *x, int *y)
{
    const building *native_meeting = get_first_working_building(BUILDING_NATIVE_MEETING);
    *x = native_meeting->x;
    *y = native_meeting->y;
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
