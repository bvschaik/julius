#include "screenshot.h"

#include "city/view.h"
#include "city/warning.h"
#include "core/buffer.h"
#include "core/config.h"
#include "core/file.h"
#include "core/log.h"
#include "core/string.h"
#include "graphics/screen.h"
#include "graphics/graphics.h"
#include "graphics/menu.h"
#include "graphics/renderer.h"
#include "graphics/screen.h"
#include "graphics/window.h"
#include "map/grid.h"
#include "translation/translation.h"
#include "widget/city_without_overlay.h"
#include "widget/minimap.h"

#include "png.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TILE_X_SIZE 60
#define TILE_Y_SIZE 30
#define IMAGE_HEIGHT_CHUNK (TILE_Y_SIZE * 15)
#define IMAGE_BYTES_PER_PIXEL 3
#define MINIMAP_SCALE 2.0f

static const char filename_formats[SCREENSHOT_MAX][32] = {
    "full city %Y-%m-%d %H.%M.%S.png",
    "city %Y-%m-%d %H.%M.%S.png",
    "minimap %Y-%m-%d %H.%M.%S.png",
};

static struct {
    int width;
    int height;
    int row_size;
    int rows_in_memory;
    int current_y;
    int final_y;
    int alpha_channel;
    uint8_t *pixels;
    FILE *fp;
    png_structp png_ptr;
    png_infop info_ptr;
} screenshot;

static void image_free(void)
{
    screenshot.width = 0;
    screenshot.height = 0;
    screenshot.row_size = 0;
    screenshot.rows_in_memory = 0;
    free(screenshot.pixels);
    screenshot.pixels = 0;
    if (screenshot.fp) {
        file_close(screenshot.fp);
        screenshot.fp = 0;
    }
    png_destroy_write_struct(&screenshot.png_ptr, &screenshot.info_ptr);
}

static int image_create(int width, int height, int has_alpha_channel, int rows_in_memory)
{
    image_free();
    if (!width || !height || !rows_in_memory) {
        return 0;
    }
    screenshot.png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!screenshot.png_ptr) {
        return 0;
    }
    screenshot.info_ptr = png_create_info_struct(screenshot.png_ptr);
    if (!screenshot.info_ptr) {
        image_free();
        return 0;
    }
    png_set_compression_level(screenshot.png_ptr, 3);
    screenshot.alpha_channel = has_alpha_channel;
    screenshot.width = width;
    screenshot.height = height;
    screenshot.row_size = width * IMAGE_BYTES_PER_PIXEL;
    if (screenshot.alpha_channel) {
        screenshot.row_size += width;
    }
    screenshot.rows_in_memory = rows_in_memory;
    screenshot.pixels = (uint8_t *) malloc(screenshot.row_size);
    if (!screenshot.pixels) {
        image_free();
        return 0;
    }
    memset(screenshot.pixels, 0, screenshot.row_size);
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

static int image_begin_io(const char *filename)
{
    FILE *fp = file_open(filename, "wb");
    if (!fp) {
        return 0;
    }
    screenshot.fp = fp;
    png_init_io(screenshot.png_ptr, fp);
    return 1;
}

