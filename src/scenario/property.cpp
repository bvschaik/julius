#include "property.h"

#include "data/scenario.hpp"

scenario_climate scenario_property_climate()
{
    return Data_Scenario.climate;
}

int scenario_property_rome_supplies_wheat()
{
    return Data_Scenario.romeSuppliesWheat;
}
