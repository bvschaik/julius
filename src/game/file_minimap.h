#ifndef GAME_FILE_MINIMAP_H
#define GAME_FILE_MINIMAP_H

#include "core/buffer.h"
#include "graphics/color.h"

#define FILE_MINIMAP_IMAGE_WIDTH 324
#define FILE_MINIMAP_IMAGE_HEIGHT 324

typedef struct {
    buffer *map_terrain;
    buffer *map_bitfields;
    buffer *map_edge;
    buffer *map_random;
    buffer *map_building;
    buffer *buildings;
    int climate;
    int grid_width;
    int grid_height;
    int grid_start;
    int grid_border_size;
} file_buffers;

const color_t *game_file_minimap_create(const file_buffers *buffers);

#endif // GAME_FILE_MINIMAP_H
