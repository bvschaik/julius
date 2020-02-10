#include "screenshot.h"

#include "city/view.h"
#include "core/buffer.h"
#include "core/config.h"
#include "core/file.h"
#include "core/log.h"
#include "graphics/screen.h"
#include "graphics/graphics.h"
#include "graphics/window.h"
#include "map/grid.h"
#include "widget/city_without_overlay.h"

#include "png/png.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TOP_MENU_HEIGHT 24
#define HEADER_SIZE 26
#define TILE_X_SIZE 60
#define TILE_Y_SIZE 30
#define IMAGE_HEIGHT_CHUNK TILE_Y_SIZE
#define IMAGE_BITS_PER_PIXEL 24
#define IMAGE_BYTES_PER_PIXEL (IMAGE_BITS_PER_PIXEL / 8)

enum {
    FULL_CITY_SCREENSHOT = 0,
    DISPLAY_SCREENSHOT = 1,
    MAX_SCREENSHOT_TYPES = 2
};

enum {
    IMAGE_FORMAT_BMP = 0,
    IMAGE_FORMAT_PNG = 1,
    MAX_IMAGE_FORMATS = 2
};

static const char filename_formats[MAX_SCREENSHOT_TYPES][MAX_IMAGE_FORMATS][32] = {
    {
        "full city %Y-%m-%d %H.%M.%S.bmp",
        "full city %Y-%m-%d %H.%M.%S.png",
    },
    {
        "city %Y-%m-%d %H.%M.%S.bmp",
        "city %Y-%m-%d %H.%M.%S.png",
    },
};

static void full_city_screenshot(void);

static struct {
    int format;
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
    image.format = config_get(CONFIG_MISC_SCREENSHOT_PNG_FORMAT) ? IMAGE_FORMAT_PNG : IMAGE_FORMAT_BMP;
    int row_padding = 0;
    if (image.format != IMAGE_FORMAT_PNG) {
        row_padding = 4 - (width * IMAGE_BYTES_PER_PIXEL) % 4;
        if (row_padding == 4) {
            row_padding = 0;
        }
    } else {
        image.png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
        if(!image.png_ptr) {
            return 0;
        }
        image.info_ptr = png_create_info_struct(image.png_ptr);
        if (!image.info_ptr) {
            image_free();
            return 0;
        }
        png_set_compression_level(image.png_ptr, 3);
    }
    image.width = width;
    image.height = height;
    image.row_size = width * IMAGE_BYTES_PER_PIXEL + row_padding;
    image.rows_in_memory = rows_in_memory;
    image.pixels = (uint8_t *) malloc(image.row_size * rows_in_memory);
    if (!image.pixels) {
        image_free();
        return 0;
    }
    memset(image.pixels, 0, image.row_size * rows_in_memory);
    return 1;
}

static const char *generate_filename(int city_screenshot)
{
    static char filename[FILE_NAME_MAX];
    time_t curtime = time(NULL);
    struct tm *loctime = localtime(&curtime);
    strftime(filename, FILE_NAME_MAX, filename_formats[city_screenshot][image.format], loctime);
    return filename;
}

int image_begin_io(const char *filename)
{
    FILE *fp = file_open(filename, "wb");
    if (!fp) {
        return 0;
    }
    image.fp = fp;
    if (image.format == IMAGE_FORMAT_PNG) {
        png_init_io(image.png_ptr, fp);
    }
    return 1;
}

static void write_bmp_header(void)
{
    uint8_t header[HEADER_SIZE];
    buffer header_buf;
    buffer *buf = &header_buf;
    buffer_init(buf, header, HEADER_SIZE);
    buffer_write_i8(buf, 'B');
    buffer_write_i8(buf, 'M');
    buffer_write_i32(buf, HEADER_SIZE + image.row_size * image.height); // file size
    buffer_write_i32(buf, 0); // reserved
    buffer_write_i32(buf, HEADER_SIZE); // data offset
    buffer_write_i32(buf, 12); // dib size
    buffer_write_i16(buf, (int16_t) image.width);
    buffer_write_i16(buf, (int16_t) image.height);
    buffer_write_i16(buf, 1); // planes
    buffer_write_i16(buf, IMAGE_BITS_PER_PIXEL);
    fwrite(header, 1, HEADER_SIZE, image.fp);
}

