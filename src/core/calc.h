#ifndef CORE_CALC_H
#define CORE_CALC_H

#include "core/direction.h"

#include <stdint.h>

/**
 * @file
 * Calculation functions.
 */


/**
 * Calculates the number of digits a value has
 * @param value Value to calculate the number of digits
 * @return The number of digits, including the minus sign if negative
 */
int calc_digits_in_number(int value);

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
 * Calculates the next multiple of step in which value will fit
 * @param value The value to fit in the multiple of step
 * @param step The step to use as a multiple
 * @return The multiple of step into which value fits
 */
int calc_value_in_step(int value, int step);

/**
 * Gets maximum distance
 * @return max(dx, dy)
 */
int calc_maximum_distance(int x1, int y1, int x2, int y2);

/**
 * Gets the distance between two boxes
 */
int calc_box_distance(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh);

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
double calc_bound_double(double value, double min, double max);

/**
 * Moves a value toward 0 by step
 * @param value Value to move toward zero
 * @param step Pointer to the value to set as a step. step is changed if it is smaller than value
 * @return Value decremented by step or 0 if the absolute value is smaller than the absolute step
 */
int calc_absolute_decrement(int value, int *step);


#endif // CORE_CALC_H
