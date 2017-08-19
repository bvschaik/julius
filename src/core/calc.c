#include "core/calc.h"

int calc_adjust_with_percentage(int value, int percentage)
{
    return percentage * value / 100;
}

int calc_percentage(int value, int total)
{
    if (total) {
        int value_times_100 = 100 * value;
        return value_times_100 / total;
    } else {
        return 0;
    }
}

static int get_delta(int value1, int value2)
{
    if (value1 <= value2) {
        return value2 - value1;
    } else {
        return value1 - value2;
    }
}

int calc_total_distance(int x1, int y1, int x2, int y2)
{
    int distance_x = get_delta(x1, x2);
    int distance_y = get_delta(y1, y2);
    return distance_x + distance_y;
}

int calc_maximum_distance(int x1, int y1, int x2, int y2)
{
    int distance_x = get_delta(x1, x2);
    int distance_y = get_delta(y1, y2);
    if (distance_x >= distance_y) {
        return distance_x;
    } else {
        return distance_y;
    }
}

int calc_minimum_distance(int x1, int y1, int x2, int y2)
{
    int distance_x = get_delta(x1, x2);
    int distance_y = get_delta(y1, y2);
    if (distance_x <= distance_y) {
        return distance_x;
    } else {
        return distance_y;
    }
}

int32_t calc_bound(int32_t value, int32_t min, int32_t max)
{
    if (value < min) {
        return min;
    } else if (value > max) {
        return max;
    } else {
        return value;
    }
}

