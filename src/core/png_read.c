#include "core/png_read.h"

#include "core/dir.h"
#include "core/file.h"
#include "core/log.h"
#include "graphics/color.h"

#include "spng/spng.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BYTES_PER_PIXEL 4

static struct {
    spng_ctx *ctx;
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
    spng_ctx_free(data.ctx);
    data.ctx = 0;
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
    data.fp = file_open_asset(path, "rb");
    if (!data.fp) {
        log_error("Unable to open png file", path, 0);
        return 0;
    }
    data.ctx = spng_ctx_new(0);
    if (!data.ctx) {
        log_error("Unable to create a png handle context", 0, 0);
        unload_png();
        return 0;
    }
    if (spng_set_png_file(data.ctx, data.fp)) {
        log_error("Unable to set png file stream", 0, 0);
        unload_png();
        return 0;
    }
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
    struct spng_ihdr ihdr;
    if (spng_get_ihdr(data.ctx, &ihdr)) {
        return 0;
    }
    *width = (int) ihdr.width;
    *height = (int) ihdr.height;

    return 1;
}

static void convert_image_to_argb(color_t *pixels, int total_pixels)
{
    uint8_t *src = (uint8_t *) pixels;
    for (int i = 0; i < total_pixels; ++i) {
        color_t pixel = ((color_t) * (src + 0)) << COLOR_BITSHIFT_RED;
        pixel |= ((color_t) * (src + 1)) << COLOR_BITSHIFT_GREEN;
        pixel |= ((color_t) * (src + 2)) << COLOR_BITSHIFT_BLUE;
        pixel |= ((color_t) * (src + 3)) << COLOR_BITSHIFT_ALPHA;
        *pixels = pixel;
        pixels++;
        src += BYTES_PER_PIXEL;
    }
}

static int load_image(void)
{
    size_t image_size;
    if (spng_decoded_image_size(data.ctx, SPNG_FMT_RGBA8, &image_size)) {
        log_error("Unable to retrieve png image size", 0, 0);
        unload_png();
        return 0;
    }
    color_t *dst = data.last_png.pixels;
    int total_pixels = data.last_png.width * data.last_png.height;
    if (data.last_png.buffer_size < total_pixels) {
        dst = realloc(data.last_png.pixels, image_size);
        if (!dst) {
            log_error("Unable to load png file. Out of memory", 0, 0);
            unload_png();
            return 0;
        }
        data.last_png.pixels = dst;
        data.last_png.buffer_size = total_pixels;
    }
    if (spng_decode_image(data.ctx, dst, image_size, SPNG_FMT_RGBA8, SPNG_DECODE_TRNS)) {
        log_error("Unable to start decoding png file", 0, 0);
        unload_png();
        return 0;
    }
    convert_image_to_argb(dst, total_pixels);
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
