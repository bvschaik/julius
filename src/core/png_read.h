#ifndef CORE_PNG_H
#define CORE_PNG_H

#include "graphics/color.h"

#include <stddef.h>
#include <stdint.h>

int png_load_from_file(const char *path, int is_asset);
int png_load_from_buffer(const uint8_t *buffer, size_t length);

int png_get_image_size(int *width, int *height);

int png_read(color_t *pixels, int src_x, int src_y, int width, int height,
	int dst_x, int dst_y, int dst_row_width, int rotate);

void png_unload(void);

#endif // CORE_PNG_H
