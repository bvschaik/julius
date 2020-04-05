#ifndef CORE_CALC_H
#define CORE_CALC_H

#include "core/direction.h"

#include <stdint.h>

/**
 * @file
 * Calculation functions.
 */

/**
 * Adjusts the value with the percentage
 * @param value Value to adjust
 * @param percentage Percentage, example: 80
 * @return value * percentage / 100
 */
int calc_adjust_with_percentage(int value, int percentage);

/**
 * Calculates which percentage 'value' of 'total' is
 * @param value Value
 * @param total Total amount
 * @return 100 * value / total
 */
int calc_percentage(int value, int total);

/**
 * Gets total distance, dx + dy
 * @return dx + dy
 */
int calc_total_distance(int x1, int y1, int x2, int y2);

/**
 * Gets maximum distance
 * @return max(dx, dy)
 */
int calc_maximum_distance(int x1, int y1, int x2, int y2);

/**
 * Gets minimum distance
 * @return min(dx, dy)
 */
int calc_minimum_distance(int x1, int y1, int x2, int y2);

/**
 * Gets maximum distance with penalty applied
 * @return max(dx, dy) + difference between dist_to_entry
 */
int calc_distance_with_penalty(int x1, int y1, int x2, int y2,
                               int dist_to_entry1, int dist_to_entry2);

/**
 * Gets the general direction from 'from' to 'to'
 * @param x_from Source X
 * @param y_from Source Y
 * @param x_to Destination X
 * @param y_to Destination Y
 * @return Direction
 */
direction_type calc_general_direction(int x_from, int y_from, int x_to, int y_to);

/**
 * Gets the direction for a missile shooter
 * @param x_from Source X
 * @param y_from Source Y
 * @param x_to Destination X
 * @param y_to Destination Y
 * @return Direction
 */
direction_type calc_missile_shooter_direction(int x_from, int y_from, int x_to, int y_to);

/**
 * Gets the direction for a missile
 * @param x_from Source X
 * @param y_from Source Y
 * @param x_to Destination X
 * @param y_to Destination Y
 * @return Direction, number between 0 and 15
 */
int calc_missile_direction(int x_from, int y_from, int x_to, int y_to);

/**
 * Make sure value is between min and max (inclusive)
 * @param value Value to bound
 * @param min Min value (inclusive)
 * @param max Max value (inclusive)
 * @return Value bounded to be between min and max
 */
int32_t calc_bound(int32_t value, int32_t min, int32_t max);

/**
 * Absolutely increments a value by "step" until it reaches "max"
 * If "max" is negative, "step" will *always* decrease "value",
 * meaning if "step" is positive, it will be converted to negative.
 * @param value Value to increment
 * @param step Amout to increment
 * @param max The incrementation limit
 * @return Value incremented by step
 */
int calc_absolute_increment(int value, int step, int max);

/**
 * Absolutely decrements a value by "step" until it reaches 0
 * If "value" is positive, "step" will *always* decrease "value",
 * meaning if "step" is positive, it will be converted to negative.
 * @param value Value to decrement
 * @param step Amout to decrement
 * @return Value decremented by step
 */
int calc_absolute_decrement(int value, int step);

#endif // CORE_CALC_H
