#include "screenshot.h"

#include "city/view.h"
#include "core/buffer.h"
#include "core/config.h"
#include "core/file.h"
#include "core/log.h"
#include "game/system.h"
#include "graphics/screen.h"
#include "graphics/graphics.h"
#include "graphics/menu.h"
#include "graphics/window.h"
#include "map/grid.h"
#include "widget/city_without_overlay.h"

#include "png.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TILE_X_SIZE 60
#define TILE_Y_SIZE 30
#define IMAGE_HEIGHT_CHUNK TILE_Y_SIZE
#define IMAGE_BYTES_PER_PIXEL 3

enum {
    FULL_CITY_SCREENSHOT = 0,
    DISPLAY_SCREENSHOT = 1,
    MAX_SCREENSHOT_TYPES = 2
};

static const char filename_formats[MAX_SCREENSHOT_TYPES][32] = {
    "full city %Y-%m-%d %H.%M.%S.png",
    "city %Y-%m-%d %H.%M.%S.png",
};

static struct {
    int width;
    int height;
    int row_size;
    int rows_in_memory;
    int current_y;
    int final_y;
    uint8_t *pixels;
    FILE *fp;
    png_structp png_ptr;
    png_infop info_ptr;
} image;

static void image_free(void)
{
    image.width = 0;
    image.height = 0;
    image.row_size = 0;
    image.rows_in_memory = 0;
    free(image.pixels);
    image.pixels = 0;
    if (image.fp) {
        file_close(image.fp);
        image.fp = 0;
    }
    png_destroy_write_struct(&image.png_ptr, &image.info_ptr);
}

static int image_create(int width, int height, int rows_in_memory)
{
    image_free();
    if (!width || !height || !rows_in_memory) {
        return 0;
    }
    image.png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!image.png_ptr) {
        return 0;
    }
    image.info_ptr = png_create_info_struct(image.png_ptr);
    if (!image.info_ptr) {
        image_free();
        return 0;
    }
    png_set_compression_level(image.png_ptr, 3);
    image.width = width;
    image.height = height;
    image.row_size = width * IMAGE_BYTES_PER_PIXEL;
    image.rows_in_memory = rows_in_memory;
    image.pixels = (uint8_t *) malloc(image.row_size);
    if (!image.pixels) {
        image_free();
        return 0;
    }
    memset(image.pixels, 0, image.row_size);
    return 1;
}

static const char *generate_filename(int city_screenshot)
{
    static char filename[FILE_NAME_MAX];
    time_t curtime = time(NULL);
    struct tm *loctime = localtime(&curtime);
    strftime(filename, FILE_NAME_MAX, filename_formats[city_screenshot], loctime);
    return filename;
}

int image_begin_io(const char *filename)
{
    FILE *fp = file_open(filename, "wb");
    if (!fp) {
        return 0;
    }
    image.fp = fp;
    png_init_io(image.png_ptr, fp);
    return 1;
}

