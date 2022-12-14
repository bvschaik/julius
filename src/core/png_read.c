#include "core/png_read.h"

#include "core/dir.h"
#include "core/file.h"
#include "core/log.h"
#include "graphics/color.h"

#include "png.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BYTES_PER_PIXEL 4

static struct {
    png_structp png_ptr;
    png_infop info_ptr;
    FILE *fp;
    struct {
        char path[FILE_NAME_MAX];
        int width;
        int height;
        color_t *pixels;
        int buffer_size;
    } last_png;
} data;

static void unload_png(void)
{
    png_destroy_read_struct(&data.png_ptr, &data.info_ptr, 0);
    if (data.fp) {
        file_close(data.fp);
        data.fp = 0;
    }
}

int png_load(const char *path)
{
    if (strcmp(path, data.last_png.path) == 0) {
        return 1;
    }
    unload_png();
    data.last_png.width = 0;
    data.last_png.height = 0;
    png_byte header[8];
    data.fp = file_open_asset(path, "rb");
    if (!data.fp) {
        log_error("Unable to open png file", path, 0);
        return 0;
    }
    size_t bytes_read = fread(header, 1, 8, data.fp);
    if (bytes_read != 8 || png_sig_cmp(header, 0, 8)) {
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
    strncpy(data.last_png.path, path, FILE_NAME_MAX - 1);
    return 1;
}

int png_get_image_size(const char *path, int *width, int *height)
{
    *width = 0;
    *height = 0;
    if (!png_load(path)) {
        return 0;
    }
    *width = !data.last_png.width ? png_get_image_width(data.png_ptr, data.info_ptr) : data.last_png.width;
    *height = !data.last_png.height ? png_get_image_height(data.png_ptr, data.info_ptr) : data.last_png.height;

    return 1;
}

static int load_image(void)
{
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

    row = malloc(sizeof(png_byte) * data.last_png.width * BYTES_PER_PIXEL);
    if (!row) {
        log_error("Unable to load png file. Out of memory", 0, 0);
        unload_png();
        return 0;
    }
    color_t *dst = data.last_png.pixels;
    if (data.last_png.buffer_size < data.last_png.width * data.last_png.height) {
        dst = realloc(data.last_png.pixels, sizeof(color_t) * data.last_png.width * data.last_png.height);
        if (!dst) {
            free(row);
            log_error("Unable to load png file. Out of memory", 0, 0);
            unload_png();
            return 0;
        }
        data.last_png.pixels = dst;
        data.last_png.buffer_size = data.last_png.width * data.last_png.height;
    }
    for (int y = 0; y < data.last_png.height; ++y) {
        png_read_row(data.png_ptr, row, 0);
        png_bytep src = row;
        for (int x = 0; x < data.last_png.width; ++x) {
            *dst = ((color_t) * (src + 0)) << COLOR_BITSHIFT_RED;
            *dst |= ((color_t) * (src + 1)) << COLOR_BITSHIFT_GREEN;
            *dst |= ((color_t) * (src + 2)) << COLOR_BITSHIFT_BLUE;
            *dst |= ((color_t) * (src + 3)) << COLOR_BITSHIFT_ALPHA;
            dst++;
            src += BYTES_PER_PIXEL;
        }
    }
    free(row);
    unload_png();
    return 1;
}

static void set_pixels(color_t *pixels,
    int src_x, int src_y, int width, int height, int dst_x, int dst_y, int dst_row_width, int rotate)
{
    int readable_height = (height + src_y <= data.last_png.height) ?
        height : (data.last_png.height - src_y);
    int readable_width = (width + src_x <= data.last_png.width) ? width : (data.last_png.width - src_x);

    if (!rotate) {
        for (int y = 0; y < readable_height; y++) {
            memcpy(&pixels[(y + dst_y) * dst_row_width + dst_x],
                &data.last_png.pixels[(src_y + y) * data.last_png.width + src_x],
                readable_width * sizeof(color_t));
        }
    } else {
        for (int y = 0; y < readable_height; y++) {
            color_t *src_pixel = &data.last_png.pixels[(src_y + y) * data.last_png.width + src_x];
            color_t *dst_pixel = &pixels[(dst_y + width - 1) *
                dst_row_width + y + dst_x];
            for (int x = 0; x < readable_width; x++) {
                *dst_pixel = *src_pixel++;
                dst_pixel -= dst_row_width;
            }
        }
    }
}

int png_read(const char *path, color_t *pixels,
    int src_x, int src_y, int width, int height, int dst_x, int dst_y, int dst_row_width, int rotate)
{
    if (!png_load(path)) {
        return 0;
    }
    if (!data.last_png.width && !data.last_png.height) {
        png_get_image_size(path, &data.last_png.width, &data.last_png.height);
        if (!load_image()) {
            return 0;
        }
    }
    set_pixels(pixels, src_x, src_y, width, height, dst_x, dst_y, dst_row_width, rotate);
    return 1;
}

void png_unload(void)
{
    unload_png();
    free(data.last_png.pixels);
    memset(&data.last_png, 0, sizeof(data.last_png));
}
