#include "image.h"

#include "map/grid.h"

static grid_u32 images;
static grid_u32 images_backup;

unsigned int map_image_at(int grid_offset)
{
    return images.items[grid_offset];
}

void map_image_set(int grid_offset, int image_id)
{
    images.items[grid_offset] = image_id;
}

void map_image_backup(void)
{
    map_grid_copy_u32(images.items, images_backup.items);
}

void map_image_restore(void)
{
    map_grid_copy_u32(images_backup.items, images.items);
}

void map_image_restore_at(int grid_offset)
{
    images.items[grid_offset] = images_backup.items[grid_offset];
}

void map_image_clear(void)
{
    map_grid_clear_u32(images.items);
}

void map_image_init_edges(void)
{
    int width, height;
    map_grid_size(&width, &height);
    for (int x = 1; x < width; x++) {
        images.items[map_grid_offset(x, height)] = 1;
    }
    for (int y = 1; y < height; y++) {
        images.items[map_grid_offset(width, y)] = 2;
    }
    images.items[map_grid_offset(0, height)] = 3;
    images.items[map_grid_offset(width, 0)] = 4;
    images.items[map_grid_offset(width, height)] = 5;
}

void map_image_save_state_legacy(buffer *buf)
{
    map_grid_save_state_u32_to_u16(images.items, buf);
}

void map_image_save_state(buffer *buf)
{
    map_grid_save_state_u32(images.items, buf);
}

void map_image_load_state_legacy(buffer *buf)
{
    map_grid_load_state_u16_to_u32(images.items, buf);
}

void map_image_load_state(buffer *buf)
{
    map_grid_load_state_u32(images.items, buf);
}