static int image_write_header(void)
{
    if (image.format == IMAGE_FORMAT_PNG) {
        if (setjmp(png_jmpbuf(image.png_ptr))) {
            return 0;
        }
        png_set_IHDR(image.png_ptr, image.info_ptr, image.width, image.height, 8, PNG_COLOR_TYPE_RGB,
                     PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(image.png_ptr, image.info_ptr);
    } else {
        write_bmp_header();
    }
    return 1;
}

static int image_set_loop_height_limits(int min, int max)
{
    image.current_y = min;
    image.final_y = max;
    if (min > max && image.format != IMAGE_FORMAT_PNG) {
        return min - image.rows_in_memory;
    }
    if (min > max || image.format != IMAGE_FORMAT_PNG) {
        image.current_y = max;
        image.final_y = min;
        return max - image.rows_in_memory;
    }
    return min;
}

static int image_request_rows(void)
{
    if (image.current_y != image.final_y) {
        int step = image.format == IMAGE_FORMAT_PNG ? image.rows_in_memory : -image.rows_in_memory;
        image.current_y += step;
        return step;
    }
    return 0;
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

static int image_write_rows(const color_t *canvas, int canvas_width)
{
    if (image.format == IMAGE_FORMAT_PNG) {
        if (setjmp(png_jmpbuf(image.png_ptr))) {
            return 0;
        }
        for (int y = 0; y < image.rows_in_memory; ++y) {
            for (int x = 0; x < image.width; x++) {
                pixel(canvas[y * canvas_width + x],
                    &image.pixels[3 * x], &image.pixels[3 * x + 1], &image.pixels[3 * x + 2]);
            }
            png_write_row(image.png_ptr, image.pixels);
        }
    } else {
        for (int y = image.rows_in_memory - 1; y >= 0; y--) {
            for (int x = 0; x < image.width; x++) {
                pixel(canvas[y * canvas_width + x],
                    &image.pixels[3 * x + 2], &image.pixels[3 * x + 1], &image.pixels[3 * x]);
            }
            if (fwrite(image.pixels, sizeof(uint8_t), image.row_size, image.fp) != image.row_size) {
                return 0;
            }
        }
    }
    return 1;
}

static int image_write_canvas(void)
{
    const color_t *canvas = graphics_canvas();
    int current_height = image_set_loop_height_limits(0, image.height);
    int size;
    while ((size = image_request_rows())) {
        if (!image_write_rows(canvas + current_height * image.width, image.width)) {
            return 0;
        }
        current_height += size;
    }
    return 1;
}

static void image_finish(void)
{
    if (image.format == IMAGE_FORMAT_PNG) {
        png_write_end(image.png_ptr, image.info_ptr);
    }
}

void graphics_save_screenshot(int full_city)
{
    if (full_city) {
        full_city_screenshot();
        return;
    }
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
    screen_set_resolution(canvas_width, TOP_MENU_HEIGHT + IMAGE_HEIGHT_CHUNK);
    graphics_set_clip_rectangle(0, TOP_MENU_HEIGHT, city_width_pixels, IMAGE_HEIGHT_CHUNK);

    int base_width = (GRID_SIZE * TILE_X_SIZE - city_width_pixels) / 2 + TILE_X_SIZE;
    int max_height = (GRID_SIZE * TILE_Y_SIZE + city_height_pixels) / 2;
    int min_height = max_height - city_height_pixels - TILE_Y_SIZE;
    map_tile dummy_tile = { 0, 0, 0 };
    int error = 0;
    int current_height = image_set_loop_height_limits(min_height, max_height);
    int size;
    const color_t *canvas = (color_t *) graphics_canvas() + TOP_MENU_HEIGHT * canvas_width;
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
    graphics_reset_clip_rectangle();
    screen_set_resolution(width, height);
    city_view_set_camera_from_pixel_position(original_camera_pixels.x, original_camera_pixels.y);
    if (!error) {
        image_finish();
        log_info("Saved full city screenshot:", filename, 0);
    }
    image_free();
}
