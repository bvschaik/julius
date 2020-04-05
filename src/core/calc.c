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

int calc_distance_with_penalty(int x1, int y1, int x2, int y2,
                               int dist_to_entry1, int dist_to_entry2)
{
    int penalty;
    if (dist_to_entry1 > dist_to_entry2) {
        penalty = dist_to_entry1 - dist_to_entry2;
    } else {
        penalty = dist_to_entry2 - dist_to_entry1;
    }
    if (dist_to_entry1 == -1) {
        penalty = 0;
    }
    return penalty + calc_maximum_distance(x1, y1, x2, y2);
}

direction_type calc_general_direction(int x_from, int y_from, int x_to, int y_to)
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

direction_type calc_missile_shooter_direction(int x_from, int y_from, int x_to, int y_to)
{
    int dx = x_from > x_to ? x_from - x_to : x_to - x_from;
    int dy = y_from > y_to ? y_from - y_to : y_to - y_from;
    int percentage;
    if (dx > dy) {
        percentage = calc_percentage(dx, dy);
    } else if (dx == dy) {
        percentage = 100;
    } else {
        percentage = -calc_percentage(dy, dx);
    }
    if (x_from == x_to) {
        if (y_from < y_to) {
            return DIR_4_BOTTOM;
        } else {
            return DIR_0_TOP;
        }
    } else if (x_from > x_to) {
        if (y_from == y_to) {
            return DIR_6_LEFT;
        } else if (y_from > y_to) {
            if (percentage >= 400) {
                return DIR_6_LEFT;
            } else if (percentage > -400) {
                return DIR_7_TOP_LEFT;
            } else {
                return DIR_0_TOP;
            }
        } else {
            if (percentage >= 400) {
                return DIR_6_LEFT;
            } else if (percentage > -400) {
                return DIR_5_BOTTOM_LEFT;
            } else {
                return DIR_4_BOTTOM;
            }
        }
    } else { // x_from < x_to
        if (y_from == y_to) {
            return DIR_2_RIGHT;
        } else if (y_from > y_to) {
            if (percentage >= 400) {
                return DIR_2_RIGHT;
            } else if (percentage > -400) {
                return DIR_1_TOP_RIGHT;
            } else {
                return DIR_0_TOP;
            }
        } else {
            if (percentage >= 400) {
                return DIR_2_RIGHT;
            } else if (percentage > -400) {
                return DIR_3_BOTTOM_RIGHT;
            } else {
                return DIR_4_BOTTOM;
            }
        }
    }
}

int calc_missile_direction(int x_from, int y_from, int x_to, int y_to)
{
    int dx = x_from > x_to ? x_from - x_to : x_to - x_from;
    int dy = y_from > y_to ? y_from - y_to : y_to - y_from;
    int percentage;
    if (dx > dy) {
        percentage = calc_percentage(dx, dy);
    } else if (dx == dy) {
        percentage = 100;
    } else {
        percentage = -calc_percentage(dy, dx);
    }
    if (x_from == x_to) {
        if (y_from < y_to) {
            return 8;
        } else {
            return 0;
        }
    } else if (x_from > x_to) {
        if (y_from == y_to) {
            return 12;
        } else if (y_from > y_to) {
            if (percentage >= 500) {
                return 12;
            } else if (percentage >= 200) {
                return 13;
            } else if (percentage > -200) {
                return 14;
            } else if (percentage > -500) {
                return 15;
            } else {
                return 0;
            }
        } else {
            if (percentage >= 500) {
                return 12;
            } else if (percentage >= 200) {
                return 11;
            } else if (percentage > -200) {
                return 10;
            } else if (percentage > -500) {
                return 9;
            } else {
                return 8;
            }
        }
    } else { // x_from < x_to
        if (y_from == y_to) {
            return 4;
        } else if (y_from > y_to) {
            if (percentage >= 500) {
                return 4;
            } else if (percentage >= 200) {
                return 3;
            } else if (percentage > -200) {
                return 2;
            } else if (percentage > -500) {
                return 1;
            } else {
                return 0;
            }
        } else {
            if (percentage >= 500) {
                return 4;
            } else if (percentage >= 200) {
                return 5;
            } else if (percentage > -200) {
                return 6;
            } else if (percentage > -500) {
                return 7;
            } else {
                return 8;
            }
        }
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

int calc_absolute_increment(int value, int step, int max)
{
    if (step == 0) {
        step = 1;
    } else if (step < 0) {
        step = -step;
    }
    if (max >= 0) {
        return (value + step >= max) ? max : value + step;
    }
    return (value - step <= max) ? max : value - step;
}

int calc_absolute_decrement(int value, int step)
{
    if (value == 0) {
        return 0;
    }
    if (step == 0) {
        step = 1;
    } else if (step < 0) {
        step = -step;
    }
    if (value >= 0) {
        return (step >= value) ? 0 : value - step;
    }
    return (step >= -value) ? 0 : value + step;
}
