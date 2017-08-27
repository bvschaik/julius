#ifndef CORE_CALC_H
#define CORE_CALC_H

#include <stdint.h>

typedef enum {
    DIR_0_TOP,
    DIR_1_TOP_RIGHT,
    DIR_2_RIGHT,
    DIR_3_BOTTOM_RIGHT,
    DIR_4_BOTTOM,
    DIR_5_BOTTOM_LEFT,
    DIR_6_LEFT,
    DIR_7_TOP_LEFT,
    DIR_8_NONE,
} direction;

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
 * Gets the general direction from 'from' to 'to'
 * @param x_from Source X
 * @param y_from Source Y
 * @param x_to Destination X
 * @param y_to Destination Y
 * @return Direction
 */
direction calc_general_direction(int x_from, int y_from, int x_to, int y_to);

/**
 * Make sure value is between min and max (inclusive)
 * @param value Value to bound
 * @param min Min value (inclusive)
 * @param max Max value (inclusive)
 * @return Value bounded to be between min and max
 */
int32_t calc_bound(int32_t value, int32_t min, int32_t max);

#endif // CORE_CALC_H
