#include "action.h"

#include "building/construction_building.h"
#include "building/construction_clear.h"
#include "building/type.h"
#include "city/finance.h"
#include "city/labor.h"
#include "city/warning.h"
#include "map/grid.h"

#include <stdio.h>
#include <string.h>

// Map gymnasium action types to building types
static building_type action_to_building_type(gymnasium_action_type_t action)
{
    switch (action) {
        case ACTION_BUILD_HOUSING: return BUILDING_HOUSE_VACANT_LOT;
        case ACTION_BUILD_ROAD: return BUILDING_ROAD;
        case ACTION_BUILD_PLAZA: return BUILDING_PLAZA;
        case ACTION_BUILD_GARDEN: return BUILDING_GARDENS;
        case ACTION_BUILD_STATUE: return BUILDING_SMALL_STATUE;
        case ACTION_BUILD_PREFECTURE: return BUILDING_PREFECTURE;
        case ACTION_BUILD_ENGINEERS_POST: return BUILDING_ENGINEERS_POST;
        case ACTION_BUILD_DOCTOR: return BUILDING_DOCTOR;
        case ACTION_BUILD_HOSPITAL: return BUILDING_HOSPITAL;
        case ACTION_BUILD_BATHHOUSE: return BUILDING_BATHHOUSE;
        case ACTION_BUILD_BARBER: return BUILDING_BARBER;
        case ACTION_BUILD_SCHOOL: return BUILDING_SCHOOL;
        case ACTION_BUILD_ACADEMY: return BUILDING_ACADEMY;
        case ACTION_BUILD_LIBRARY: return BUILDING_LIBRARY;
        case ACTION_BUILD_THEATER: return BUILDING_THEATER;
        case ACTION_BUILD_AMPHITHEATER: return BUILDING_AMPHITHEATER;
        case ACTION_BUILD_COLOSSEUM: return BUILDING_COLOSSEUM;
        case ACTION_BUILD_HIPPODROME: return BUILDING_HIPPODROME;
        case ACTION_BUILD_TEMPLE_SMALL: return BUILDING_SMALL_TEMPLE_CERES;
        case ACTION_BUILD_TEMPLE_LARGE: return BUILDING_LARGE_TEMPLE_CERES;
        case ACTION_BUILD_ORACLE: return BUILDING_ORACLE;
        case ACTION_BUILD_MARKET: return BUILDING_MARKET;
        case ACTION_BUILD_GRANARY: return BUILDING_GRANARY;
        case ACTION_BUILD_WAREHOUSE: return BUILDING_WAREHOUSE;
        case ACTION_BUILD_FOUNTAIN: return BUILDING_FOUNTAIN;
        case ACTION_BUILD_WELL: return BUILDING_WELL;
        case ACTION_BUILD_FORUM: return BUILDING_FORUM;
        case ACTION_BUILD_SENATE: return BUILDING_SENATE;
        case ACTION_BUILD_GOVERNORS_VILLA: return BUILDING_GOVERNORS_VILLA;
        case ACTION_BUILD_GOVERNORS_PALACE: return BUILDING_GOVERNORS_PALACE;
        case ACTION_BUILD_WHEAT_FARM: return BUILDING_WHEAT_FARM;
        case ACTION_BUILD_VEGETABLE_FARM: return BUILDING_VEGETABLE_FARM;
        case ACTION_BUILD_FRUIT_FARM: return BUILDING_FRUIT_FARM;
        case ACTION_BUILD_OLIVE_FARM: return BUILDING_OLIVE_FARM;
        case ACTION_BUILD_VINES_FARM: return BUILDING_VINES_FARM;
        case ACTION_BUILD_PIG_FARM: return BUILDING_PIG_FARM;
        case ACTION_BUILD_CLAY_PIT: return BUILDING_CLAY_PIT;
        case ACTION_BUILD_TIMBER_YARD: return BUILDING_TIMBER_YARD;
        case ACTION_BUILD_IRON_MINE: return BUILDING_IRON_MINE;
        case ACTION_BUILD_MARBLE_QUARRY: return BUILDING_MARBLE_QUARRY;
        case ACTION_BUILD_WINE_WORKSHOP: return BUILDING_WINE_WORKSHOP;
        case ACTION_BUILD_OIL_WORKSHOP: return BUILDING_OIL_WORKSHOP;
        case ACTION_BUILD_WEAPONS_WORKSHOP: return BUILDING_WEAPONS_WORKSHOP;
        case ACTION_BUILD_FURNITURE_WORKSHOP: return BUILDING_FURNITURE_WORKSHOP;
        case ACTION_BUILD_POTTERY_WORKSHOP: return BUILDING_POTTERY_WORKSHOP;
        case ACTION_BUILD_ACTOR_COLONY: return BUILDING_ACTOR_COLONY;
        case ACTION_BUILD_GLADIATOR_SCHOOL: return BUILDING_GLADIATOR_SCHOOL;
        case ACTION_BUILD_LION_HOUSE: return BUILDING_LION_HOUSE;
        case ACTION_BUILD_CHARIOT_MAKER: return BUILDING_CHARIOT_MAKER;
        default: return BUILDING_NONE;
    }
}

