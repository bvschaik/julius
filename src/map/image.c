#include "image.h"

#include "building/image.h"
#include "building/industry.h"
#include "core/image.h"
#include "core/image_group.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/orientation.h"
#include "map/tiles.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    uint32_t *ids;
    int total;
} tile_images;

static tile_images images[GRID_SIZE * GRID_SIZE];
static tile_images images_backup[GRID_SIZE * GRID_SIZE];

uint32_t map_image_at(int grid_offset)
{
    return images[grid_offset].total ? images[grid_offset].ids[0] : 0;
}

void map_image_set(int grid_offset, int image_id)
{
    tile_images *image = &images[grid_offset];
    free(image->ids);
    image->total = 0;
    if (image_id <= 0) {
        image->ids = 0;
        return;
    }
    image->ids = malloc(sizeof(uint32_t));
    if (image->ids) {
        image->total = 1;
        image->ids[0] = image_id;
    }
}

void map_image_add(int grid_offset, int image_id)
{
    if (image_id <= 0) {
        return;
    }
    tile_images *image = &images[grid_offset];
    if (image->total == 0) {
        map_image_set(grid_offset, image_id);
        return;
    }
    uint32_t *ids = realloc(image->ids, sizeof(uint32_t) * (image->total + 1));
    if (ids) {
        image->ids = ids;
        image->ids[image->total] = image_id;
        image->total++;
    }
}

static void copy_tile_image(tile_images *dst, const tile_images *src)
{
    free(dst->ids);
    dst->total = 0;
    if (!src->total) {
        dst->ids = 0;
        return;
    }
    dst->ids = malloc(sizeof(tile_images) * src->total);
    if (!dst->ids) {
        return;
    }
    memcpy(dst->ids, src->ids, sizeof(tile_images) * src->total);
    dst->total = src->total;
}

void map_image_backup(void)
{
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        copy_tile_image(&images_backup[i], &images[i]);
    }
}

void map_image_restore(void)
{
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        copy_tile_image(&images[i], &images_backup[i]);
    }
}

void map_image_restore_at(int grid_offset)
{
    copy_tile_image(&images[grid_offset], &images_backup[grid_offset]);
}

void map_image_clear(void)
{
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        free(images[i].ids);
        images[i].ids = 0;
        images[i].total = 0;
    }
}

void map_image_init_edges(void)
{
    int width, height;
    map_grid_size(&width, &height);
    for (int x = 1; x < width; x++) {
        map_image_set(map_grid_offset(x, height), 1);
    }
    for (int y = 1; y < height; y++) {
        map_image_set(map_grid_offset(width, y), 2);
    }
    map_image_set(map_grid_offset(0, height), 3);
    map_image_set(map_grid_offset(width, 0), 4);
    map_image_set(map_grid_offset(width, height), 5);
}

void map_image_update_all(void)
{
    map_tiles_update_all();
    for (int i = 1; i < building_count(); i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE && b->state != BUILDING_STATE_MOTHBALLED && b->state != BUILDING_STATE_CREATED) {
            continue;
        }
        if (building_is_farm(b->type)) {
            map_building_tiles_add_farm(b->id, b->x, b->y,
                image_group(GROUP_BUILDING_FARM_CROPS) + 5 * (b->output_resource_id - 1),
                b->data.industry.progress);
            continue;
        }
        int image_id = building_image_get(b);

        for (int dy = 0; dy < b->size; dy++) {
            for (int dx = 0; dx < b->size; dx++) {
                map_image_set(map_grid_offset(b->x + dx, b->y + dy), image_id);
            }
        }
    }
}

void map_image_save_state(buffer *buf)
{
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        uint16_t value = images[i].total ? (uint16_t) images[i].ids[0] : 0;
        buffer_write_u16(buf, value);
    }
}

void map_image_load_state(buffer *buf)
{
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        map_image_set(i, buffer_read_u16(buf));
    }
}
