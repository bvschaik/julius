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
    int speed_difference;
    int desired_speed;
    int current_speed;
    int cumulative_delta;
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
 */
void speed_set_target(speed_type *speed, int new_speed, time_millis total_time);

/**
 * Immediately invert the speed (positive speed becomes negative and vice-versa)
 * @param sp Speed structure to act on
 */
void speed_invert(speed_type *speed);

/**
 * Gets the delta, in pixels, to move since the last time speed_get_delta was called
 * The delta value is normalized to keep the same rate regardless of FPS, EXCEPT if
 * SPEED_CHANGE_IMMEDIATE was passed in speed_set_target
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

#endif // CORE_SPEED_H
