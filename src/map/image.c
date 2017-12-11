#include "image.h"

#include "map/grid.h"

#include "Data/Grid.h"

static grid_u16 images;
static grid_u16 images_backup;

int map_image_at(int grid_offset)
{
    return images.items[grid_offset];
}

void map_image_set(int grid_offset, int image)
{
    images.items[grid_offset] = image;
}

void map_image_backup()
{
    map_grid_copy_u16(images.items, images_backup.items);
}

void map_image_restore()
{
    map_grid_copy_u16(images_backup.items, images.items);
}

void map_image_restore_at(int grid_offset)
{
    images.items[grid_offset] = images_backup.items[grid_offset];
}

void map_image_clear()
{
    map_grid_clear_u16(images.items);
}

void map_image_save_state(buffer *buf)
{
    map_grid_save_state_u16(images.items, buf);
}

void map_image_load_state(buffer *buf)
{
    map_grid_load_state_u16(images.items, buf);
}
