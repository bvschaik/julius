#include "rotation.h"

#include "building/building_variant.h"
#include "building/construction.h"
#include "building/image_context.h"
#include "building/properties.h"
#include "city/view.h"
#include "core/config.h"
#include "core/direction.h"
#include "core/time.h"
#include "map/grid.h"

#define MAX_ROTATION 3
#define MAX_BIG_ROTATION 100

static struct {
    int rotation;
    int extra_rotation;
    int road_orientation;
} data = { 0, 0, 1 };

static void rotate_forward(void)
{
    building_construction_cycle_forward();
    data.rotation += 1;
    data.extra_rotation += 1;
    if (data.rotation > MAX_ROTATION) {
        data.rotation = 0;
    }
    if (data.extra_rotation >= MAX_BIG_ROTATION) {
        data.extra_rotation = 0;
    }
}

static void rotate_backward()
{
    building_construction_cycle_back();
    data.rotation -= 1;
    data.extra_rotation -= 1;
    if (data.rotation < 0) {
        data.rotation = MAX_ROTATION;
    }
    if (data.extra_rotation < 0) {
        data.extra_rotation = MAX_BIG_ROTATION;
    }
}

int building_rotation_get_road_orientation(void)
{
    return data.road_orientation;
}

void building_rotation_force_two_orientations(void)
{
    // for composite buildings like hippodrome
    if (data.rotation == 1 || data.rotation == 2) {
        data.rotation = 3;
    }
}

int building_rotation_get_rotation(void)
{
    return data.rotation;
}

int building_rotation_get_rotation_with_limit(int limit)
{
    return data.extra_rotation % limit;
}

void building_rotation_rotate_forward(void)
{
    rotate_forward();
    data.road_orientation = data.road_orientation == 1 ? 2 : 1;
}

void building_rotation_rotate_backward(void)
{
    rotate_backward();
    data.road_orientation = data.road_orientation == 1 ? 2 : 1;
}

void building_rotation_reset_rotation(void)
{
    data.rotation = 0;
    data.extra_rotation = 0;
}

int building_rotation_get_building_orientation(int building_rotation)
{
    return (2 * building_rotation + city_view_orientation()) % 8;
}

int building_rotation_get_delta_with_rotation(int default_delta)
{
    switch (data.rotation) {
        case 0:
            return map_grid_delta(default_delta, 0);
        case 1:
            return map_grid_delta(0, -default_delta);
        case 2:
            return map_grid_delta(-default_delta, 0);
        default:
            return map_grid_delta(0, default_delta);
    }
}

void building_rotation_get_offset_with_rotation(int offset, int rotation, int *x, int *y)
{
    switch (rotation) {
        case 0:
            *x = offset;
            *y = 0;
            return;
        case 1:
            *x = 0;
            *y = -offset;
            return;
        case 2:
            *x = -offset;
            *y = 0;
            return;
        default:
            *x = 0;
            *y = offset;
            return;
    }
}

int building_rotation_get_corner(int rotation)
{
    switch (rotation) {
        case DIR_2_RIGHT:
            return 4; // left corner
        case DIR_4_BOTTOM:
            return 8; // bottom corner
        case DIR_6_LEFT:
            return 5; // right corner
        default:
            return 0; // top corner
    }
}

int building_rotation_type_has_rotations(building_type type)
{
    if (building_variant_has_variants(type) || building_properties_for_type(type)->rotation_offset ||
        building_image_context_type_is_connecting(type)) {
        return 1;
    }
    switch (type) {
        case BUILDING_FORT_JAVELIN:
        case BUILDING_FORT_LEGIONARIES:
        case BUILDING_FORT_MOUNTED:
        case BUILDING_WAREHOUSE:
        case BUILDING_HIPPODROME:
            return 1;
        default:
            return 0;
    }
}
