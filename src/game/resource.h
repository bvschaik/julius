#ifndef GAME_RESOURCE_H
#define GAME_RESOURCE_H

/**
 * @file
 * Type definitions for resources
 */

/**
 * Resource types
 */
typedef enum {
    RESOURCE_NONE = 0,
    RESOURCE_WHEAT = 1,
    RESOURCE_VEGETABLES = 2,
    RESOURCE_FRUIT = 3,
    RESOURCE_OLIVES = 4,
    RESOURCE_VINES = 5,
    RESOURCE_MEAT = 6,
    RESOURCE_WINE = 7,
    RESOURCE_OIL = 8,
    RESOURCE_IRON = 9,
    RESOURCE_TIMBER = 10,
    RESOURCE_CLAY = 11,
    RESOURCE_MARBLE = 12,
    RESOURCE_WEAPONS = 13,
    RESOURCE_FURNITURE = 14,
    RESOURCE_POTTERY = 15,
    RESOURCE_DENARII = 16,
    RESOURCE_TROOPS = 17,
    // helper constants
    RESOURCE_MIN = 1,
    RESOURCE_MAX = 16,
    RESOURCE_MIN_FOOD = 1,
    RESOURCE_MAX_FOOD = 7
} resource_type;

#endif // GAME_RESOURCE_H
