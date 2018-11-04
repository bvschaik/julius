#include "orientation.h"

#include "city/view.h"
#include "city/warning.h"
#include "core/direction.h"
#include "map/orientation.h"

void game_orientation_rotate_left(void)
{
    city_view_rotate_left();
    map_orientation_change(0);
    city_warning_show(WARNING_ORIENTATION);
}

void game_orientation_rotate_right(void)
{
    city_view_rotate_right();
    map_orientation_change(1);
    city_warning_show(WARNING_ORIENTATION);
}

void game_orientation_rotate_north(void)
{
    switch (city_view_orientation()) {
        case DIR_2_RIGHT:
            city_view_rotate_right();
            map_orientation_change(1);
            break;
        case DIR_4_BOTTOM:
            city_view_rotate_left();
            map_orientation_change(0);
            // fallthrough
        case DIR_6_LEFT:
            city_view_rotate_left();
            map_orientation_change(0);
            break;
        default: // already north
            return;
    }
    city_warning_show(WARNING_ORIENTATION);
}
