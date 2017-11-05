#ifndef CORE_TIME_H
#define CORE_TIME_H
/**
 * @file
 * Time tracking functions.
 */

/**
 * Time in millisecond-precision. Use only for time difference calculations.
 */
typedef unsigned int time_millis;

/**
 * Gets the current time
 * @return Current time in milliseconds
 */
time_millis time_get_millis();

/**
 * Sets the current time
 * @param millis Current milliseconds
 */
void time_set_millis(time_millis millis);

#endif // CORE_TIME_H
