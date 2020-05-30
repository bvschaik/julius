#include "speed.h"

#include <math.h>

#define FRAME_TIME 16.67

void speed_clear(speed_type *speed)
{
    speed->cumulative_delta = 0.0;
    speed->fine_position = 0.0;
    speed->desired_speed = 0.0;
    speed->current_speed = 0.0;
    speed->speed_difference = 0.0;
    speed->start_time = 0;
    speed->total_time = 0;
    speed->last_speed_check = time_get_millis();
}

static double adjust_speed_for_elapsed_time(double delta, int adjust_for_time, time_millis last_time)
{
    return adjust_for_time ? (delta / FRAME_TIME) * (time_get_millis() - last_time) : delta;
}

static double adjust_speed_for_frame_time(double delta, int adjust_for_time, time_millis last_time)
{
    return adjust_for_time ? ((delta / (double) (time_get_millis() - last_time)) * FRAME_TIME) : delta;
}

void speed_set_target(speed_type *speed, double new_speed, time_millis total_time, int adjust_for_time)
{
    speed->adjust_for_time = adjust_for_time;
    if (new_speed == speed->desired_speed) {
        return;
    }
    if (total_time == SPEED_CHANGE_IMMEDIATE) {
        speed->desired_speed = new_speed;
        speed->current_speed = new_speed;
        speed->total_time = total_time;
        if (!adjust_for_time && time_get_millis() - speed->last_speed_check > 0) {
            speed->adjusted_current_speed = adjust_speed_for_frame_time(new_speed, 1, speed->last_speed_check);
        } else {
            speed->adjusted_current_speed = new_speed;
        }
        return;
    }
    speed->cumulative_delta = 0.0;
    speed->fine_position = 0.0;
    double base_speed = adjust_for_time ? speed->adjusted_current_speed : speed->current_speed;
    speed->speed_difference = base_speed - new_speed;
    speed->desired_speed = new_speed;
    speed->start_time = time_get_millis();
    speed->total_time = total_time;
}

void speed_invert(speed_type *speed)
{
    speed_set_target(speed, -speed->current_speed, SPEED_CHANGE_IMMEDIATE, speed->adjust_for_time);
}

speed_direction speed_get_current_direction(const speed_type *speed)
{
    if (!speed->current_speed) {
        return SPEED_DIRECTION_STOPPED;
    }
    return (speed->current_speed > 0) ? SPEED_DIRECTION_POSITIVE : SPEED_DIRECTION_NEGATIVE;
}

int handle_fine_position(speed_type *speed, double delta)
{
    int delta_rounded = (int) delta;
    speed->fine_position += delta - delta_rounded;
    int extra_position = (int) speed->fine_position;
    speed->fine_position -= extra_position;
    return delta_rounded + extra_position;
}

int speed_get_delta(speed_type *speed)
{
    if (speed->adjust_for_time && speed->last_speed_check == time_get_millis()) {
        return 0;
    }
    double delta;
    time_millis elapsed = time_get_millis() - speed->start_time;
    double desired = speed->desired_speed;
    desired = adjust_speed_for_elapsed_time(speed->desired_speed, speed->adjust_for_time, speed->last_speed_check);
    if (speed->total_time == SPEED_CHANGE_IMMEDIATE) {
        delta = desired;
    } else if (speed->current_speed == speed->desired_speed || elapsed > speed->total_time * 4) {
        delta = desired;
        speed->current_speed = speed->desired_speed;
        speed->adjusted_current_speed = speed->desired_speed;
    } else {
        if (elapsed == 0) {
            delta = adjust_speed_for_elapsed_time(speed->current_speed, speed->adjust_for_time, speed->last_speed_check);
        } else {
            double full_delta = speed->speed_difference * (speed->total_time / FRAME_TIME);
            double exponent = exp(-((int) elapsed) / (double) speed->total_time);
            delta = full_delta - full_delta * exponent - speed->cumulative_delta;
            speed->cumulative_delta += delta;
            delta += desired;
            speed->current_speed = adjust_speed_for_frame_time(delta, speed->adjust_for_time, speed->last_speed_check);
            speed->adjusted_current_speed = speed->current_speed;
        }
    }
    speed->last_speed_check = time_get_millis();
    return handle_fine_position(speed, delta);
}

int speed_is_changing(const speed_type *speed)
{
    return speed->current_speed != speed->desired_speed;
}
