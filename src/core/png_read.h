#ifndef CORE_PNG_H
#define CORE_PNG_H

#include "graphics/color.h"

int png_load(const char *path);

int png_get_image_size(const char *path, int *width, int *height);

int png_read(const char *path, color_t *pixels,
	int src_x, int src_y, int width, int height, int dst_x, int dst_y, int dst_row_width, int rotate);

void png_unload(void);

#endif // CORE_PNG_H
