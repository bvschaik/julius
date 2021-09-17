#ifndef BUILDING_CLONE_H
#define BUILDING_CLONE_H

#include "building/type.h"

/**
 * Get the building type for cloning at the specified offset
 * @param grid_offset Grid offset to clone building from
 * @return Building type to use for cloning the building
 */
building_type building_clone_type_from_grid_offset(int grid_offset);

#endif
