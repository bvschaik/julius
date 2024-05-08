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

typedef enum {
    CACHE_TYPE_NONE = 0,
    CACHE_TYPE_FILE,
    CACHE_TYPE_MEMORY
} cache_type;

static struct {
    spng_ctx *ctx;
    FILE *fp;
    struct {
        cache_type type;
        char path[FILE_NAME_MAX];
        const uint8_t *buffer;
        int width;
        int height;
        color_t *pixels;
    } cache;
} data;

int png_load_from_file(const char *path, int is_asset)
{
    if (data.cache.type == CACHE_TYPE_FILE && strcmp(path, data.cache.path) == 0) {
        return 1;
    }
    png_unload();
    data.fp = is_asset ? file_open_asset(path, "rb") : file_open(path, "rb");
    if (!data.fp) {
        log_error("Unable to open png file", path, 0);
        return 0;
    }
    data.ctx = spng_ctx_new(0);
    if (!data.ctx) {
        log_error("Unable to create a png handle context", 0, 0);
        png_unload();
        return 0;
    }
    if (spng_set_png_file(data.ctx, data.fp)) {
        log_error("Unable to set png file stream", 0, 0);
        png_unload();
        return 0;
    }
    data.cache.type = CACHE_TYPE_FILE;
    snprintf(data.cache.path, FILE_NAME_MAX, "%s", path);
    return 1;
}

int png_load_from_buffer(const uint8_t *buffer, size_t length)
{
    if (data.cache.type == CACHE_TYPE_MEMORY && buffer == data.cache.buffer) {
        return 1;
    }
    png_unload();
    if (!buffer) {
        log_error("Unable to open png file - no buffer provided", 0, 0);
        return 0;
    }
    data.ctx = spng_ctx_new(0);
    if (!data.ctx) {
        log_error("Unable to create a png handle context", 0, 0);
        png_unload();
        return 0;
    }
    if (spng_set_png_buffer(data.ctx, buffer, length)) {
        log_error("Unable to set png buffer", 0, 0);
        png_unload();
        return 0;
    }
    data.cache.type = CACHE_TYPE_MEMORY;
    data.cache.buffer = buffer;
    return 1;
}

int png_get_image_size(int *width, int *height)
{
    if (data.cache.width && data.cache.height) {
        *width = data.cache.width;
        *height = data.cache.height;
        return 1;
    }
    *width = 0;
    *height = 0;
    if (!data.ctx) {
        return 0;
    }
    struct spng_ihdr ihdr;
    if (spng_get_ihdr(data.ctx, &ihdr)) {
        return 0;
    }
    data.cache.width = *width = (int) ihdr.width;
    data.cache.height = *height = (int) ihdr.height;

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

static void close_png(void)
{
    spng_ctx_free(data.ctx);
    data.ctx = 0;
    if (data.fp) {
        file_close(data.fp);
        data.fp = 0;
    }
}

static int load_image(void)
{
    size_t image_size;
    if (spng_decoded_image_size(data.ctx, SPNG_FMT_RGBA8, &image_size)) {
        log_error("Unable to retrieve png image size", 0, 0);
        png_unload();
        return 0;
    }
    int total_pixels = data.cache.width * data.cache.height;
    data.cache.pixels = malloc(image_size);
    if (!data.cache.pixels) {
        log_error("Unable to load png file. Out of memory", 0, 0);
        png_unload();
        return 0;
    }
    if (spng_decode_image(data.ctx, data.cache.pixels, image_size, SPNG_FMT_RGBA8, SPNG_DECODE_TRNS)) {
        log_error("Unable to start decoding png file", 0, 0);
        png_unload();
        return 0;
    }
    convert_image_to_argb(data.cache.pixels, total_pixels);
    close_png();
    return 1;
}

static void set_pixels(color_t *pixels,
    int src_x, int src_y, int width, int height, int dst_x, int dst_y, int dst_row_width, int rotate)
{
    int readable_height = (height + src_y <= data.cache.height) ?
        height : (data.cache.height - src_y);
    int readable_width = (width + src_x <= data.cache.width) ? width : (data.cache.width - src_x);

    if (!rotate) {
        for (int y = 0; y < readable_height; y++) {
            memcpy(&pixels[(y + dst_y) * dst_row_width + dst_x],
                &data.cache.pixels[(src_y + y) * data.cache.width + src_x],
                readable_width * sizeof(color_t));
        }
    } else {
        for (int y = 0; y < readable_height; y++) {
            color_t *src_pixel = &data.cache.pixels[(src_y + y) * data.cache.width + src_x];
            color_t *dst_pixel = &pixels[(dst_y + width - 1) *
                dst_row_width + y + dst_x];
            for (int x = 0; x < readable_width; x++) {
                *dst_pixel = *src_pixel++;
                dst_pixel -= dst_row_width;
            }
        }
    }
}

int png_read(color_t *pixels, int src_x, int src_y, int width, int height,
    int dst_x, int dst_y, int dst_row_width, int rotate)
{
    if (!data.cache.pixels) {
        if (!data.ctx) {
            return 0;
        }
        if (!png_get_image_size(&data.cache.width, &data.cache.height) || !load_image()) {
            return 0;
        }
    }
    set_pixels(pixels, src_x, src_y, width, height, dst_x, dst_y, dst_row_width, rotate);
    return 1;
}

void png_unload(void)
{
    close_png();
    free(data.cache.pixels);
    memset(&data.cache, 0, sizeof(data.cache));
}
