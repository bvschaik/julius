#ifndef BUILDING_HOUSE_EVOLUTION_H
#define BUILDING_HOUSE_EVOLUTION_H

#include "building/building.h"

/**
 * Evolves/devolves houses if appropriate, and consumes pottery/furniture/oil/wine
 */
void building_house_process_evolve_and_consume_goods();

/**
 * Determine the text to show for evolution of a house, stored in house->evolveTextId
 * @param house House to determine text for
 * @param worst_desirability_building The ID of the building with worst contribution to desirability
 */
void building_house_determine_evolve_text(building *house, int worst_desirability_building);

#endif // BUILDING_HOUSE_EVOLUTION_H
