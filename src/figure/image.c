#include "image.h"

#include "city/view.h"

static const int CORPSE_IMAGE_OFFSETS[128] = {
    0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

static const int MISSILE_LAUNCHER_OFFSETS[128] = {
    0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const int CART_OFFSETS_X[] = {13, 18, 12, 0, -13, -18, -13, 0};
static const int CART_OFFSETS_Y[] = {-7, -1, 7, 11, 6, -1, -7, -12};

void figure_image_update(figure *f, int image_base)
{
    if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        f->image_id = image_base + CORPSE_IMAGE_OFFSETS[f->wait_ticks / 2] + 96;
    } else {
        f->image_id = image_base + figure_image_direction(f) + 8 * f->image_offset;
    }
}

void figure_image_increase_offset(figure *f, int max)
{
    f->image_offset++;
    if (f->image_offset >= max) {
        f->image_offset = 0;
    }
}

void figure_image_set_cart_offset(figure *f, int direction)
{
    f->x_offset_cart = CART_OFFSETS_X[direction];
    f->y_offset_cart = CART_OFFSETS_Y[direction];
}

int figure_image_corpse_offset(figure *f)
{
    return CORPSE_IMAGE_OFFSETS[f->wait_ticks / 2];
}

int figure_image_missile_launcher_offset(figure *f)
{
    return MISSILE_LAUNCHER_OFFSETS[f->attack_image_offset / 2];
}

int figure_image_direction(figure *f)
{
    int dir = f->direction - city_view_orientation();
    if (dir < 0) {
        dir += 8;
    }
    return dir;
}

int figure_image_normalize_direction(int direction)
{
    int normalized_direction = direction - city_view_orientation();
    if (normalized_direction < 0) {
        normalized_direction += 8;
    }
    return normalized_direction;
}

int figure_image_offset_direction(int direction, int offset)
{
    direction += offset;
    if (direction < 0) {
        direction += 8;
    }
    return direction;
}
