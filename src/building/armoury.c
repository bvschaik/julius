#include "armoury.h"

#include "building/barracks.h"
#include "building/count.h"
#include "building/distribution.h"
#include "city/resource.h"
#include "map/point.h"

int building_armoury_is_needed(building *armoury)
{
    if (!building_count_active(BUILDING_BARRACKS) && !building_count_active(BUILDING_GRAND_TEMPLE_MARS)) {
        return 0;
    }

    if (city_resource_is_stockpiled(RESOURCE_WEAPONS) ||
        !building_get_barracks_for_weapon(armoury->x, armoury->y, RESOURCE_WEAPONS, armoury->road_network_id, 0)) {
        return 0;
    }

    resource_storage_info info[RESOURCE_MAX] = { 0 };
    info[RESOURCE_WEAPONS].needed = 1;

    return building_distribution_get_resource_storages_for_building(info, armoury, 10000);
}