const char *gymnasium_action_name(gymnasium_action_type_t type)
{
    switch (type) {
        case ACTION_NONE: return "NONE";
        case ACTION_ADJUST_TAX_UP: return "ADJUST_TAX_UP";
        case ACTION_ADJUST_TAX_DOWN: return "ADJUST_TAX_DOWN";
        case ACTION_ADJUST_WAGES_UP: return "ADJUST_WAGES_UP";
        case ACTION_ADJUST_WAGES_DOWN: return "ADJUST_WAGES_DOWN";
        case ACTION_BUILD_HOUSING: return "BUILD_HOUSING";
        case ACTION_BUILD_ROAD: return "BUILD_ROAD";
        case ACTION_BUILD_PLAZA: return "BUILD_PLAZA";
        case ACTION_BUILD_GARDEN: return "BUILD_GARDEN";
        case ACTION_BUILD_STATUE: return "BUILD_STATUE";
        case ACTION_BUILD_PREFECTURE: return "BUILD_PREFECTURE";
        case ACTION_BUILD_ENGINEERS_POST: return "BUILD_ENGINEERS_POST";
        case ACTION_BUILD_DOCTOR: return "BUILD_DOCTOR";
        case ACTION_BUILD_HOSPITAL: return "BUILD_HOSPITAL";
        case ACTION_BUILD_BATHHOUSE: return "BUILD_BATHHOUSE";
        case ACTION_BUILD_BARBER: return "BUILD_BARBER";
        case ACTION_BUILD_SCHOOL: return "BUILD_SCHOOL";
        case ACTION_BUILD_ACADEMY: return "BUILD_ACADEMY";
        case ACTION_BUILD_LIBRARY: return "BUILD_LIBRARY";
        case ACTION_BUILD_THEATER: return "BUILD_THEATER";
        case ACTION_BUILD_AMPHITHEATER: return "BUILD_AMPHITHEATER";
        case ACTION_BUILD_COLOSSEUM: return "BUILD_COLOSSEUM";
        case ACTION_BUILD_HIPPODROME: return "BUILD_HIPPODROME";
        case ACTION_BUILD_TEMPLE_SMALL: return "BUILD_TEMPLE_SMALL";
        case ACTION_BUILD_TEMPLE_LARGE: return "BUILD_TEMPLE_LARGE";
        case ACTION_BUILD_ORACLE: return "BUILD_ORACLE";
        case ACTION_BUILD_MARKET: return "BUILD_MARKET";
        case ACTION_BUILD_GRANARY: return "BUILD_GRANARY";
        case ACTION_BUILD_WAREHOUSE: return "BUILD_WAREHOUSE";
        case ACTION_BUILD_FOUNTAIN: return "BUILD_FOUNTAIN";
        case ACTION_BUILD_WELL: return "BUILD_WELL";
        case ACTION_BUILD_FORUM: return "BUILD_FORUM";
        case ACTION_BUILD_SENATE: return "BUILD_SENATE";
        case ACTION_BUILD_GOVERNORS_VILLA: return "BUILD_GOVERNORS_VILLA";
        case ACTION_BUILD_GOVERNORS_PALACE: return "BUILD_GOVERNORS_PALACE";
        case ACTION_BUILD_WHEAT_FARM: return "BUILD_WHEAT_FARM";
        case ACTION_BUILD_VEGETABLE_FARM: return "BUILD_VEGETABLE_FARM";
        case ACTION_BUILD_FRUIT_FARM: return "BUILD_FRUIT_FARM";
        case ACTION_BUILD_OLIVE_FARM: return "BUILD_OLIVE_FARM";
        case ACTION_BUILD_VINES_FARM: return "BUILD_VINES_FARM";
        case ACTION_BUILD_PIG_FARM: return "BUILD_PIG_FARM";
        case ACTION_BUILD_CLAY_PIT: return "BUILD_CLAY_PIT";
        case ACTION_BUILD_TIMBER_YARD: return "BUILD_TIMBER_YARD";
        case ACTION_BUILD_IRON_MINE: return "BUILD_IRON_MINE";
        case ACTION_BUILD_MARBLE_QUARRY: return "BUILD_MARBLE_QUARRY";
        case ACTION_BUILD_WINE_WORKSHOP: return "BUILD_WINE_WORKSHOP";
        case ACTION_BUILD_OIL_WORKSHOP: return "BUILD_OIL_WORKSHOP";
        case ACTION_BUILD_WEAPONS_WORKSHOP: return "BUILD_WEAPONS_WORKSHOP";
        case ACTION_BUILD_FURNITURE_WORKSHOP: return "BUILD_FURNITURE_WORKSHOP";
        case ACTION_BUILD_POTTERY_WORKSHOP: return "BUILD_POTTERY_WORKSHOP";
        case ACTION_BUILD_ACTOR_COLONY: return "BUILD_ACTOR_COLONY";
        case ACTION_BUILD_GLADIATOR_SCHOOL: return "BUILD_GLADIATOR_SCHOOL";
        case ACTION_BUILD_LION_HOUSE: return "BUILD_LION_HOUSE";
        case ACTION_BUILD_CHARIOT_MAKER: return "BUILD_CHARIOT_MAKER";
        case ACTION_CLEAR_LAND: return "CLEAR_LAND";
        default: return "UNKNOWN";
    }
}

