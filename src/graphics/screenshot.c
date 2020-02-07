#include "screenshot.h"

#include "city/view.h"
#include "core/buffer.h"
#include "core/file.h"
#include "core/log.h"
#include "graphics/screen.h"
#include "graphics/graphics.h"
#include "map/grid.h"
#include "graphics/window.h"
#include "widget/city_without_overlay.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HEADER_SIZE 26
#define TILE_X_SIZE 60
#define TILE_Y_SIZE 30
#define IMAGE_HEIGHT_CHUNK (2 * TILE_Y_SIZE)
#define BMP_BITS_PER_PIXEL 24
#define BMP_BYTES_PER_PIXEL (BMP_BITS_PER_PIXEL / 8)

static const char filename_format[] = "city %Y-%m-%d %H.%M.%S.bmp";

static void full_city_screenshot(void);

static struct {
    int width;
    int height;
    int scanline_size;
    uint8_t *pixels;
} bmp_chunk;

static void free_bmp_chunk(void)
{
    bmp_chunk.width = 0;
    bmp_chunk.height = 0;
    bmp_chunk.scanline_size = 0;
    free(bmp_chunk.pixels);
    bmp_chunk.pixels = 0;
}

static int create_bmp_chunk(int width, int height)
{
    free(bmp_chunk.pixels);
    if (!width || !height) {
        return 0;
    }
    int scanline_padding = 4 - (width * BMP_BYTES_PER_PIXEL) % 4;
    if (scanline_padding == 4) {
        scanline_padding = 0;
    }
    bmp_chunk.width = width;
    bmp_chunk.height = height;
    bmp_chunk.scanline_size = width * BMP_BYTES_PER_PIXEL + scanline_padding;
    bmp_chunk.pixels = (uint8_t *) malloc(bmp_chunk.scanline_size * height);
    if (!bmp_chunk.pixels) {
        free_bmp_chunk();
        return 0;
    }
    memset(bmp_chunk.pixels, 0, bmp_chunk.scanline_size * height);
    return 1;
}

static const char *generate_filename(void)
{
    static char filename[100];
    time_t curtime = time(NULL);
    struct tm *loctime = localtime(&curtime);

    strftime(filename, 100, filename_format, loctime);

    return filename;
}

static void write_bmp_header(buffer *buf, int full_height)
{
    buffer_write_i8(buf, 'B');
    buffer_write_i8(buf, 'M');
    buffer_write_i32(buf, HEADER_SIZE + bmp_chunk.scanline_size * full_height); // file size
    buffer_write_i32(buf, 0); // reserved
    buffer_write_i32(buf, HEADER_SIZE); // data offset
    buffer_write_i32(buf, 12); // dib size
    buffer_write_i16(buf, (int16_t) bmp_chunk.width);
    buffer_write_i16(buf, (int16_t) full_height);
    buffer_write_i16(buf, 1); // planes
    buffer_write_i16(buf, BMP_BITS_PER_PIXEL);
}

static void pixel(color_t input, uint8_t *r, uint8_t *g, uint8_t *b)
{
    color_t rr = (input & 0xff0000) >> 16;
    color_t gg = (input & 0x00ff00) >> 8;
    color_t bb = (input & 0x0000ff) >> 0;
    *r = (uint8_t) rr;
    *g = (uint8_t) gg;
    *b = (uint8_t) bb;
}

void graphics_save_screenshot(int full_city)
{
    if (full_city) {
        full_city_screenshot();
        return;
    }
    uint8_t header[HEADER_SIZE];
    buffer buf;
    buffer_init(&buf, header, HEADER_SIZE);

    int width = screen_width();
    int height = screen_height();
    const color_t *canvas = graphics_canvas();

    if (!create_bmp_chunk(width, 1)) {
        log_error("Unable to create memory for screenshot", 0, 0);
        return;
    }

    const char *filename = generate_filename();
    FILE *fp = file_open(filename, "wb");
    if (!fp) {
        log_error("Unable to write screenshot to:", filename, 0);
        free_bmp_chunk();
        return;
    }

    write_bmp_header(&buf, height);
    fwrite(header, 1, HEADER_SIZE, fp);
    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            pixel(canvas[y * width + x],
                &bmp_chunk.pixels[3*x+2], &bmp_chunk.pixels[3*x+1], &bmp_chunk.pixels[3*x]);
        }
        fwrite(bmp_chunk.pixels, sizeof(uint8_t), bmp_chunk.scanline_size, fp);
    }
    file_close(fp);
    free_bmp_chunk();

    log_info("Saved screenshot:", filename, 0);
}

static void full_city_screenshot(void)
{
    if (!window_is(WINDOW_CITY) && !window_is(WINDOW_CITY_MILITARY)) {
        return;
    }
    pixel_offset original_camera_pixels;
    city_view_get_camera_in_pixels(&original_camera_pixels.x, &original_camera_pixels.y);
    int width = screen_width();
    int height = screen_height();

    int city_width_pixels = map_grid_width() * TILE_X_SIZE;
    int city_height_pixels = map_grid_height() * TILE_Y_SIZE;

    if (!create_bmp_chunk(city_width_pixels, IMAGE_HEIGHT_CHUNK)) {
        log_error("Unable to assign memory for full city screenshot creation", 0, 0);
        return;
    }

    const char *filename = generate_filename();
    FILE *fp = file_open(filename, "wb");
    if (!fp) {
        log_error("Unable to write screenshot to:", filename, 0);
        free_bmp_chunk();
        return;
    }

    uint8_t header[HEADER_SIZE];
    buffer buf;
    buffer_init(&buf, header, HEADER_SIZE);
    write_bmp_header(&buf, city_height_pixels);
    fwrite(header, 1, HEADER_SIZE, fp);

    int new_width = city_width_pixels + (city_view_is_sidebar_collapsed() ? 40 : 160);
    screen_set_resolution(new_width, 24 + IMAGE_HEIGHT_CHUNK);
    graphics_set_clip_rectangle(0, 24, city_width_pixels, IMAGE_HEIGHT_CHUNK);

    int base_width = (GRID_SIZE * TILE_X_SIZE - city_width_pixels) / 2 + TILE_X_SIZE;
    int max_height = (GRID_SIZE * TILE_Y_SIZE + city_height_pixels) / 2;
    int min_height = max_height - city_height_pixels;
    map_tile dummy_tile = { 0, 0, 0 };
    const color_t *canvas = graphics_canvas();

    for (int current_height = max_height - IMAGE_HEIGHT_CHUNK; current_height >= min_height; current_height -= IMAGE_HEIGHT_CHUNK) {
        city_view_set_camera_from_pixel_position(base_width, current_height);
        city_without_overlay_draw(0, 0, &dummy_tile);
        for (int y = IMAGE_HEIGHT_CHUNK - 1; y >= 0; y--) {
            for (int x = 0; x < city_width_pixels; x++) {
                pixel(canvas[(y + 24) * new_width + x],
                    &bmp_chunk.pixels[3 * x + 2], &bmp_chunk.pixels[3 * x + 1], &bmp_chunk.pixels[3 * x]);
            }
            fwrite(bmp_chunk.pixels, sizeof(uint8_t), bmp_chunk.scanline_size, fp);
        }
    }
    graphics_reset_clip_rectangle();
    screen_set_resolution(width, height);
    city_view_set_camera_from_pixel_position(original_camera_pixels.x, original_camera_pixels.y);

    file_close(fp);
    free_bmp_chunk();

    log_info("Saved full city screenshot:", filename, 0);
}