static int image_write_header(void)
{
    if (setjmp(png_jmpbuf(screenshot.png_ptr))) {
        return 0;
    }
    int color_type = screenshot.alpha_channel ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB;
    png_set_IHDR(screenshot.png_ptr, screenshot.info_ptr, screenshot.width, screenshot.height, 8, color_type,
                    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(screenshot.png_ptr, screenshot.info_ptr);
    return 1;
}

static int image_set_loop_height_limits(int min, int max)
{
    screenshot.current_y = min;
    screenshot.final_y = max;
    return screenshot.current_y;
}

static int image_request_rows(void)
{
    if (screenshot.current_y < screenshot.final_y) {
        screenshot.current_y += screenshot.rows_in_memory;
        return screenshot.rows_in_memory;
    }
    return 0;
}

static int image_write_rows(const color_t *canvas, int canvas_width)
{
    if (setjmp(png_jmpbuf(screenshot.png_ptr))) {
        return 0;
    }
    for (int y = 0; y < screenshot.rows_in_memory; ++y) {
        uint8_t *pixel = screenshot.pixels;
        if (screenshot.alpha_channel) {
            for (int x = 0; x < screenshot.width; x++) {
                color_t input = canvas[y * canvas_width + x];
                *(pixel + 0) = (uint8_t) COLOR_COMPONENT(input, COLOR_BITSHIFT_RED);
                *(pixel + 1) = (uint8_t) COLOR_COMPONENT(input, COLOR_BITSHIFT_GREEN);
                *(pixel + 2) = (uint8_t) COLOR_COMPONENT(input, COLOR_BITSHIFT_BLUE);
                *(pixel + 3) = (uint8_t) COLOR_COMPONENT(input, COLOR_BITSHIFT_ALPHA);
                pixel += IMAGE_BYTES_PER_PIXEL + 1;
            }
        } else {
            for (int x = 0; x < screenshot.width; x++) {
                color_t input = canvas[y * canvas_width + x];
                *(pixel + 0) = (uint8_t) COLOR_COMPONENT(input, COLOR_BITSHIFT_RED);
                *(pixel + 1) = (uint8_t) COLOR_COMPONENT(input, COLOR_BITSHIFT_GREEN);
                *(pixel + 2) = (uint8_t) COLOR_COMPONENT(input, COLOR_BITSHIFT_BLUE);
                pixel += IMAGE_BYTES_PER_PIXEL;
            }
        }
        png_write_row(screenshot.png_ptr, screenshot.pixels);
    }
    return 1;
}

static int image_write_canvas(void)
{
    const color_t *canvas;
    color_t *buffer = 0;
    buffer = malloc(sizeof(color_t) * screenshot.width * screenshot.height);
    if (!graphics_renderer()->save_screen_buffer(buffer, 0, 0, screen_width(), screen_height(), screen_width())) {
        free(buffer);
        return 0;
    }
    canvas = buffer;
    int current_height = image_set_loop_height_limits(0, screenshot.height);
    int size;
    while ((size = image_request_rows())) {
        if (!image_write_rows(canvas + current_height * screenshot.width, screenshot.width)) {
            free(buffer);
            return 0;
        }
        current_height += size;
    }
    free(buffer);
    return 1;
}

static void image_finish(void)
{
    png_write_end(screenshot.png_ptr, screenshot.info_ptr);
}

static void show_saved_notice(const char *filename)
{
    uint8_t notice_text[FILE_NAME_MAX];
    const uint8_t *prefix = translation_for(TR_WARNING_SCREENSHOT_SAVED);
    string_copy(prefix, notice_text, FILE_NAME_MAX);
    int prefix_length = string_length(prefix);
    string_copy(string_from_ascii(filename), &notice_text[prefix_length], FILE_NAME_MAX - prefix_length);

    city_warning_show_custom(notice_text, 0);
}

static void create_window_screenshot(void)
{
    int width = screen_width();
    int height = screen_height();

    if (!image_create(width, height, 0, 1)) {
        log_error("Unable to create memory for screenshot", 0, 0);
        return;
    }

    const char *filename = generate_filename(SCREENSHOT_DISPLAY);
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
    show_saved_notice(filename);
    image_free();
}

static void create_full_city_screenshot(void)
{
    if (!window_is(WINDOW_CITY) && !window_is(WINDOW_CITY_MILITARY)) {
        return;
    }
    pixel_offset original_camera_pixels;
    city_view_get_camera_in_pixels(&original_camera_pixels.x, &original_camera_pixels.y);

    int city_width_pixels = map_grid_width() * TILE_X_SIZE;
    int city_height_pixels = map_grid_height() * TILE_Y_SIZE;

    if (!image_create(city_width_pixels, city_height_pixels + TILE_Y_SIZE, 0, IMAGE_HEIGHT_CHUNK)) {
        log_error("Unable to set memory for full city screenshot", 0, 0);
        return;
    }
    const char *filename = generate_filename(SCREENSHOT_FULL_CITY);
    if (!image_begin_io(filename) || !image_write_header()) {
        log_error("Unable to write screenshot to:", filename, 0);
        image_free();
        return;
    }

    color_t *canvas = malloc(sizeof(color_t) * city_width_pixels * IMAGE_HEIGHT_CHUNK);
    if (!canvas) {
        image_free();
        return;
    }
    memset(canvas, 0, sizeof(color_t) * city_width_pixels * IMAGE_HEIGHT_CHUNK);

    int canvas_width = 8 * TILE_X_SIZE;
    int old_scale = city_view_get_scale();

    int draw_cloud_shadows = config_get(CONFIG_UI_DRAW_CLOUD_SHADOWS);
    config_set(CONFIG_UI_DRAW_CLOUD_SHADOWS, 0);

    int min_width = (GRID_SIZE * TILE_X_SIZE - city_width_pixels) / 2 + TILE_X_SIZE;
    int max_height = (GRID_SIZE * TILE_Y_SIZE + city_height_pixels) / 2;
    int min_height = max_height - city_height_pixels - TILE_Y_SIZE;
    map_tile dummy_tile = {0, 0, 0};
    int error = 0;
    int base_height = image_set_loop_height_limits(min_height, max_height);
    int size;
    city_view_set_scale(100);
    graphics_set_clip_rectangle(0, TOP_MENU_HEIGHT, canvas_width, IMAGE_HEIGHT_CHUNK);
    int viewport_x, viewport_y, viewport_width, viewport_height;
    city_view_get_viewport(&viewport_x, &viewport_y, &viewport_width, &viewport_height);
    city_view_set_viewport(canvas_width + (city_view_is_sidebar_collapsed() ? 42 : 162),
        IMAGE_HEIGHT_CHUNK + TOP_MENU_HEIGHT);
    int current_height = base_height;
    while ((size = image_request_rows())) {
        int y_offset = current_height + IMAGE_HEIGHT_CHUNK > max_height ?
            IMAGE_HEIGHT_CHUNK - (max_height - current_height) - TILE_Y_SIZE: 0;
        for (int width = 0; width < city_width_pixels; width += canvas_width) {
            int image_section_width = canvas_width;
            int x_offset = 0;
            if (canvas_width + width > city_width_pixels) {
                image_section_width = city_width_pixels - width;
                x_offset = canvas_width - image_section_width - TILE_X_SIZE * 2;
            }
            city_view_set_camera_from_pixel_position(min_width + width, current_height);
            city_without_overlay_draw(0, 0, &dummy_tile);
            graphics_renderer()->save_screen_buffer(&canvas[width], x_offset, TOP_MENU_HEIGHT + y_offset,
                image_section_width, IMAGE_HEIGHT_CHUNK - y_offset, city_width_pixels);
        }
        if (!image_write_rows(canvas, city_width_pixels)) {
            log_error("Error writing image", 0, 0);
            error = 1;
            break;
        }
        current_height += IMAGE_HEIGHT_CHUNK;
    }
    city_view_set_viewport(viewport_width + (city_view_is_sidebar_collapsed() ? 42 : 162), viewport_height + TOP_MENU_HEIGHT);
    city_view_set_scale(old_scale);
    config_set(CONFIG_UI_DRAW_CLOUD_SHADOWS, draw_cloud_shadows);
    graphics_reset_clip_rectangle();
    city_view_set_camera_from_pixel_position(original_camera_pixels.x, original_camera_pixels.y);
    if (!error) {
        image_finish();
        log_info("Saved full city screenshot:", filename, 0);
        show_saved_notice(filename);
    }
    image_free();
    window_invalidate();
}

static void create_minimap_screenshot(void)
{
    if (!window_is(WINDOW_CITY) && !window_is(WINDOW_CITY_MILITARY)) {
        return;
    }

    int width_pixels = map_grid_width() * (int) MINIMAP_SCALE * 2;
    int height_pixels = map_grid_height() * (int) MINIMAP_SCALE * 2;

    if (!image_create(width_pixels, height_pixels, 1, height_pixels)) {
        log_error("Unable to set memory for minimap screenshot", 0, 0);
        return;
    }
    const char *filename = generate_filename(SCREENSHOT_MINIMAP);
    if (!image_begin_io(filename) || !image_write_header()) {
        log_error("Unable to write screenshot to:", filename, 0);
        image_free();
        return;
    }

    color_t *canvas = malloc(sizeof(color_t) * width_pixels * height_pixels);
    if (!canvas) {
        image_free();
        return;
    }
    memset(canvas, 0, sizeof(color_t) * width_pixels * height_pixels);
    widget_minimap_update(0);
    graphics_clear_screen();
    graphics_renderer()->draw_custom_image(CUSTOM_IMAGE_MINIMAP, 0, 0, 1 / MINIMAP_SCALE, 1);
    graphics_renderer()->save_screen_buffer(canvas, 0, 0, width_pixels, height_pixels, width_pixels);
    if (image_write_rows(canvas, width_pixels)) {
        image_finish();
        log_info("Saved city map screenshot:", filename, 0);
        show_saved_notice(filename);
    }
    image_free();
    window_invalidate();
}

void graphics_save_screenshot(int screenshot_type)
{
    switch (screenshot_type) {
        case SCREENSHOT_FULL_CITY:
            create_full_city_screenshot();
            return;
        case SCREENSHOT_DISPLAY:
            create_window_screenshot();
            return;
        case SCREENSHOT_MINIMAP:
            create_minimap_screenshot();
            return;
    }
}
