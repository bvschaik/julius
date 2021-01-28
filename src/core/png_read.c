#include "core/png_read.h"

#include "core/dir.h"
#include "core/file.h"
#include "core/log.h"

#include "png.h"

#include <stdio.h>
#include <stdlib.h>

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

int png_read(const char *path, uint8_t *pixels)
{
    if (!load_png(path)) {
        return 0;
    }
    if (setjmp(png_jmpbuf(data.png_ptr))) {
        log_error("Unable to read png file", 0, 0);
        unload_png();
        return 0;
    }
    png_set_gray_to_rgb(data.png_ptr);
    png_set_filler(data.png_ptr, 0xFF, PNG_FILLER_AFTER);
    png_set_expand(data.png_ptr);
    png_set_strip_16(data.png_ptr);
    png_set_bgr(data.png_ptr);

    png_set_interlace_handling(data.png_ptr);
    png_read_update_info(data.png_ptr, data.info_ptr);

    int width = png_get_image_width(data.png_ptr, data.info_ptr);
    int height = png_get_image_height(data.png_ptr, data.info_ptr);
    png_bytep *row_pointers = malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; ++y) {
        row_pointers[y] = pixels + y * width * sizeof(color_t);
    }

    png_read_image(data.png_ptr, row_pointers);
    free(row_pointers);

    unload_png();
    return 1;
}
