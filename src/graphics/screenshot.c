#include "screenshot.h"

#include "core/buffer.h"
#include "core/file.h"
#include "core/log.h"
#include "graphics/screen.h"
#include "graphics/graphics.h"

#include "platform/vita.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HEADER_SIZE 26

static const char filename_format[] = "city %Y-%m-%d %H.%M.%S.bmp";

static const char *generate_filename(void)
{
    static char filename[100];
    time_t curtime = time(NULL);
    struct tm *loctime = localtime(&curtime);

    strftime(filename, 100, filename_format, loctime);

    return filename;
}

static void write_bmp_header(buffer *buf, int width, int height, int scanline_size)
{
    buffer_write_i8(buf, 'B');
    buffer_write_i8(buf, 'M');
    buffer_write_i32(buf, HEADER_SIZE + scanline_size * height); // file size
    buffer_write_i32(buf, 0); // reserved
    buffer_write_i32(buf, HEADER_SIZE); // data offset
    buffer_write_i32(buf, 12); // dib size
    buffer_write_i16(buf, (int16_t) width);
    buffer_write_i16(buf, (int16_t) height);
    buffer_write_i16(buf, 1); // planes
    buffer_write_i16(buf, 24); // bpp
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

void graphics_save_screenshot(void)
{
    uint8_t header[HEADER_SIZE];
    buffer buf;
    buffer_init(&buf, header, HEADER_SIZE);

    int width = screen_width();
    int height = screen_height();
    const color_t *canvas = graphics_canvas();

    int scanline_padding = 4 - (width * 3) % 4;
    int scanline_size = width * 3 + scanline_padding;
    uint8_t *pixels = (uint8_t*) malloc(scanline_size);
    if (!pixels) {
        return;
    }
    memset(pixels, 0, scanline_size);

    const char *filename = generate_filename();
    FILE *fp = file_open(filename, "wb");
    if (!fp) {
        log_error("Unable to write screenshot to:", filename, 0);
        free(pixels);
        return;
    }

    write_bmp_header(&buf, width, height, scanline_size);
    fwrite(header, 1, HEADER_SIZE, fp);
    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            pixel(canvas[y * width + x],
                &pixels[3*x+2], &pixels[3*x+1], &pixels[3*x]);
        }
        fwrite(pixels, 1, scanline_size, fp);
    }
    file_close(fp);
    free(pixels);

    log_info("Saved screenshot:", filename, 0);
}
