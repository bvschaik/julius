#ifndef BUILDING_MODEL_H
#define BUILDING_MODEL_H

#include "building/type.h"
#include "city/resource.h"

/**
 * @file
 * Building models.
 */

/**
 * Building model
 */
typedef struct {
    int cost; /**< Cost of structure or of one tile of a structure (for walls) */
    int desirability_value; /**< Initial desirability value */
    int desirability_step; /**< Desirability step (in tiles) */
    int desirability_step_size; /**< Desirability step size */
    int desirability_range; /**< Max desirability range */
    int laborers; /**< Number of people a building employs */
} model_building;

/**
 * House model
 */
typedef struct {
    int devolve_desirability; /**< Desirability at which the house devolves */
    int evolve_desirability; /**< Desirability at which the house evolves */
    int entertainment; /**< Entertainment points required */
    int water; /**< Water required: 1 = well, 2 = fountain */
    int religion; /**< Number of gods required */
    int education; /**< Education required:
        1 = school or library, 2 = school and library, 3 = school, library and academy */
    int barber; /**< Barber required (boolean) */
    int bathhouse; /**< Bathhouse required (boolean) */
    int health; /**< Health required: 1 = doctor or hospital, 2 = doctor and hospital */
    int food_types; /**< Number of food types required */
    int pottery; /**< Pottery required */
    int oil; /**< Oil required */
    int furniture; /**< Furniture required */
    int wine; /**< Wine types required: 1 = any wine, 2 = two types of wine */
    int prosperity; /**< Prosperity contribution */
    int max_people; /**< Maximum people per tile (medium insula and lower) or per house (large insula and up) */
    int tax_multiplier; /**< Tax rate multiplier */
} model_house;

/**
 * Loads the model from file
 * @return boolean true if model was loaded successfully, false otherwise
 */
int model_load(void);

/**
 * Gets the model for a building
 * @param type Building type
 * @return Read-only model
 */
const model_building *model_get_building(building_type type);

/**
 * Gets the model for a house
 * @param level House level
 * @return Read-only model
 */
const model_house *model_get_house(house_level level);

int model_house_uses_inventory(house_level level, resource_type inventory);

#endif // BUILDING_MODEL_H
