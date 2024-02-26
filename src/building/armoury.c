#include "armoury.h"

#include "building/barracks.h"
#include "building/count.h"
#include "building/warehouse.h"
#include "city/resource.h"
#include "map/point.h"

int building_armory_is_needed(building *armoury)
{
    map_point dst;

    if ((building_count_active(BUILDING_BARRACKS) || building_count_active(BUILDING_GRAND_TEMPLE_MARS)) &&
    !city_resource_is_stockpiled(RESOURCE_WEAPONS)) {
        int barracks_id = building_get_barracks_for_weapon(armoury->x, armoury->y, RESOURCE_WEAPONS, armoury->road_network_id, 0);

        if (!barracks_id) {
            return 0;
        }

        return building_warehouse_with_resource(armoury->x, armoury->y, RESOURCE_WEAPONS, armoury->road_network_id, 0, &dst);
    }

    return 0;
}
