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

class Time
{
public:
    int year;
    unsigned int month;
    unsigned int day;
    unsigned int hour;
    unsigned int minutes;
    unsigned int seconds;

    static Time current();
};

#endif // CORE_TIME_H
