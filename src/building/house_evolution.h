#ifndef BUILDING_HOUSE_EVOLUTION_H
#define BUILDING_HOUSE_EVOLUTION_H

#include "building/building.h"

/**
 * Evolves/devolves a house if appropriate
 * @param house House to process
 * @return True if the house has expanded, false otherwise
 */
int building_house_process_evolve(building *house);

/**
 * Determine the text to show for evolution of a house, stored in house->evolveTextId
 * @param house House to determine text for
 * @param worst_desirability_building The ID of the building with worst contribution to desirability
 */
void building_house_determine_evolve_text(building *house, int worst_desirability_building);

#endif // BUILDING_HOUSE_EVOLUTION_H