static void set_result(gymnasium_action_result_t *result, int success,
                      gymnasium_action_error_t reason, const char *message)
{
    result->success = success;
    result->reason = reason;
    snprintf(result->message, sizeof(result->message), "%s", message);
}

static int is_administrative_action(gymnasium_action_type_t type)
{
    return type == ACTION_ADJUST_TAX_UP || type == ACTION_ADJUST_TAX_DOWN ||
           type == ACTION_ADJUST_WAGES_UP || type == ACTION_ADJUST_WAGES_DOWN;
}

static int is_building_action(gymnasium_action_type_t type)
{
    return type >= ACTION_BUILD_HOUSING && type < ACTION_CLEAR_LAND;
}

int gymnasium_validate_action(const gymnasium_action_t *action, gymnasium_action_result_t *result)
{
    if (!action || !result) {
        if (result) {
            set_result(result, 0, ACTION_ERROR_INVALID_TYPE, "NULL action or result pointer");
        }
        return 0;
    }

    // Validate action type
    if (action->type < 0 || action->type >= ACTION_COUNT) {
        set_result(result, 0, ACTION_ERROR_INVALID_TYPE, "Invalid action type");
        return 0;
    }

    // Administrative actions don't need coordinate validation
    if (is_administrative_action(action->type)) {
        set_result(result, 1, ACTION_ERROR_NONE, "Valid administrative action");
        return 1;
    }

    // NONE action is always valid
    if (action->type == ACTION_NONE) {
        set_result(result, 1, ACTION_ERROR_NONE, "Valid no-op action");
        return 1;
    }

    // Building and demolition actions need valid coordinates
    if (is_building_action(action->type) || action->type == ACTION_CLEAR_LAND) {
        // Check coordinate bounds
        int map_width = map_grid_width();
        int map_height = map_grid_height();

        // If map isn't initialized (width/height = 0), use reasonable defaults
        // Standard Caesar III map is 162x162
        if (map_width == 0) map_width = 162;
        if (map_height == 0) map_height = 162;

        if (action->x < 0 || action->y < 0 || action->x >= map_width || action->y >= map_height) {
            set_result(result, 0, ACTION_ERROR_INVALID_COORDS, "Coordinates out of bounds");
            return 0;
        }
    }

    // For building actions, check if we can map to a building type
    if (is_building_action(action->type)) {
        building_type type = action_to_building_type(action->type);
        if (type == BUILDING_NONE) {
            set_result(result, 0, ACTION_ERROR_NOT_IMPLEMENTED, "Building type not mapped");
            return 0;
        }
    }

    set_result(result, 1, ACTION_ERROR_NONE, "Valid action");
    return 1;
}

