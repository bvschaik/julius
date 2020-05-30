#ifndef CORE_SPEED_H
#define CORE_SPEED_H

#include "core/time.h"

/**
 * @file
 * Scroll speed functions.
 * Calculates deltas for constant speed/acceleration/deceleration based on
 * a "number of pixels per unit of time" value.
 */

/**
 * Constant for immediate speed change with no acceleration/deceleration
 */
#define SPEED_CHANGE_IMMEDIATE 0

/**
* Speed direction constants
*/
typedef enum {
    SPEED_DIRECTION_NEGATIVE = -1,
    SPEED_DIRECTION_STOPPED  = 0,
    SPEED_DIRECTION_POSITIVE = 1
} speed_direction;

typedef struct {
    time_millis start_time;
    time_millis total_time;
    time_millis last_speed_check;
    double speed_difference;
    double desired_speed;
    double current_speed;
    double adjusted_current_speed;
    double cumulative_delta;
    double fine_position;
    int adjust_for_time;
} speed_type;

/**
 * Clears speed structure
 * @param speed Speed structure to clear
 */
void speed_clear(speed_type *speed);

/**
 * Sets the new target speed
 * @param speed Speed structure to act on
 * @param new_speed The new speed
 * @param total_time the time for the acceleration/deceleration, or SPEED_CHANGE_IMMEDIATE
 * @param adjust_for_time If set to "1", in speed_get_delta, the delta value will be
 *                        adjusted to keep the same rate regardless of FPS
 *                        If set to "0", speed_get_delta will return the brute value
 *                        without taking into account the elapsed time
 */
void speed_set_target(speed_type *speed, double new_speed, time_millis total_time, int adjust_for_time);

/**
 * Immediately invert the speed (positive speed becomes negative and vice-versa)
 * @param sp Speed structure to act on
 */
void speed_invert(speed_type *speed);

/**
 * Gets the delta, in pixels, to move since the last time speed_get_delta was called
 * If speed_should_adjust_for_elapsed_time is enabled, the delta value is adjust_for_time to keep
 * the same rate regardless of FPS
 * Please note: this function should only be called once per frame
 * @param speed Speed structure to act on
 * @return The delta movement
 */
int speed_get_delta(speed_type *speed);

/**
 * Gets the current speed direction
 * @param speed Speed structure to act on
 * @return An enum with the current direction
 */
speed_direction speed_get_current_direction(const speed_type *speed);

/**
 * Indicates whether the speed is accelerating/decelerating
 * @param speed Speed structure to act on
 * @return Whether the speed is changing
 */
int speed_is_changing(const speed_type *speed);

#endif // CORE_SPEED_H
