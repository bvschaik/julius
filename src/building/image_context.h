#ifndef BUILDING_IMAGE_CONTEXT_H
#define BUILDING_IMAGE_CONTEXT_H

#define CONNECTING_BUILDINGS_ROTATION_LIMIT 3

typedef struct {
    int is_valid;
    int group_offset;
    int item_offset;
} building_image;

void building_image_context_init(void);

void building_image_context_set_hedge_image(int grid_offset);

void building_image_context_clear_connection_grid(void);

void building_image_context_set_connecting_type(int building_type);

void building_image_context_mark_connection_grid(int map_grid);

const building_image* building_image_context_get_hedges(int grid_offset);

const building_image* building_image_context_get_colonnade(int grid_offset);

const building_image* building_image_context_get_garden_path(int grid_offset, int context);

int building_image_context_get_connecting_image_for_tile(int grid_offset, int building_type);

#endif // BUILDING_IMAGE_CONTEXT_H

