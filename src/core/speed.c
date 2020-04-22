#include "speed.h"

#include <math.h>

#define FRAME_TIME 16.67

void speed_clear(speed *sp)
{
    sp->cumulative_delta = 0;
    sp->desired_speed = 0;
    sp->current_speed = 0;
    sp->speed_difference = 0;
    sp->start_time = 0;
    sp->total_time = 0;
    sp->last_speed_check = time_get_millis();
}

void speed_change(speed *sp, int new_speed, time_millis total_time)
{
    if (new_speed == sp->desired_speed) {
        return;
    }
    if (total_time == SPEED_CHANGE_IMMEDIATE) {
        speed_clear(sp);
        sp->desired_speed = new_speed;
        sp->current_speed = new_speed;
        return;
    }
    sp->cumulative_delta = 0;
    sp->speed_difference = sp->current_speed - new_speed;
    sp->desired_speed = new_speed;
    sp->start_time = time_get_millis();
    sp->total_time = total_time;
}

void speed_invert(speed *sp)
{
    speed_change(sp, -sp->desired_speed, SPEED_CHANGE_IMMEDIATE);
}

speed_direction speed_get_current_direction(const speed *sp)
{
    if (!sp->current_speed) {
        return SPEED_DIRECTION_STOPPED;
    }
    return (sp->current_speed > 0) ? SPEED_DIRECTION_POSITIVE : SPEED_DIRECTION_NEGATIVE;
}

static int adjust_speed_for_elapsed_time(int delta, time_millis last_time)
{
    return lround((delta / FRAME_TIME) * (time_get_millis() - last_time));
}

static int adjust_speed_for_frame_time(int delta, time_millis last_time)
{
    return lround((delta / (double) (time_get_millis() - last_time)) * FRAME_TIME);
}

int speed_get_delta(speed *sp)
{
    int delta;
    time_millis elapsed = time_get_millis() - sp->start_time;
    int desired = adjust_speed_for_elapsed_time(sp->desired_speed, sp->last_speed_check);
    if (sp->current_speed == sp->desired_speed || elapsed > sp->total_time * 5) {
        delta = (sp->total_time == SPEED_CHANGE_IMMEDIATE) ? sp->desired_speed : desired;
        sp->current_speed = sp->desired_speed;
    } else {
        double speed = sp->speed_difference * (sp->total_time / FRAME_TIME);
        double exponent = exp(-((int) elapsed) / (double) sp->total_time);
        delta = (int) (speed - speed * exponent - sp->cumulative_delta);
        sp->cumulative_delta += delta;
        delta = desired + delta;
        sp->current_speed = adjust_speed_for_frame_time(delta, sp->last_speed_check);
    }
    sp->last_speed_check = time_get_millis();
    return delta;
}
