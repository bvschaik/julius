#ifndef MAP_WATER_SUPPLY_H
#define MAP_WATER_SUPPLY_H

void map_water_supply_update_houses();
void map_water_supply_update_reservoir_fountain();

enum {
    WELL_NECESSARY = 0,
    WELL_UNNECESSARY_FOUNTAIN = 1,
    WELL_UNNECESSARY_NO_HOUSES = 2
};

int map_water_supply_is_well_unnecessary(int well_id, int radius);

#endif // MAP_WATER_SUPPLY_H
