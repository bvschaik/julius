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

direction calc_general_direction(int x_from, int y_from, int x_to, int y_to)
{
    if (x_from < x_to) {
        if (y_from > y_to) {
            return DIR_1_TOP_RIGHT;
        } else if (y_from == y_to) {
            return DIR_2_RIGHT;
        } else if (y_from < y_to) {
            return DIR_3_BOTTOM_RIGHT;
        }
    } else if (x_from == x_to) {
        if (y_from > y_to) {
            return DIR_0_TOP;
        } else if (y_from < y_to) {
            return DIR_4_BOTTOM;
        }
    } else if (x_from > x_to) {
        if (y_from > y_to) {
            return DIR_7_TOP_LEFT;
        } else if (y_from == y_to) {
            return DIR_6_LEFT;
        } else if (y_from < y_to) {
            return DIR_5_BOTTOM_LEFT;
        }
    }
    return DIR_8_NONE;
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

