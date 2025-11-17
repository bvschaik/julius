#ifndef GYMNASIUM_ACTION_H
#define GYMNASIUM_ACTION_H

#include <stdint.h>

/**
 * Action types for Gymnasium integration
 * These represent high-level actions that can be taken in the game
 */
typedef enum {
    // Administrative actions
    ACTION_NONE = 0,                // No action / wait
    ACTION_ADJUST_TAX_UP,           // Increase tax rate by 1%
    ACTION_ADJUST_TAX_DOWN,         // Decrease tax rate by 1%
    ACTION_ADJUST_WAGES_UP,         // Increase wages
    ACTION_ADJUST_WAGES_DOWN,       // Decrease wages

    // Building construction (at specified coordinates)
    ACTION_BUILD_HOUSING,           // Build small tent (upgrades automatically)
    ACTION_BUILD_ROAD,              // Build road
    ACTION_BUILD_PLAZA,             // Build plaza
    ACTION_BUILD_GARDEN,            // Build garden
    ACTION_BUILD_STATUE,            // Build statue
    ACTION_BUILD_PREFECTURE,        // Build prefecture (fire/crime)
    ACTION_BUILD_ENGINEERS_POST,    // Build engineer's post
    ACTION_BUILD_DOCTOR,            // Build doctor
    ACTION_BUILD_HOSPITAL,          // Build hospital
    ACTION_BUILD_BATHHOUSE,         // Build bathhouse
    ACTION_BUILD_BARBER,            // Build barber
    ACTION_BUILD_SCHOOL,            // Build school
    ACTION_BUILD_ACADEMY,           // Build academy
    ACTION_BUILD_LIBRARY,           // Build library
    ACTION_BUILD_THEATER,           // Build theater
    ACTION_BUILD_AMPHITHEATER,      // Build amphitheater
    ACTION_BUILD_COLOSSEUM,         // Build colosseum
    ACTION_BUILD_HIPPODROME,        // Build hippodrome
    ACTION_BUILD_TEMPLE_SMALL,      // Build small temple (Ceres)
    ACTION_BUILD_TEMPLE_LARGE,      // Build large temple (Ceres)
    ACTION_BUILD_ORACLE,            // Build oracle
    ACTION_BUILD_MARKET,            // Build market
    ACTION_BUILD_GRANARY,           // Build granary
    ACTION_BUILD_WAREHOUSE,         // Build warehouse
    ACTION_BUILD_FOUNTAIN,          // Build fountain
    ACTION_BUILD_WELL,              // Build well
    ACTION_BUILD_FORUM,             // Build forum
    ACTION_BUILD_SENATE,            // Build senate
    ACTION_BUILD_GOVERNORS_VILLA,   // Build governor's villa
    ACTION_BUILD_GOVERNORS_PALACE,  // Build governor's palace

    // Farms
    ACTION_BUILD_WHEAT_FARM,        // Build wheat farm
    ACTION_BUILD_VEGETABLE_FARM,    // Build vegetable farm
    ACTION_BUILD_FRUIT_FARM,        // Build fruit farm
    ACTION_BUILD_OLIVE_FARM,        // Build olive farm
    ACTION_BUILD_VINES_FARM,        // Build vines farm
    ACTION_BUILD_PIG_FARM,          // Build pig farm

    // Raw material production
    ACTION_BUILD_CLAY_PIT,          // Build clay pit
    ACTION_BUILD_TIMBER_YARD,       // Build timber yard
    ACTION_BUILD_IRON_MINE,         // Build iron mine
    ACTION_BUILD_MARBLE_QUARRY,     // Build marble quarry

    // Workshops
    ACTION_BUILD_WINE_WORKSHOP,     // Build wine workshop
    ACTION_BUILD_OIL_WORKSHOP,      // Build oil workshop
    ACTION_BUILD_WEAPONS_WORKSHOP,  // Build weapons workshop
    ACTION_BUILD_FURNITURE_WORKSHOP,// Build furniture workshop
    ACTION_BUILD_POTTERY_WORKSHOP,  // Build pottery workshop

    // Entertainment training
    ACTION_BUILD_ACTOR_COLONY,      // Build actor colony
    ACTION_BUILD_GLADIATOR_SCHOOL,  // Build gladiator school
    ACTION_BUILD_LION_HOUSE,        // Build lion house
    ACTION_BUILD_CHARIOT_MAKER,     // Build chariot maker

    // Destruction
    ACTION_CLEAR_LAND,              // Clear/demolish at specified coordinates

    ACTION_COUNT                    // Total number of actions
} gymnasium_action_type_t;

/**
 * Action parameter structure
 * Specifies where and what to build/demolish
 */
typedef struct {
    gymnasium_action_type_t type;   // Action type
    int32_t x;                      // X coordinate (grid position)
    int32_t y;                      // Y coordinate (grid position)
    int32_t param1;                 // Optional parameter (e.g., building size, god type)
    int32_t param2;                 // Optional parameter (reserved for future use)
} gymnasium_action_t;

/**
 * Action result structure
 * Indicates success/failure and provides feedback
 */
typedef struct {
    int32_t success;                // 1 if action succeeded, 0 if failed
    int32_t reason;                 // Reason code for failure (0 = no error)
    char message[256];              // Human-readable message
} gymnasium_action_result_t;

// Reason codes for action failures
typedef enum {
    ACTION_ERROR_NONE = 0,
    ACTION_ERROR_INVALID_TYPE,      // Unknown action type
    ACTION_ERROR_INVALID_COORDS,    // Coordinates out of bounds
    ACTION_ERROR_BLOCKED,           // Location blocked/occupied
    ACTION_ERROR_NO_MONEY,          // Insufficient funds
    ACTION_ERROR_NO_LABOR,          // Insufficient labor
    ACTION_ERROR_NO_RESOURCES,      // Missing required resources
    ACTION_ERROR_TERRAIN,           // Terrain not suitable
    ACTION_ERROR_WATER,             // Location is water
    ACTION_ERROR_ROCK,              // Location is rock
    ACTION_ERROR_TREE,              // Location has trees
    ACTION_ERROR_BUILDING,          // Building already exists
    ACTION_ERROR_ROAD_REQUIRED,     // Road access required
    ACTION_ERROR_NOT_IMPLEMENTED,   // Action not yet implemented
} gymnasium_action_error_t;

/**
 * Execute an action in the game
 * @param action Pointer to action structure
 * @param result Pointer to result structure (filled on return)
 * @return 0 on success, non-zero on error
 */
int gymnasium_execute_action(const gymnasium_action_t *action, gymnasium_action_result_t *result);

/**
 * Validate an action without executing it
 * Checks if the action would be legal to execute
 * @param action Pointer to action structure
 * @param result Pointer to result structure (filled on return)
 * @return 1 if valid, 0 if invalid
 */
int gymnasium_validate_action(const gymnasium_action_t *action, gymnasium_action_result_t *result);

/**
 * Get the name of an action type
 * @param type Action type
 * @return String name of the action
 */
const char *gymnasium_action_name(gymnasium_action_type_t type);

#endif // GYMNASIUM_ACTION_H
