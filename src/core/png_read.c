#include "core/png_read.h"

#include "core/dir.h"
#include "core/file.h"
#include "core/log.h"
#include "graphics/color.h"

#include "png.h"

#include <stdio.h>
#include <stdlib.h>

#define BYTES_PER_PIXEL 4

static struct {
    png_structp png_ptr;
    png_infop info_ptr;
    FILE *fp;
} data;

static void unload_png(void)
{
    png_destroy_read_struct(&data.png_ptr, &data.info_ptr, 0);
    if (data.fp) {
        file_close(data.fp);
        data.fp = 0;
    }
}

static int load_png(const char *path)
{
    unload_png();
    png_byte header[8];
    const char *cased_path = dir_get_file(path, NOT_LOCALIZED);
    if (!cased_path) {
        log_error("Unable to open png file", path, 0);
        return 0;
    }
    data.fp = file_open(cased_path, "rb");
    if (!data.fp) {
        log_error("Unable to open png file", path, 0);
        return 0;
    }
    fread(header, 1, 8, data.fp);
    if (png_sig_cmp(header, 0, 8)) {
        log_error("Invalid png file", path, 0);
        unload_png();
        return 0;
    }

    data.png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!data.png_ptr) {
        log_error("Unable to create a png struct", 0, 0);
        unload_png();
        return 0;
    }
    data.info_ptr = png_create_info_struct(data.png_ptr);
    if (!data.info_ptr) {
        log_error("Unable to create a png struct", 0, 0);
        unload_png();
        return 0;
    }

    if (setjmp(png_jmpbuf(data.png_ptr))) {
        log_error("Unable to read png information", 0, 0);
        unload_png();
        return 0;
    }
    png_init_io(data.png_ptr, data.fp);
    png_set_sig_bytes(data.png_ptr, 8);
    png_read_info(data.png_ptr, data.info_ptr);
    return 1;
}

int png_get_image_size(const char *path, int *width, int *height)
{
    *width = 0;
    *height = 0;
    if (!load_png(path)) {
        return 0;
    }
    *width = png_get_image_width(data.png_ptr, data.info_ptr);
    *height = png_get_image_height(data.png_ptr, data.info_ptr);
    unload_png();

    return 1;
}

int png_read(const char *path, color_t *pixels)
{
    if (!load_png(path)) {
        return 0;
    }
    png_bytep row = 0;
    if (setjmp(png_jmpbuf(data.png_ptr))) {
        log_error("Unable to read png file", 0, 0);
        free(row);
        unload_png();
        return 0;
    }
    png_set_gray_to_rgb(data.png_ptr);
    png_set_filler(data.png_ptr, 0xFF, PNG_FILLER_AFTER);
    png_set_expand(data.png_ptr);
    png_set_strip_16(data.png_ptr);
    if (png_set_interlace_handling(data.png_ptr) != 1) {
        log_info("The image has interlacing and therefore will not open correctly", 0, 0);
    }
    png_read_update_info(data.png_ptr, data.info_ptr);

    int width = png_get_image_width(data.png_ptr, data.info_ptr);
    int height = png_get_image_height(data.png_ptr, data.info_ptr);
    row = malloc(sizeof(png_byte) * width * BYTES_PER_PIXEL);
    if (!row) {
        log_error("Unable to load png file. Out of memory", 0, 0);
        unload_png();
        return 0;
    }
    color_t *dst = pixels;
    for (int y = 0; y < height; ++y) {
        png_read_row(data.png_ptr, row, 0);
        png_bytep src = row;
        for (int x = 0; x < width; ++x) {
            *dst  = ((color_t) *(src + 0)) << COLOR_BITSHIFT_RED;
            *dst |= ((color_t) *(src + 1)) << COLOR_BITSHIFT_GREEN;
            *dst |= ((color_t) *(src + 2)) << COLOR_BITSHIFT_BLUE;
            *dst |= ((color_t) *(src + 3)) << COLOR_BITSHIFT_ALPHA;
            dst++;
            src += BYTES_PER_PIXEL;
        }
    }
    free(row);
    unload_png();
    return 1;
}