static int image_write_header(void)
{
    if (setjmp(png_jmpbuf(image.png_ptr))) {
        return 0;
    }
    png_set_IHDR(image.png_ptr, image.info_ptr, image.width, image.height, 8, PNG_COLOR_TYPE_RGB,
                    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(image.png_ptr, image.info_ptr);
    return 1;
}

static int image_set_loop_height_limits(int min, int max)
{
    image.current_y = min;
    image.final_y = max;
    return image.current_y;
}

static int image_request_rows(void)
{
    if (image.current_y != image.final_y) {
        image.current_y += image.rows_in_memory;
        return image.rows_in_memory;
    }
    return 0;
}

static int image_write_rows(const color_t *canvas, int canvas_width)
{
    if (setjmp(png_jmpbuf(image.png_ptr))) {
        return 0;
    }
    for (int y = 0; y < image.rows_in_memory; ++y) {
        uint8_t *pixel = image.pixels;
        for (int x = 0; x < image.width; x++) {
            color_t input = canvas[y * canvas_width + x];
            *(pixel + 0) = (uint8_t) ((input & 0xff0000) >> 16);
            *(pixel + 1) = (uint8_t) ((input & 0x00ff00) >> 8);
            *(pixel + 2) = (uint8_t) ((input & 0x0000ff) >> 0);
            pixel += 3;
        }
        png_write_row(image.png_ptr, image.pixels);
    }
    return 1;
}

static int image_write_canvas(void)
{
    const color_t *canvas;
    color_t *screen_buffer = 0;
    if (config_get(CONFIG_UI_ZOOM)) {
        screen_buffer = malloc(image.width * image.height * sizeof(color_t));
        if (!system_save_screen_buffer(screen_buffer)) {
            free(screen_buffer);
            return 0;
        }
        canvas = screen_buffer;
    } else {
        canvas = graphics_canvas(CANVAS_UI);
    }
    int current_height = image_set_loop_height_limits(0, image.height);
    int size;
    while ((size = image_request_rows())) {
        if (!image_write_rows(canvas + current_height * image.width, image.width)) {
            free(screen_buffer);
            return 0;
        }
        current_height += size;
    }
    free(screen_buffer);
    return 1;
}

static void image_finish(void)
{
    png_write_end(image.png_ptr, image.info_ptr);
}

static void create_window_screenshot(void)
{
    int width = screen_width();
    int height = screen_height();

    if (!image_create(width, height, 1)) {
        log_error("Unable to create memory for screenshot", 0, 0);
        return;
    }

    const char *filename = generate_filename(DISPLAY_SCREENSHOT);
    if (!image_begin_io(filename) || !image_write_header()) {
        log_error("Unable to write screenshot to:", filename, 0);
        image_free();
        return;
    }

    if (!image_write_canvas()) {
        log_error("Error writing image", 0, 0);
        image_free();
        return;
    }

    image_finish();
    log_info("Saved screenshot:", filename, 0);
    image_free();
}

static void create_full_city_screenshot(void)
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

    if (!image_create(city_width_pixels, city_height_pixels + TILE_Y_SIZE, IMAGE_HEIGHT_CHUNK)) {
        log_error("Unable to set memory for full city screenshot", 0, 0);
        return;
    }
    const char *filename = generate_filename(FULL_CITY_SCREENSHOT);
    if (!image_begin_io(filename) || !image_write_header()) {
        log_error("Unable to write screenshot to:", filename, 0);
        image_free();
        return;
    }

    int canvas_width = city_width_pixels + (city_view_is_sidebar_collapsed() ? 40 : 160);
    int zoom_active = config_get(CONFIG_UI_ZOOM);
    int old_scale = 100;
    if (zoom_active) {
        old_scale = city_view_get_scale();
        city_view_set_scale(100);
        config_set(CONFIG_UI_ZOOM, 0);
    }
    screen_set_resolution(canvas_width, TOP_MENU_HEIGHT + IMAGE_HEIGHT_CHUNK);
    graphics_set_clip_rectangle(0, TOP_MENU_HEIGHT, city_width_pixels, IMAGE_HEIGHT_CHUNK);

    int base_width = (GRID_SIZE * TILE_X_SIZE - city_width_pixels) / 2 + TILE_X_SIZE;
    int max_height = (GRID_SIZE * TILE_Y_SIZE + city_height_pixels) / 2;
    int min_height = max_height - city_height_pixels - TILE_Y_SIZE;
    map_tile dummy_tile = { 0, 0, 0 };
    int error = 0;
    int current_height = image_set_loop_height_limits(min_height, max_height);
    int size;
    const color_t *canvas = (color_t *) graphics_canvas(CANVAS_UI) + TOP_MENU_HEIGHT * canvas_width;
    while ((size = image_request_rows())) {
        city_view_set_camera_from_pixel_position(base_width, current_height);
        city_without_overlay_draw(0, 0, &dummy_tile);
        if (!image_write_rows(canvas, canvas_width)) {
            log_error("Error writing image", 0, 0);
            error = 1;
            break;
        }
        current_height += size;
    }
    if (zoom_active) {
        config_set(CONFIG_UI_ZOOM, 1);
        city_view_set_scale(old_scale);
    }
    screen_set_resolution(width, height);
    city_view_set_camera_from_pixel_position(original_camera_pixels.x, original_camera_pixels.y);
    if (!error) {
        image_finish();
        log_info("Saved full city screenshot:", filename, 0);
    }
    image_free();
}

void graphics_save_screenshot(int full_city)
{
    if (full_city) {
        create_full_city_screenshot();
    } else {
        create_window_screenshot();
    }
}
