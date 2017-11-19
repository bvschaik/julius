#ifndef SCENARIO_PROPERTY_H
#define SCENARIO_PROPERTY_H

typedef enum {
    CLIMATE_CENTRAL = 0,
    CLIMATE_NORTHERN = 1,
    CLIMATE_DESERT = 2
} scenario_climate;

scenario_climate scenario_property_climate();

int scenario_property_rome_supplies_wheat();

#endif // SCENARIO_PROPERTY_H