int gymnasium_execute_action(const gymnasium_action_t *action, gymnasium_action_result_t *result)
{
    if (!action || !result) {
        if (result) {
            set_result(result, 0, ACTION_ERROR_INVALID_TYPE, "NULL action or result pointer");
        }
        return -1;
    }

    // First validate the action
    if (!gymnasium_validate_action(action, result)) {
        return -1;
    }

    // Handle different action types
    switch (action->type) {
        case ACTION_NONE:
            set_result(result, 1, ACTION_ERROR_NONE, "No action taken");
            return 0;

        case ACTION_ADJUST_TAX_UP: {
            int current_tax = city_finance_tax_percentage();
            if (current_tax >= 25) {
                set_result(result, 0, ACTION_ERROR_INVALID_TYPE, "Tax already at maximum (25%)");
                return -1;
            }
            city_finance_change_tax_percentage(1);
            set_result(result, 1, ACTION_ERROR_NONE, "Tax increased by 1%");
            return 0;
        }

        case ACTION_ADJUST_TAX_DOWN: {
            int current_tax = city_finance_tax_percentage();
            if (current_tax <= 0) {
                set_result(result, 0, ACTION_ERROR_INVALID_TYPE, "Tax already at minimum (0%)");
                return -1;
            }
            city_finance_change_tax_percentage(-1);
            set_result(result, 1, ACTION_ERROR_NONE, "Tax decreased by 1%");
            return 0;
        }

        case ACTION_ADJUST_WAGES_UP: {
            int current_wages = city_labor_wages();
            if (current_wages >= 50) {
                set_result(result, 0, ACTION_ERROR_INVALID_TYPE, "Wages already at maximum");
                return -1;
            }
            city_labor_change_wages(1);
            set_result(result, 1, ACTION_ERROR_NONE, "Wages increased");
            return 0;
        }

        case ACTION_ADJUST_WAGES_DOWN: {
            int current_wages = city_labor_wages();
            if (current_wages <= 0) {
                set_result(result, 0, ACTION_ERROR_INVALID_TYPE, "Wages already at minimum");
                return -1;
            }
            city_labor_change_wages(-1);
            set_result(result, 1, ACTION_ERROR_NONE, "Wages decreased");
            return 0;
        }

        case ACTION_CLEAR_LAND: {
            // Clear a single tile (can be expanded to area later)
            int cleared = building_construction_clear_land(0, action->x, action->y, action->x, action->y);
            if (cleared > 0) {
                set_result(result, 1, ACTION_ERROR_NONE, "Land cleared");
                return 0;
            } else {
                set_result(result, 0, ACTION_ERROR_BLOCKED, "Nothing to clear at location");
                return -1;
            }
        }

        default:
            // Handle building construction
            if (is_building_action(action->type)) {
                building_type type = action_to_building_type(action->type);
                int success = building_construction_place_building(type, action->x, action->y);

                if (success) {
                    char msg[256];
                    snprintf(msg, sizeof(msg), "Built %s at (%d, %d)",
                            gymnasium_action_name(action->type), action->x, action->y);
                    set_result(result, 1, ACTION_ERROR_NONE, msg);
                    return 0;
                } else {
                    // Building construction failed - could be many reasons
                    // The game shows warnings which we can't easily access here
                    set_result(result, 0, ACTION_ERROR_BLOCKED,
                              "Building construction failed (blocked, no money, or terrain unsuitable)");
                    return -1;
                }
            }

            set_result(result, 0, ACTION_ERROR_NOT_IMPLEMENTED, "Action not yet implemented");
            return -1;
    }
}
