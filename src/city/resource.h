#ifndef CITY_RESOURCE_H
#define CITY_RESOURCE_H

#include "city/constants.h"
#include "game/resource.h"

#define FOOD_PER_SOLDIER_MONTHLY 4
#define FOOD_PER_TRADER_MONTHLY 10
#define RESOURCE_ONE_LOAD 100

typedef struct {
    int size;
    resource_type items[RESOURCE_MAX];
} resource_list;

int city_resource_count_food_on_granaries(resource_type food);
int city_resource_count(resource_type resource);
int city_resource_get_amount_including_granaries(resource_type resource, int amount, int *checked_granaries);

const resource_list *city_resource_get_available(void);

const resource_list *city_resource_get_available_foods(void);

const resource_list *city_resource_get_potential(void);

const resource_list *city_resource_get_potential_foods(void);

int city_resource_multiple_wine_available(void);

int city_resource_food_types_available(void);
int city_resource_food_stored(void);
int city_resource_food_needed(void);
int city_resource_food_supply_months(void);
int city_resource_food_percentage_produced(void);

int city_resource_operating_granaries(void);

int city_resource_last_used_warehouse(void);
void city_resource_set_last_used_warehouse(int warehouse_id);

resource_trade_status city_resource_trade_status(resource_type resource);
void city_resource_cycle_trade_status(resource_type resource, resource_trade_status status);

int city_resource_import_over(resource_type resource);
void city_resource_change_import_over(resource_type resource, int change);

int city_resource_export_over(resource_type resource);
void city_resource_change_export_over(resource_type resource, int change);

int city_resource_is_stockpiled(resource_type resource);
void city_resource_toggle_stockpiled(resource_type resource);

int city_resource_is_mothballed(resource_type resource);
void city_resource_toggle_mothballed(resource_type resource);

void city_resource_add_produced_to_granary(int amount);
void city_resource_add_to_granary(resource_type food, int amount);
void city_resource_remove_from_granary(resource_type food, int amount);

void city_resource_add_to_warehouse(resource_type resource, int amount);
void city_resource_remove_from_warehouse(resource_type resource, int amount);
void city_resource_calculate_warehouse_stocks(void);

void city_resource_determine_available(void);
resource_type city_resource_ceres_temple_food(void);

void city_resource_calculate_food_stocks_and_supply_wheat(void);

void city_resource_consume_food(void);

#endif // CITY_RESOURCE_H
