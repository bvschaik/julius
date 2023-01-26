#include "count.h"

#include "building/building.h"
#include "building/monument.h"
#include "city/buildings.h"
#include "city/health.h"
#include "figure/figure.h"

int building_count_grand_temples(void)
{
    return building_count_total(BUILDING_GRAND_TEMPLE_CERES) +
        building_count_total(BUILDING_GRAND_TEMPLE_NEPTUNE) +
        building_count_total(BUILDING_GRAND_TEMPLE_MERCURY) +
        building_count_total(BUILDING_GRAND_TEMPLE_MARS) +
        building_count_total(BUILDING_GRAND_TEMPLE_VENUS) +
        building_count_total(BUILDING_PANTHEON);
}

int building_count_grand_temples_active(void)
{
    return building_count_active(BUILDING_GRAND_TEMPLE_CERES) +
        building_count_active(BUILDING_GRAND_TEMPLE_NEPTUNE) +
        building_count_active(BUILDING_GRAND_TEMPLE_MERCURY) +
        building_count_active(BUILDING_GRAND_TEMPLE_MARS) +
        building_count_active(BUILDING_GRAND_TEMPLE_VENUS) +
        building_count_active(BUILDING_PANTHEON);
}

int building_count_active(building_type type)
{
    int active = 0;
    for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
        if (building_is_active(b) && b == building_main(b)) {
            active++;
        }
    }
    return active;
}

int building_count_total(building_type type)
{
    int total = 0;
    for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
        if ((b->state == BUILDING_STATE_IN_USE || b->state == BUILDING_STATE_CREATED) && b == building_main(b)) {
            total++;
        }
    }
    return total;
}

int building_count_upgraded(building_type type)
{
    int upgraded = 0;
    for (building *b = building_first_of_type(type); b; b = b->next_of_type) {
        if ((b->state == BUILDING_STATE_IN_USE || b->state == BUILDING_STATE_CREATED) && b->upgrade_level > 0 && b == building_main(b)) {
            upgraded++;
        }
    }
    return upgraded;
}
