#ifndef BUILDING_INDUSTRY_H
#define BUILDING_INDUSTRY_H

#include "building/building.h"
#include "map/point.h"

#define BUILDING_INDUSTRY_CITY_MINT_GOLD_PER_COIN 20

int building_is_farm(building_type type);
int building_is_raw_resource_producer(building_type type);
int building_is_workshop(building_type type);
/** Returns 0-100 representing building efficiency, or -1 if building type does
 * not have efficiency. Wharfs are calculated against an arbitrary benchmark. */
int building_get_efficiency(const building *b);

void building_industry_update_production(int new_day);

int building_industry_get_max_progress(const building *b);

int building_stockpiling_enabled(building *b);
int building_industry_has_produced_resource(building *b);
void building_industry_start_new_production(building *b);

void building_bless_farms(void);
void building_curse_farms(int big_curse);
void building_bless_industry(void);

void building_workshop_add_raw_material(building *b, int resource);

int building_get_workshop_for_raw_material(int x, int y, int resource, int road_network_id, map_point *dst);
int building_has_workshop_for_raw_material_with_room(int resource, int road_network_id);
int building_get_workshop_for_raw_material_with_room(int x, int y, int resource, int road_network_id, map_point *dst);

void building_industry_advance_stats(void);
void building_industry_start_strikes(void);

#endif // BUILDING_INDUSTRY_H
