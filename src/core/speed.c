#include "speed.h"

#include <math.h>

#define FRAME_TIME 16.67

void speed_clear(speed_type *speed)
{
    speed->cumulative_delta = 0;
    speed->desired_speed = 0;
    speed->current_speed = 0;
    speed->speed_difference = 0;
    speed->start_time = 0;
    speed->total_time = 0;
    speed->last_speed_check = time_get_millis();
}

void speed_set_target(speed_type *speed, int new_speed, time_millis total_time)
{
    if (new_speed == speed->desired_speed) {
        return;
    }
    if (total_time == SPEED_CHANGE_IMMEDIATE) {
        speed_clear(speed);
        speed->desired_speed = new_speed;
        speed->current_speed = new_speed;
        return;
    }
    speed->cumulative_delta = 0;
    speed->speed_difference = speed->current_speed - new_speed;
    speed->desired_speed = new_speed;
    speed->start_time = time_get_millis();
    speed->total_time = total_time;
}

void speed_invert(speed_type *speed)
{
    speed_set_target(speed, -speed->desired_speed, SPEED_CHANGE_IMMEDIATE);
}

speed_direction speed_get_current_direction(const speed_type *speed)
{
    if (!speed->current_speed) {
        return SPEED_DIRECTION_STOPPED;
    }
    return (speed->current_speed > 0) ? SPEED_DIRECTION_POSITIVE : SPEED_DIRECTION_NEGATIVE;
}

static double adjust_speed_for_elapsed_time(int delta, time_millis last_time)
{
    return round((delta / FRAME_TIME) * (time_get_millis() - last_time));
}

static int adjust_speed_for_frame_time(double delta, time_millis last_time)
{
    return lround((delta / (double) (time_get_millis() - last_time)) * FRAME_TIME);
}

int speed_get_delta(speed_type *speed)
{
    double delta;
    time_millis elapsed = time_get_millis() - speed->start_time;
    double desired = adjust_speed_for_elapsed_time(speed->desired_speed, speed->last_speed_check);
    if (speed->current_speed == speed->desired_speed || elapsed > speed->total_time * 4) {
        delta = (speed->total_time == SPEED_CHANGE_IMMEDIATE) ? speed->desired_speed : desired;
        speed->current_speed = speed->desired_speed;
    } else {
        if (elapsed == 0) {
            delta = adjust_speed_for_elapsed_time(speed->current_speed, speed->last_speed_check);
        } else {
            double normalized_speed = speed->speed_difference * (speed->total_time / FRAME_TIME);
            double exponent = exp(-((int) elapsed) / (double) speed->total_time);
            delta = normalized_speed - normalized_speed * exponent - speed->cumulative_delta;
            speed->cumulative_delta += (int) delta;
            delta = desired + delta;
            speed->current_speed = adjust_speed_for_frame_time(delta, speed->last_speed_check);
        }
    }
    speed->last_speed_check = time_get_millis();
    return (int) delta;
}
