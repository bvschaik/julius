#include "image.h"

#include "assets/assets.h"
#include "core/buffer.h"
#include "core/file.h"
#include "core/image_packer.h"
#include "core/io.h"
#include "core/log.h"
#include "graphics/font.h"
#include "graphics/renderer.h"

#include <stdlib.h>
#include <string.h>

#define HEADER_SIZE 20680
#define ENTRY_SIZE 64

#define ENEMY_ENTRIES 801
#define CYRILLIC_FONT_ENTRIES 2000
#define BASE_FONT_ENTRIES 1340

#define FONT_STYLES 3

#define MAIN_INDEX_SIZE 660680
#define ENEMY_INDEX_OFFSET HEADER_SIZE
#define ENEMY_INDEX_SIZE ENTRY_SIZE * ENEMY_ENTRIES
#define CYRILLIC_FONT_INDEX_OFFSET HEADER_SIZE
#define CYRILLIC_FONT_INDEX_SIZE ENTRY_SIZE * CYRILLIC_FONT_ENTRIES

#define JAPANESE_HALF_WIDTH_CHARS 63

#define MAIN_DATA_SIZE 12100000
#define ENEMY_DATA_SIZE 2400000
#define CYRILLIC_FONT_DATA_SIZE 1500000
#define TRAD_CHINESE_FONT_DATA_SIZE 7200000
#define KOREAN_FONT_DATA_SIZE 7500000
#define JAPANESE_FONT_DATA_SIZE 11000000

#define CYRILLIC_FONT_BASE_OFFSET 201

#define NAME_SIZE 32

#define IMAGE_TYPE_ISOMETRIC 30

enum {
    NO_EXTRA_FONT = 0,
    FULL_CHARSET_IN_FONT = 1,
    MULTIBYTE_IN_FONT = 2
};

typedef struct {
    int offset;
    int is_compressed;
    int data_length;
    int uncompressed_length;
    int bitmap_id;
    int width;
    int height;
    void *buffer;
} image_draw_data;

typedef struct {
    int width;
    int height;
    int half_width;
} multibyte_font_sizes;

typedef struct {
    const char *name;
    const char *file_v1;
    const char *file_v2;
    int data_size;
    int chars;
    int half_width_chars;
    struct {
        multibyte_font_sizes v1[3];
        multibyte_font_sizes v2[3];
    } sizes;
    int letter_spacing;
} multibyte_font_data;

typedef enum {
    MULTIBYTE_FONT_TRADITIONAL_CHINESE = 0,
    MULTIBYTE_FONT_SIMPLIFIED_CHINESE = 1,
    MULTIBYTE_FONT_KOREAN = 2,
    MULTIBYTE_FONT_JAPANESE = 3,
    MULTIBYTE_FONT_MAX = 4
} multibyte_font_type;

static const int FOOTPRINT_X_START_PER_HEIGHT[] = {
    28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2, 0,
    0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28
};

static const char MAIN_GRAPHICS_SG2[][NAME_SIZE] = {
    "c3.sg2",
    "c3_north.sg2",
    "c3_south.sg2"
};
static const char MAIN_GRAPHICS_555[][NAME_SIZE] = {
    "c3.555",
    "c3_north.555",
    "c3_south.555"
};
static const char EDITOR_GRAPHICS_SG2[][NAME_SIZE] = {
    "c3map.sg2",
    "c3map_north.sg2",
    "c3map_south.sg2"
};
static const char EDITOR_GRAPHICS_555[][NAME_SIZE] = {
    "c3map.555",
    "c3map_north.555",
    "c3map_south.555"
};

static const char CYRILLIC_FONTS_SG2[NAME_SIZE] = "C3_fonts.sg2";
static const char CYRILLIC_FONTS_555[NAME_SIZE] = "C3_fonts.555";
static const char CHINESE_FONTS_555[NAME_SIZE] = "rome.555";
static const char CHINESE_FONTS_555_V2[NAME_SIZE] = "rome-v2.555";
static const char KOREAN_FONTS_555[NAME_SIZE] = "korean.555";
static const char KOREAN_FONTS_555_V2[NAME_SIZE] = "korean-v2.555";
static const char JAPANESE_FONTS_555[NAME_SIZE] = "japanese-v2.555";

static const char ENEMY_GRAPHICS_SG2[][NAME_SIZE] = {
    "goths.sg2",
    "Etruscan.sg2",
    "Etruscan.sg2",
    "carthage.sg2",
    "Greek.sg2",
    "Greek.sg2",
    "egyptians.sg2",
    "Persians.sg2",
    "Phoenician.sg2",
    "celts.sg2",
    "celts.sg2",
    "celts.sg2",
    "Gaul.sg2",
    "Gaul.sg2",
    "goths.sg2",
    "goths.sg2",
    "goths.sg2",
    "Phoenician.sg2",
    "North African.sg2",
    "Phoenician.sg2",
};
static const char ENEMY_GRAPHICS_555[][NAME_SIZE] = {
    "goths.555",
    "Etruscan.555",
    "Etruscan.555",
    "carthage.555",
    "Greek.555",
    "Greek.555",
    "egyptians.555",
    "Persians.555",
    "Phoenician.555",
    "celts.555",
    "celts.555",
    "celts.555",
    "Gaul.555",
    "Gaul.555",
    "goths.555",
    "goths.555",
    "goths.555",
    "Phoenician.555",
    "North African.555",
    "Phoenician.555",
};

static multibyte_font_data multibyte_font_info[MULTIBYTE_FONT_MAX] = {
    {
        .name = "Traditional Chinese",
        .file_v1 = CHINESE_FONTS_555,
        .file_v2 = CHINESE_FONTS_555_V2,
        .data_size = TRAD_CHINESE_FONT_DATA_SIZE,
        .chars = IMAGE_FONT_MULTIBYTE_TRAD_CHINESE_MAX_CHARS,
        .sizes = {
            .v1 = { { 13, 11 }, { 17, 15 }, { 20, 18 } },
            .v2 = { { 13, 12 }, { 16, 15 }, { 21, 20 } }
        },
        .letter_spacing = 1
    },
    {
        .name = "Simplified Chinese",
        .file_v1 = CHINESE_FONTS_555,
        .file_v2 = CHINESE_FONTS_555_V2,
        .data_size = TRAD_CHINESE_FONT_DATA_SIZE,
        .chars = IMAGE_FONT_MULTIBYTE_SIMP_CHINESE_MAX_CHARS,
        .sizes = {
            .v1 = { { 13, 11 }, { 17, 15 }, { 20, 18 } },
            .v2 = { { 13, 12 }, { 16, 15 }, { 21, 20 } }
        },
        .letter_spacing = 1
    },
    {
        .name = "Korean",
        .file_v1 = KOREAN_FONTS_555,
        .file_v2 = KOREAN_FONTS_555_V2,
        .data_size = KOREAN_FONT_DATA_SIZE,
        .chars = IMAGE_FONT_MULTIBYTE_KOREAN_MAX_CHARS,
        .sizes = {
            .v1 = { { 12, 12 }, { 15, 15 }, { 20, 20 } },
            .v2 = { { 12, 12 }, { 15, 15 }, { 20, 20 } },
        }
    },
    {
        .name = "Japanese",
        .file_v2 = JAPANESE_FONTS_555,
        .data_size = JAPANESE_FONT_DATA_SIZE,
        .chars = IMAGE_FONT_MULTIBYTE_JAPANESE_MAX_CHARS,
        .half_width_chars = JAPANESE_HALF_WIDTH_CHARS,
        .sizes = {
            .v2 = { { 13, 12, 7 }, { 16, 15, 9 }, { 21, 20, 11 } }
        },
        .letter_spacing = 1
    }
};

static const image DUMMY_IMAGE;

static struct {
    int current_climate;
    int current_enemy;

    int is_editor;
    int fonts_enabled;
    int font_base_offset;

    uint16_t group_image_ids[IMAGE_MAX_GROUPS];
    char bitmaps[100][200];
    image main[IMAGE_MAIN_ENTRIES];
    image enemy[ENEMY_ENTRIES];
    image *font;
    image_draw_data *external_draw_data;

    int external_image_id;
    int total_external_images;
    int images_with_tops;

    image_packer packer;
    int max_image_width;
    int max_image_height;
} data;

static void read_header(buffer *buf)
{
    buffer_skip(buf, 80); // header integers
    for (int i = 0; i < IMAGE_MAX_GROUPS; i++) {
        data.group_image_ids[i] = buffer_read_u16(buf);
    }
    buffer_read_raw(buf, data.bitmaps, 20000);
}

static void read_index_entry(buffer *buf, image *img, image_draw_data *draw_data)
{
    draw_data->offset = buffer_read_i32(buf);
    draw_data->data_length = buffer_read_i32(buf);
    draw_data->uncompressed_length = buffer_read_i32(buf);
    buffer_skip(buf, 8);
    img->original.width = img->width = buffer_read_u16(buf);
    img->original.height = img->height = buffer_read_u16(buf);
    buffer_skip(buf, 6);
    int num_sprites = buffer_read_u16(buf);
    if (num_sprites) {
        img->animation = malloc(sizeof(image_animation));
        if (!img->animation) {
            log_error("Not enough memory to add animations. The game will probably crash.", 0, 0);
            buffer_skip(buf, 18);
        } else {
            memset(img->animation, 0, sizeof(image_animation));
            img->animation->num_sprites = num_sprites;
            buffer_skip(buf, 2);
            img->animation->sprite_offset_x = buffer_read_i16(buf);
            img->animation->sprite_offset_y = buffer_read_i16(buf);
            buffer_skip(buf, 10);
            img->animation->can_reverse = buffer_read_i8(buf);
            buffer_skip(buf, 1); 
        }
    } else {
        buffer_skip(buf, 18);
    }
    img->is_isometric = buffer_read_u8(buf) == IMAGE_TYPE_ISOMETRIC;
    draw_data->is_compressed = buffer_read_i8(buf);
    int is_external = buffer_read_i8(buf);
    if (is_external) {
        img->atlas.id = (ATLAS_EXTERNAL << IMAGE_ATLAS_BIT_OFFSET) + data.total_external_images;
        data.total_external_images++;
    }
    int has_top = buffer_read_i8(buf);
    if (has_top && img->is_isometric) {
        img->top = malloc(sizeof(image));
        if (!img->top) {
            log_error("Not enough memory to add animations. The game will probably crash.", 0, 0);
        }
        memset(img->top, 0, sizeof(image));
    }
    buffer_skip(buf, 2);
    draw_data->bitmap_id = buffer_read_u8(buf);
    buffer_skip(buf, 1);
    if (img->animation) {
        img->animation->speed_id = buffer_read_u8(buf);
    } else {
        buffer_skip(buf, 1);
    }
    buffer_skip(buf, 5);
}

static int prepare_images(buffer *buf, image *images, image_draw_data *draw_datas, int size, atlas_type image_type)
{
    for (int i = 0; i < size; i++) {
        image *img = &images[i];
        read_index_entry(buf, img, &draw_datas[i]);
        if (!image_is_external(&images[i]) && img->top) {
            int tiles = (img->width + 2) / (FOOTPRINT_WIDTH + 2);
            int footprint_height = FOOTPRINT_HEIGHT * tiles;
            img->top->height = img->height - footprint_height / 2;
            img->top->width = img->width;
            img->height = footprint_height;
            data.images_with_tops++;
        }
    }
    if (image_type == ATLAS_MAIN) {
        data.external_draw_data = malloc(data.total_external_images * sizeof(image_draw_data));
        if (!data.external_draw_data) {
            return 0;
        }
        memset(data.external_draw_data, 0, data.total_external_images * sizeof(image_draw_data));
    }
    return 1;
}

static void convert_compressed(buffer *buf, int width, int x_offset, int y_offset,
    int buf_length, color_t *dst, int dst_width);

static int crop_and_pack_images(buffer *buf, image *images, image_draw_data *draw_datas,
    int num_images, atlas_type type)
{
    if (image_packer_init(&data.packer, num_images + data.images_with_tops,
            data.max_image_width, data.max_image_height) != IMAGE_PACKER_OK) {
        return 0;
    }
    data.packer.options.fail_policy = IMAGE_PACKER_NEW_IMAGE;
    data.packer.options.reduce_image_size = 1;
    data.packer.options.sort_by = IMAGE_PACKER_SORT_BY_AREA;

    int offset = 4;
    for (int i = 1, rect = 1; i < num_images; i++, rect++) {
        image *img = &images[i];
        image_draw_data *draw_data = &draw_datas[i];

        if (image_is_external(img)) {
            image_draw_data *external_data = &data.external_draw_data[img->atlas.id & IMAGE_ATLAS_BIT_MASK];
            memcpy(external_data, draw_data, sizeof(image_draw_data));
            if (!external_data->offset) {
                external_data->offset = 1;
            }
            external_data->width = img->original.width;
            external_data->height = img->original.height;
            continue;
        }
        draw_data->offset = offset;
        offset += draw_data->data_length;

        // Don't load original placeholder images
        if (type == ATLAS_MAIN && i >= 6145 && i <= 6192) {
            continue;
        }
        if (!img->is_isometric && draw_data->is_compressed) {
            draw_data->buffer = malloc(img->width * img->height * sizeof(color_t));
            if (draw_data->buffer) {
                memset(draw_data->buffer, 0, img->width * img->height * sizeof(color_t));
                buffer_set(buf, draw_data->offset);
                convert_compressed(buf, img->width, 0, 0, draw_data->data_length, draw_data->buffer, img->width);
                image_crop(img, draw_data->buffer);
            }
        }
        data.packer.rects[rect].input.width = img->width;
        data.packer.rects[rect].input.height = img->height;
        if (img->top) {
            draw_data->buffer = malloc(img->top->width * img->top->height * sizeof(color_t));
            if (draw_data->buffer) {
                img->top->original.width = img->top->width;
                img->top->original.height = img->top->height;
                memset(draw_data->buffer, 0, img->top->width * img->top->height * sizeof(color_t));
                buffer_set(buf, draw_data->offset + draw_data->uncompressed_length);
                convert_compressed(buf, img->top->width, 0, 0,
                    draw_data->data_length - draw_data->uncompressed_length, draw_data->buffer, img->top->width);
                image_crop(img->top, draw_data->buffer);
                if (!img->top->height) {
                    free(img->top);
                    img->top = 0;
                } else {
                    rect++;
                    data.packer.rects[rect].input.width = img->top->width;
                    data.packer.rects[rect].input.height = img->top->height;
                }
            }
        }
    }

    image_packer_pack(&data.packer);

    for (int i = 0, rect = 0; i < num_images; i++, rect++) {
        image *img = &images[i];
        if (image_is_external(img)) {
            continue;
        }
        img->atlas.id = (type << IMAGE_ATLAS_BIT_OFFSET) + data.packer.rects[rect].output.image_index;
        img->atlas.x_offset = data.packer.rects[rect].output.x;
        img->atlas.y_offset = data.packer.rects[rect].output.y;
        if (img->top) {
            rect++;
            img->top->atlas.id = (type << IMAGE_ATLAS_BIT_OFFSET) + data.packer.rects[rect].output.image_index;
            img->top->atlas.x_offset = data.packer.rects[rect].output.x;
            img->top->atlas.y_offset = data.packer.rects[rect].output.y;
        }
    }
    return 1;
}

static color_t to_32_bit(uint16_t c)
{
    return ALPHA_OPAQUE |
           ((c & 0x7c00) << 9) | ((c & 0x7000) << 4) |
           ((c & 0x3e0) << 6)  | ((c & 0x380) << 1) |
           ((c & 0x1f) << 3)   | ((c & 0x1c) >> 2);
}

static void convert_uncompressed(buffer *buf, int width, int height, int x_offset, int y_offset,
    color_t *dst, int dst_width)
{
    for (int y = 0; y < height; y++) {
        color_t *pixel = &dst[(y_offset + y) * dst_width + x_offset];
        for (int x = 0; x < width; x++) {
            color_t color = to_32_bit(buffer_read_u16(buf));
            pixel[x] = color == COLOR_SG2_TRANSPARENT ? ALPHA_TRANSPARENT : color;
        }
    }
}

static void copy_compressed(const image *img, image_draw_data *draw_data, color_t *dst, int dst_width)
{
    for (int y = 0; y < img->height; y++) {
        memcpy(&dst[(img->atlas.y_offset + y) * dst_width + img->atlas.x_offset],
            &((color_t *) draw_data->buffer)[(y + img->y_offset) * img->original.width + img->x_offset],
            img->width * sizeof(color_t));
    }
}

static void convert_compressed(buffer *buf, int width, int x_offset, int y_offset,
    int buf_length, color_t *dst, int dst_width)
{
    int y = 0;
    int x = 0;
    while (buf_length > 0) {
        int control = buffer_read_u8(buf);
        if (control == 255) {
            // next byte = transparent pixels to skip
            int skip = buffer_read_u8(buf);
            y += skip / width;
            x += skip % width;
            if (x >= width) {
                y++;
                x -= width;
            }
            buf_length -= 2;
        } else {
            // control = number of concrete pixels
            for (int i = 0; i < control; i++) {
                dst[(y + y_offset) * dst_width + x_offset + x] = to_32_bit(buffer_read_u16(buf));
                x++;
                if (x >= width) {
                    y++;
                    x -= width;
                }
            }
            buf_length -= control * 2 + 1;
        }
    }
}

static void convert_footprint_tile(buffer *buf, const image *img,
    int x_offset, int y_offset, color_t *dst, int dst_width)
{
    for (int y = 0; y < FOOTPRINT_HEIGHT; y++) {
        int x_start = FOOTPRINT_X_START_PER_HEIGHT[y];
        int x_max = FOOTPRINT_WIDTH - x_start;
        for (int x = x_start; x < x_max; x++) {
            dst[(y + y_offset + img->atlas.y_offset) * dst_width + img->atlas.x_offset + x + x_offset] =
                to_32_bit(buffer_read_u16(buf));
        }
    }
}

static void convert_isometric_footprint(buffer *buf, const image *img, color_t *dst, int dst_width)
{
    int num_tiles = (img->width + 2) / (FOOTPRINT_WIDTH + 2);
    int x_start = (num_tiles - 1) * 30;

    for (int i = 0; i < num_tiles; i++) {
        int x = -30 * i + x_start;
        int y = FOOTPRINT_HALF_HEIGHT * i;
        for (int j = 0; j <= i; j++) {
            convert_footprint_tile(buf, img, x, y, dst, dst_width);
            x += 60;
        }
    }
    for (int i = num_tiles - 2; i >= 0; i--) {
        int x = -30 * i + x_start;
        int y = FOOTPRINT_HALF_HEIGHT * (num_tiles * 2 - i - 2);
        for (int j = 0; j <= i; j++) {
            convert_footprint_tile(buf, img, x, y, dst, dst_width);
            x += 60;
        }
    }
}

static void convert_images(image *images, image_draw_data *draw_datas, int size, buffer *buf,
    const image_atlas_data *atlas_data)
{
    for (int i = 0; i < size; i++) {
        image *img = &images[i];
        image_draw_data *draw_data = &draw_datas[i];
        if (image_is_external(img)) {
            continue;
        }
        // Don't load original placeholder images
        if (atlas_data->type == ATLAS_MAIN && i >= 6145 && i <= 6192) {
            continue;
        }
        buffer_set(buf, draw_data->offset);
        color_t *dst = atlas_data->buffers[img->atlas.id & IMAGE_ATLAS_BIT_MASK];
        int dst_width = atlas_data->image_widths[img->atlas.id & IMAGE_ATLAS_BIT_MASK];
        if (draw_data->is_compressed) {
            if (draw_data->buffer) {
                copy_compressed(img, draw_data, dst, dst_width);
                free(draw_data->buffer);
                draw_data->buffer = 0;
            } else {
                convert_compressed(buf, img->width, img->atlas.x_offset, img->atlas.y_offset,
                    draw_data->data_length, dst, dst_width);
            }
        } else if (img->is_isometric) {
            convert_isometric_footprint(buf, img, dst, dst_width);
            if (img->top) {
                color_t *dst_top = atlas_data->buffers[img->top->atlas.id & IMAGE_ATLAS_BIT_MASK];
                int dst_width_top = atlas_data->image_widths[img->top->atlas.id & IMAGE_ATLAS_BIT_MASK];
                copy_compressed(img->top, draw_data, dst_top, dst_width_top);
            }
        } else {
            convert_uncompressed(buf, img->width, img->height, img->atlas.x_offset, img->atlas.y_offset,
                dst, dst_width);
        }
    }
}

static void make_font_white(const image *img, const image_atlas_data *atlas_data)
{
    color_t *pixels = atlas_data->buffers[img->atlas.id & IMAGE_ATLAS_BIT_MASK];
    int width = atlas_data->image_widths[img->atlas.id & IMAGE_ATLAS_BIT_MASK];
    pixels += img->atlas.y_offset * width + img->atlas.x_offset;
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            if ((pixels[x] & COLOR_CHANNEL_ALPHA) != ALPHA_TRANSPARENT) {
                pixels[x] |= 0x00ffffff;
            }
        }
        pixels += width;
    }
}

static void make_plain_fonts_white(const image *img_info, const image_atlas_data *atlas_data, int start_offset)
{
    int limit = font_definition_for(FONT_NORMAL_BLACK)->image_offset -
        font_definition_for(FONT_NORMAL_PLAIN)->image_offset;
    for (int i = 0; i < limit; i++) {
        make_font_white(&img_info[i + start_offset], atlas_data);
    }
    int start_font_offset = start_offset + font_definition_for(FONT_LARGE_PLAIN)->image_offset;
    limit = font_definition_for(FONT_LARGE_BLACK)->image_offset -
        font_definition_for(FONT_LARGE_PLAIN)->image_offset;
    for (int i = 0; i < limit; i++) {
        make_font_white(&img_info[i + start_font_offset], atlas_data);
    }
    start_font_offset = start_offset + font_definition_for(FONT_SMALL_PLAIN)->image_offset;
    limit = font_definition_for(FONT_NORMAL_GREEN)->image_offset -
        font_definition_for(FONT_SMALL_PLAIN)->image_offset;
    for (int i = 0; i < limit; i++) {
        make_font_white(&img_info[i + start_font_offset], atlas_data);
    }
}

static void free_draw_data(image_draw_data *draw_datas, int entries)
{
    for (int i = 0; i < entries; i++) {
        free(draw_datas[i].buffer);
    }
    free(draw_datas);
}

static void release_external_buffers(void)
{
    for (int i = 0; i < data.total_external_images; i++) {
        free(data.external_draw_data[i].buffer);
        data.external_draw_data[i].buffer = 0;
    }
}

int image_load_climate(int climate_id, int is_editor, int force_reload, int keep_atlas_buffers)
{
    if (climate_id == data.current_climate && is_editor == data.is_editor && !force_reload &&
        graphics_renderer()->has_image_atlas(ATLAS_MAIN)) {
        return 1;
    }
    graphics_renderer()->get_max_image_size(&data.max_image_width, &data.max_image_height);

    for (int i = 0; i < IMAGE_MAIN_ENTRIES; i++) {
        free(data.main[i].top);
        free(data.main[i].animation);
    }

    release_external_buffers();
    free(data.external_draw_data);
    data.total_external_images = 0;
    data.images_with_tops = 0;

    const char *filename_bmp = is_editor ? EDITOR_GRAPHICS_555[climate_id] : MAIN_GRAPHICS_555[climate_id];
    const char *filename_idx = is_editor ? EDITOR_GRAPHICS_SG2[climate_id] : MAIN_GRAPHICS_SG2[climate_id];
    uint8_t *tmp_data = malloc(MAIN_DATA_SIZE * sizeof(uint8_t));
    image_draw_data *draw_data = malloc((IMAGE_MAIN_ENTRIES + data.images_with_tops) * sizeof(image_draw_data));
    if (!tmp_data || !draw_data ||
        MAIN_INDEX_SIZE != io_read_file_into_buffer(filename_idx, MAY_BE_LOCALIZED, tmp_data, MAIN_INDEX_SIZE)) {
        free(tmp_data);
        free(draw_data);
        return 0;
    }
    memset(data.main, 0, sizeof(data.main));
    memset(draw_data, 0, IMAGE_MAIN_ENTRIES * sizeof(image_draw_data));

    buffer buf;
    buffer_init(&buf, tmp_data, HEADER_SIZE);
    read_header(&buf);
    buffer_init(&buf, &tmp_data[HEADER_SIZE], ENTRY_SIZE * IMAGE_MAIN_ENTRIES);
    if (!prepare_images(&buf, data.main, draw_data, IMAGE_MAIN_ENTRIES, ATLAS_MAIN)) {
        free(tmp_data);
        free(draw_data);
        return 0;
    }

    int data_size = io_read_file_into_buffer(filename_bmp, MAY_BE_LOCALIZED, tmp_data, MAIN_DATA_SIZE);
    if (!data_size) {
        free(tmp_data);
        free_draw_data(draw_data, IMAGE_MAIN_ENTRIES);
        release_external_buffers();
        free(data.external_draw_data);
        data.external_draw_data = 0;
        return 0;
    }

    buffer_init(&buf, tmp_data, data_size);
    if (!crop_and_pack_images(&buf, data.main, draw_data, IMAGE_MAIN_ENTRIES, ATLAS_MAIN)) {
        free(tmp_data);
        free_draw_data(draw_data, IMAGE_MAIN_ENTRIES);
        release_external_buffers();
        free(data.external_draw_data);
        data.external_draw_data = 0;
        return 0;
    }

    const image_atlas_data *atlas_data = graphics_renderer()->prepare_image_atlas(ATLAS_MAIN,
        data.packer.result.images_needed, data.packer.result.last_image_width, data.packer.result.last_image_height);
    if (!atlas_data) {
        image_packer_free(&data.packer);
        free(tmp_data);
        free_draw_data(draw_data, IMAGE_MAIN_ENTRIES);
        release_external_buffers();
        free(data.external_draw_data);
        data.external_draw_data = 0;
        return 0;
    }

    convert_images(data.main, draw_data, IMAGE_MAIN_ENTRIES, &buf, atlas_data);
    free_draw_data(draw_data, IMAGE_MAIN_ENTRIES);
    free(tmp_data);
    make_plain_fonts_white(data.main, atlas_data, image_group(GROUP_FONT));
    if (!keep_atlas_buffers) {
        assets_init(data.is_editor != is_editor, atlas_data->buffers, atlas_data->image_widths);
    }
    graphics_renderer()->create_image_atlas(atlas_data, !keep_atlas_buffers);
    image_packer_free(&data.packer);

    // Fix engineer's post animation offset
    if (!is_editor) {
        data.main[image_group(GROUP_BUILDING_ENGINEERS_POST)].animation->sprite_offset_y += 1;
    }

    data.current_climate = climate_id;
    data.is_editor = is_editor;

    data.images_with_tops = 0;

    return 1;
}

static void free_font_memory(void)
{
    graphics_renderer()->free_image_atlas(ATLAS_FONT);
    free(data.font);
    data.font = 0;
    data.fonts_enabled = NO_EXTRA_FONT;
}

static int alloc_font_memory(int font_entries)
{
    free_font_memory();
    data.font = (image*) malloc(font_entries * sizeof(image));
    if (!data.font) {
        return 0;
    }
    memset(data.font, 0, font_entries * sizeof(image));
    return 1;
}

static int load_cyrillic_fonts(void)
{
    uint8_t *tmp_data = malloc(CYRILLIC_FONT_DATA_SIZE * sizeof(uint8_t));
    image_draw_data *draw_data = malloc(CYRILLIC_FONT_ENTRIES * sizeof(image_draw_data));
    if (!tmp_data || !draw_data || !alloc_font_memory(CYRILLIC_FONT_ENTRIES)) {
        free(tmp_data);
        free_draw_data(draw_data, CYRILLIC_FONT_ENTRIES);
        return 0;
    }
    memset(draw_data, 0, CYRILLIC_FONT_ENTRIES * sizeof(image_draw_data));

    if (CYRILLIC_FONT_INDEX_SIZE != io_read_file_part_into_buffer(CYRILLIC_FONTS_SG2, MAY_BE_LOCALIZED,
        tmp_data, CYRILLIC_FONT_INDEX_SIZE, CYRILLIC_FONT_INDEX_OFFSET)) {
        free_font_memory();
        free(tmp_data);
        free_draw_data(draw_data, CYRILLIC_FONT_ENTRIES);
        return 0;
    }
    buffer buf;
    buffer_init(&buf, tmp_data, CYRILLIC_FONT_INDEX_SIZE);
    if (!prepare_images(&buf, data.font, draw_data, CYRILLIC_FONT_ENTRIES, ATLAS_FONT)) {
        free_font_memory();
        free(tmp_data);
        free_draw_data(draw_data, CYRILLIC_FONT_ENTRIES);
        return 0;
    }

    int data_size = io_read_file_into_buffer(CYRILLIC_FONTS_555, MAY_BE_LOCALIZED, tmp_data, CYRILLIC_FONT_DATA_SIZE);
    if (!data_size) {
        free_font_memory();
        free(tmp_data);
        free_draw_data(draw_data, CYRILLIC_FONT_ENTRIES);
        return 0;
    }

    buffer_init(&buf, tmp_data, data_size);
    if (!crop_and_pack_images(&buf, data.font, draw_data, CYRILLIC_FONT_ENTRIES, ATLAS_FONT)) {
        free_font_memory();
        free(tmp_data);
        free_draw_data(draw_data, CYRILLIC_FONT_ENTRIES);
        return 0;
    }

    const image_atlas_data *atlas_data = graphics_renderer()->prepare_image_atlas(ATLAS_FONT,
        data.packer.result.images_needed, data.packer.result.last_image_width, data.packer.result.last_image_height);
    if (!atlas_data) {
        image_packer_free(&data.packer);
        free_font_memory();
        free(tmp_data);
        free_draw_data(draw_data, CYRILLIC_FONT_ENTRIES);
        return 0;
    }

    convert_images(data.font, draw_data, CYRILLIC_FONT_ENTRIES, &buf, atlas_data);
    free(tmp_data);
    free_draw_data(draw_data, CYRILLIC_FONT_ENTRIES);
    make_plain_fonts_white(data.font, atlas_data, CYRILLIC_FONT_BASE_OFFSET);
    graphics_renderer()->create_image_atlas(atlas_data, 1);
    image_packer_free(&data.packer);

    data.fonts_enabled = FULL_CHARSET_IN_FONT;
    data.font_base_offset = CYRILLIC_FONT_BASE_OFFSET;
    return 1;
}

static int parse_4bit_multibyte_font(buffer *input, color_t *pixels, multibyte_font_sizes *font_size,
    int letter_spacing, int num_chars, int num_half_width, int offset)
{
    int pixel_offset = 0;
    for (int i = 0; i < num_chars; i++) {
        int width = i < num_half_width ? font_size->half_width : font_size->width;
        color_t *letter = &pixels[pixel_offset];
        int x_first_opaque = width;
        int x_last_opaque = -1;
        int y_first_opaque = font_size->height;
        int y_last_opaque = -1;
        for (int row = 0; row < font_size->height; row++) {
            color_t *pixel = &letter[row * width];
            uint8_t bits = 0;
            for (int col = 0; col < font_size->width - letter_spacing; col++) {
                if (col % 2 == 0) {
                    bits = buffer_read_u8(input);
                }
                if (col < width) {
                    uint8_t value = bits & 0xf;
                    if (value != 0) {
                        uint32_t color_value = (value * 16 + value);
                        *pixel = (color_value << COLOR_BITSHIFT_ALPHA) | COLOR_CHANNEL_RGB;
                        if (col < x_first_opaque) {
                            x_first_opaque = col;
                        }
                        if (col > x_last_opaque) {
                            x_last_opaque = col;
                        }
                        if (row < y_first_opaque) {
                            y_first_opaque = row;
                        }
                        y_last_opaque = row;
                    }
                    pixel++;
                }
                bits >>= 4;
            }
        }
        image *img = &data.font[offset + i];
        img->width = x_last_opaque - x_first_opaque + 1;
        img->x_offset = x_first_opaque;
        img->original.width = width;
        img->original.height = font_size->height;
        img->y_offset = y_first_opaque;
        img->height = y_last_opaque - y_first_opaque + 1;

        if (img->width < 0) {
            img->width = 0;
        }
        if (img->height < 0) {
            img->height = 0;
        }

        image_packer_rect *rect = &data.packer.rects[offset + i];
        rect->input.width = img->width;
        rect->input.height = img->height;

        pixel_offset += width * font_size->height;
    }
    return pixel_offset;
}

static int parse_1bit_multibyte_font(buffer *input, color_t *pixels, multibyte_font_sizes *font_size,
    int letter_spacing, int num_chars, int num_half_width, int offset)
{
    int pixel_offset = 0;
    for (int i = 0; i < num_chars; i++) {
        int width = i < num_half_width ? font_size->half_width : font_size->width;
        int bytes_per_row = (width - 1) <= 16 ? 2 : 3;
        color_t *letter = &pixels[pixel_offset];
        int x_first_opaque = width;
        int x_last_opaque = -1;
        int y_first_opaque = font_size->height;
        int y_last_opaque = -1;
        for (int row = 0; row < font_size->height; row++) {
            color_t *pixel = &letter[row * width];
            unsigned int bits = buffer_read_u16(input);
            if (bytes_per_row == 3) {
                bits += buffer_read_u8(input) << 16;
            }
            int prev_set = 0;
            for (int col = 0; col < font_size->width - letter_spacing; col++) {
                int set = bits & 1;
                if (set || prev_set) {
                    *pixel = set ? COLOR_WHITE : ALPHA_FONT_SEMI_TRANSPARENT;
                    if (col < x_first_opaque) {
                        x_first_opaque = col;
                    }
                    if (col > x_last_opaque) {
                        x_last_opaque = col;
                    }
                    if (row < y_first_opaque) {
                        y_first_opaque = row;
                    }
                    y_last_opaque = row;
                }
                pixel++;
                bits >>= 1;
                prev_set = set;
            }
        }
        image *img = &data.font[offset + i];
        img->width = x_last_opaque - x_first_opaque + 1;
        img->x_offset = x_first_opaque;
        img->original.width = width;
        img->original.height = font_size->height;
        img->y_offset = y_first_opaque;
        img->height = y_last_opaque - y_first_opaque + 1;

        if (img->width < 0) {
            img->width = 0;
        }
        if (img->height < 0) {
            img->height = 0;
        }

        image_packer_rect *rect = &data.packer.rects[offset + i];
        rect->input.width = img->width;
        rect->input.height = img->height;

        pixel_offset += width * font_size->height;
    }
    return pixel_offset;
}

static int load_multibyte_font(multibyte_font_type type)
{
    multibyte_font_data *font_info = &multibyte_font_info[type];

    int entries = FONT_STYLES * font_info->chars;

    uint8_t *tmp_data = malloc(font_info->data_size * sizeof(uint8_t));
    if (!tmp_data || !alloc_font_memory(entries)) {
        free(tmp_data);
        return 0;
    }

    log_info("Parsing multibyte font", font_info->name, 0);

    int file_version = 2;
    int data_size = io_read_file_into_buffer(font_info->file_v2, MAY_BE_LOCALIZED, tmp_data, font_info->data_size);
    if (!data_size) {
        if (font_info->file_v1) {
            file_version = 1;
            data_size = io_read_file_into_buffer(font_info->file_v1, MAY_BE_LOCALIZED, tmp_data, font_info->data_size);
        }
        if (!data_size) {
            free_font_memory();
            free(tmp_data);
            log_error("Augustus requires extra files for the characters:", font_info->file_v2, 0);
            return 0;
        }
    }

    buffer input;
    buffer_init(&input, tmp_data, data_size);
    int num_chars = font_info->chars;
    int num_half_width = font_info->half_width_chars;
    int num_full_width = num_chars - num_half_width;

    if (image_packer_init(&data.packer, entries, data.max_image_width, data.max_image_height) != IMAGE_PACKER_OK) {
        free_font_memory();
        free(tmp_data);
        log_error("Internal error loading font", 0, 0);
        return 0;
    }
    data.packer.options.fail_policy = IMAGE_PACKER_NEW_IMAGE;
    data.packer.options.reduce_image_size = 1;
    data.packer.options.sort_by = IMAGE_PACKER_SORT_BY_AREA;

    multibyte_font_sizes *font_sizes;
    int (*parse_multibyte_font)(buffer *input, color_t *pixels, multibyte_font_sizes *font_size,
        int letter_spacing, int num_chars, int num_half_width, int offset);
    if (file_version == 2) {
        font_sizes = font_info->sizes.v2;
        parse_multibyte_font = parse_4bit_multibyte_font;
    } else {
        font_sizes = font_info->sizes.v1;
        parse_multibyte_font = parse_1bit_multibyte_font;
    }

    int font_data_size = (font_sizes[0].width * font_sizes[0].height + font_sizes[1].width * font_sizes[1].height +
        font_sizes[2].width * font_sizes[2].height) * num_full_width * sizeof(color_t);
    font_data_size += (font_sizes[0].half_width * font_sizes[0].height +
        font_sizes[1].half_width * font_sizes[1].height +
        font_sizes[2].half_width * font_sizes[2].height) * num_half_width * sizeof(color_t);

    color_t *font_data = malloc(font_data_size);
    if (!font_data) {
        image_packer_free(&data.packer);
        free_font_memory();
        free(tmp_data);
        return 0;
    }

    memset(font_data, 0, font_data_size);
    color_t *font_offset = font_data;
    for (int i = 0; i < FONT_STYLES; i++) {
        font_offset += parse_multibyte_font(&input, font_offset, &font_sizes[i], font_info->letter_spacing,
            num_chars, num_half_width, num_chars * i);
    }

    image_packer_pack(&data.packer);
    const image_atlas_data *atlas_data = graphics_renderer()->prepare_image_atlas(ATLAS_FONT,
        data.packer.result.images_needed, data.packer.result.last_image_width, data.packer.result.last_image_height);
    if (!atlas_data) {
        free(font_data);
        image_packer_free(&data.packer);
        free_font_memory();
        free(tmp_data);
        return 0;
    }

    color_t *letter = font_data;
    for (int i = 0; i < FONT_STYLES; i++) {
        int offset = i * num_chars;
        int original_height = font_sizes[i].height;
        for (int j = 0; j < num_chars; j++) {
            image *img = &data.font[offset + j];
            image_packer_rect *rect = &data.packer.rects[offset + j];
            img->atlas.id = (ATLAS_FONT << IMAGE_ATLAS_BIT_OFFSET) | rect->output.image_index;
            img->atlas.x_offset = rect->output.x;
            img->atlas.y_offset = rect->output.y;

            color_t *dst = atlas_data->buffers[rect->output.image_index];
            int dst_width = atlas_data->image_widths[rect->output.image_index];

            for (int y = 0; y < img->height; y++) {
                memcpy(&dst[(img->atlas.y_offset + y) * dst_width + img->atlas.x_offset],
                    &letter[(img->y_offset + y) * img->original.width + img->x_offset], img->width * sizeof(color_t));
            }
            letter += original_height * img->original.width;
        }
    }

    graphics_renderer()->create_image_atlas(atlas_data, 1);

    log_info("Done parsing font", font_info->name, 0);

    image_packer_free(&data.packer);
    free(font_data);
    free(tmp_data);
    data.fonts_enabled = MULTIBYTE_IN_FONT;
    data.font_base_offset = 0;
    return 1;
}

int image_load_fonts(encoding_type encoding)
{
    graphics_renderer()->get_max_image_size(&data.max_image_width, &data.max_image_height);

    if (encoding == ENCODING_CYRILLIC) {
        return load_cyrillic_fonts();
    } else if (encoding == ENCODING_TRADITIONAL_CHINESE) {
        return load_multibyte_font(MULTIBYTE_FONT_TRADITIONAL_CHINESE);
    } else if (encoding == ENCODING_SIMPLIFIED_CHINESE) {
        return load_multibyte_font(MULTIBYTE_FONT_SIMPLIFIED_CHINESE);
    } else if (encoding == ENCODING_KOREAN) {
        return load_multibyte_font(MULTIBYTE_FONT_KOREAN);
    } else if (encoding == ENCODING_JAPANESE) {
        return load_multibyte_font(MULTIBYTE_FONT_JAPANESE);
    } else {
        free_font_memory();
        return 1;
    }
}

int image_load_enemy(int enemy_id)
{
    if (enemy_id == data.current_enemy && graphics_renderer()->has_image_atlas(ATLAS_ENEMY)) {
        return 1;
    }

    graphics_renderer()->get_max_image_size(&data.max_image_width, &data.max_image_height);

    const char *filename_bmp = ENEMY_GRAPHICS_555[enemy_id];
    const char *filename_idx = ENEMY_GRAPHICS_SG2[enemy_id];

    for (int i = 0; i < ENEMY_ENTRIES; i++) {
        free(data.enemy[i].top);
        free(data.enemy[i].animation);
    }

    memset(data.enemy, 0, sizeof(data.enemy));

    uint8_t *tmp_data = malloc(ENEMY_DATA_SIZE * sizeof(uint8_t));
    image_draw_data *draw_data = malloc(ENEMY_ENTRIES * sizeof(image_draw_data));
    memset(draw_data, 0, ENEMY_ENTRIES * sizeof(image_draw_data));

    if (!tmp_data || ENEMY_INDEX_SIZE != io_read_file_part_into_buffer(
        filename_idx, MAY_BE_LOCALIZED, tmp_data, ENEMY_INDEX_SIZE, ENEMY_INDEX_OFFSET)) {
        free(tmp_data);
        free_draw_data(draw_data, ENEMY_ENTRIES);
        return 0;
    }

    buffer buf;
    buffer_init(&buf, tmp_data, ENEMY_INDEX_SIZE);
    if (!prepare_images(&buf, data.enemy, draw_data, ENEMY_ENTRIES, ATLAS_ENEMY)) {
        free(tmp_data);
        free_draw_data(draw_data, ENEMY_ENTRIES);
        return 0;
    }

    int data_size = io_read_file_into_buffer(filename_bmp, MAY_BE_LOCALIZED, tmp_data, ENEMY_DATA_SIZE);
    if (!data_size) {
        free(tmp_data);
        free_draw_data(draw_data, ENEMY_ENTRIES);
        return 0;
    }

    buffer_init(&buf, tmp_data, data_size);
    if (!crop_and_pack_images(&buf, data.enemy, draw_data, ENEMY_ENTRIES, ATLAS_ENEMY)) {
        free(tmp_data);
        free_draw_data(draw_data, ENEMY_ENTRIES);
        return 0;
    }

    const image_atlas_data *atlas_data = graphics_renderer()->prepare_image_atlas(ATLAS_ENEMY,
        data.packer.result.images_needed, data.packer.result.last_image_width, data.packer.result.last_image_height);
    if (!atlas_data) {
        free(tmp_data);
        free_draw_data(draw_data, ENEMY_ENTRIES);
        image_packer_free(&data.packer);
        return 0;
    }

    convert_images(data.enemy, draw_data, ENEMY_ENTRIES, &buf, atlas_data);
    free(tmp_data);
    free_draw_data(draw_data, ENEMY_ENTRIES);
    data.current_enemy = enemy_id;
    graphics_renderer()->create_image_atlas(atlas_data, 1);
    image_packer_free(&data.packer);

    return 1;
}

int image_is_external(const image *img)
{
    return (img->atlas.id >> IMAGE_ATLAS_BIT_OFFSET) == ATLAS_EXTERNAL;
}

int image_load_external_pixels(color_t *dst, const image *img, int row_width)
{
    image_draw_data *draw_data = &data.external_draw_data[img->atlas.id & IMAGE_ATLAS_BIT_MASK];
    char filename[FILE_NAME_MAX] = "555/";
    strcpy(&filename[4], data.bitmaps[draw_data->bitmap_id]);
    file_change_extension(filename, "555");
    if (!draw_data->buffer) {
        draw_data->buffer = malloc(draw_data->data_length * sizeof(uint8_t));
        if (!draw_data->buffer) {
            release_external_buffers();
            draw_data->buffer = malloc(draw_data->data_length * sizeof(uint8_t));
            if (!draw_data->buffer) {
                log_error("unable to load external image - out of memory",
                    data.bitmaps[draw_data->bitmap_id], 0);
                return 0;
            }
        }
        int size = io_read_file_part_into_buffer(
            &filename[4], MAY_BE_LOCALIZED, draw_data->buffer,
            draw_data->data_length, draw_data->offset - 1
        );
        if (!size) {
            // try in 555 dir
            size = io_read_file_part_into_buffer(
                filename, MAY_BE_LOCALIZED, draw_data->buffer,
                draw_data->data_length, draw_data->offset - 1
            );
            if (!size) {
                log_error("unable to load external image",
                    data.bitmaps[draw_data->bitmap_id], 0);
                free(draw_data->buffer);
                return 0;
            }
        }
    }
    buffer buf;
    buffer_init(&buf, draw_data->buffer, draw_data->data_length);
    // NB: isometric images are never external
    if (draw_data->is_compressed) {
        convert_compressed(&buf, draw_data->width, 0, 0, draw_data->data_length, dst, row_width);
    } else {
        convert_uncompressed(&buf, draw_data->width, draw_data->height, 0, 0, dst, row_width);
    }
    return 1;
}

void image_load_external_data(const image *img)
{
    int external_image_id = img->atlas.id & IMAGE_ATLAS_BIT_MASK;
    if (data.external_image_id == external_image_id && graphics_renderer()->has_custom_image(CUSTOM_IMAGE_EXTERNAL)) {
        return;
    }
    data.external_image_id = external_image_id;
    image_draw_data *draw_data = &data.external_draw_data[external_image_id];
    graphics_renderer()->create_custom_image(CUSTOM_IMAGE_EXTERNAL, draw_data->width, draw_data->height, 0);
    int row_width;
    color_t *dst = graphics_renderer()->get_custom_image_buffer(CUSTOM_IMAGE_EXTERNAL, &row_width);
    if (!dst) {
        return;
    }
    if (image_load_external_pixels(dst, img, row_width)) {
        graphics_renderer()->update_custom_image(CUSTOM_IMAGE_EXTERNAL);
    }
    graphics_renderer()->release_custom_image_buffer(CUSTOM_IMAGE_EXTERNAL);
}

int image_get_external_dimensions(const image *img, int *width, int *height)
{
    if ((img->atlas.id >> IMAGE_ATLAS_BIT_OFFSET) != ATLAS_EXTERNAL) {
        return 0;
    }
    image_draw_data *draw_data = &data.external_draw_data[img->atlas.id & IMAGE_ATLAS_BIT_MASK];
    if (width) {
        *width = draw_data->width;
    }
    if (height) {
        *height = draw_data->height;
    }
    return 1;
}

void image_crop(image *img, const color_t *pixels)
{
    if (!img->width || !img->height) {
        return;
    }
    int x_first_opaque = 0;
    int x_last_opaque = 0;
    int y_first_opaque = 0;
    int y_last_opaque = 0;

    int found_opaque = 0;

    // Check all four sides of the rect
    for (int y = 0; y < img->height; y++) {
        const color_t *row = &pixels[y * img->width];
        for (int x = 0; x < img->width; x++) {
            if ((row[x] & COLOR_CHANNEL_ALPHA) != ALPHA_TRANSPARENT) {
                y_first_opaque = y;
                y_last_opaque = y;
                x_first_opaque = x;
                x_last_opaque = x;
                found_opaque = 1;
                break;
            }
        }
        if (found_opaque) {
            break;
        }
    }
    if (!found_opaque) {
        img->width = 0;
        img->height = 0;
        return;
    }
    found_opaque = 0;
    for (int y = img->height - 1; y > y_last_opaque; y--) {
        const color_t *row = &pixels[y * img->width];
        for (int x = img->width - 1; x >= 0; x--) {
            if ((row[x] & COLOR_CHANNEL_ALPHA) != ALPHA_TRANSPARENT) {
                y_last_opaque = y;
                if (x_first_opaque > x) {
                    x_first_opaque = x;
                }
                if (x_last_opaque < x) {
                    x_last_opaque = x;
                }
                found_opaque = 1;
                break;
            }
        }
        if (found_opaque) {
            break;
        }
    }
    found_opaque = 0;
    for (int x = 0; x < x_first_opaque; x++) {
        for (int y = y_first_opaque; y <= y_last_opaque; y++) {
            if ((pixels[y * img->width + x] & COLOR_CHANNEL_ALPHA) != ALPHA_TRANSPARENT) {
                x_first_opaque = x;
                found_opaque = 1;
                break;
            }
        }
        if (found_opaque) {
            break;
        }
    }
    found_opaque = 0;
    for (int x = img->width - 1; x > x_last_opaque; x--) {
        for (int y = y_first_opaque; y <= y_last_opaque; y++) {
            if ((pixels[y * img->width + x] & COLOR_CHANNEL_ALPHA) != ALPHA_TRANSPARENT) {
                x_last_opaque = x;
                found_opaque = 1;
                break;
            }
        }
        if (found_opaque) {
            break;
        }
    }

    img->x_offset = x_first_opaque;
    img->y_offset = y_first_opaque;
    img->width = x_last_opaque - x_first_opaque + 1;
    img->height = y_last_opaque - y_first_opaque + 1;
}

int image_group(int group)
{
    return data.group_image_ids[group];
}

const image *image_get(int id)
{
    if (id >= 0 && id < IMAGE_MAIN_ENTRIES) {
        return &data.main[id];
    } else {
        return assets_get_image(id);
    }
}

const image *image_letter(int letter_id)
{
    if (data.fonts_enabled == FULL_CHARSET_IN_FONT) {
        return &data.font[data.font_base_offset + letter_id];
    } else if (data.fonts_enabled == MULTIBYTE_IN_FONT && letter_id >= IMAGE_FONT_MULTIBYTE_OFFSET) {
        return &data.font[data.font_base_offset + letter_id - IMAGE_FONT_MULTIBYTE_OFFSET];
    } else if (letter_id < IMAGE_FONT_MULTIBYTE_OFFSET) {
        return &data.main[data.group_image_ids[GROUP_FONT] + letter_id];
    } else {
        return &DUMMY_IMAGE;
    }
}

const image *image_get_enemy(int id)
{
    if (id >= 0 && id < ENEMY_ENTRIES) {
        return &data.enemy[id];
    } else {
        return NULL;
    }
}

static void determine_max_width_and_height(const image_copy_info *copy, int *width, int *height)
{
    int max_src_width = copy->src.width - copy->src.x;
    int max_dst_width = copy->dst.width - copy->dst.x;
    *width = copy->rect.width > max_src_width ? max_src_width : copy->rect.width;
    if (*width > max_dst_width) {
        *width = max_dst_width;
    }
    int max_src_height = copy->src.height - copy->src.y;
    int max_dst_height = copy->dst.height - copy->dst.y;
    *height = copy->rect.height > max_src_height ? max_src_height : copy->rect.height;
    if (*height > max_dst_height) {
        *height = max_dst_height;
    }
}

void image_copy(const image_copy_info *copy)
{
    int width, height;
    determine_max_width_and_height(copy, &width, &height);
    if (width <= 0 || height <= 0) {
        return;
    }
    color_t *dst = &copy->dst.pixels[(copy->dst.y + copy->rect.y_offset) * copy->dst.width +
        copy->dst.x + copy->rect.x_offset];
    const color_t *src = &copy->src.pixels[copy->src.y * copy->src.width + copy->src.x];
    for (int y = 0; y < height; y++) {
        memcpy(&dst[y * copy->dst.width], &src[y * copy->src.width], width * sizeof(color_t));
    }
}

void image_copy_isometric_footprint(const image_copy_info *copy)
{
    int width, height;
    determine_max_width_and_height(copy, &width, &height);
    if (width <= 0 || height <= 0) {
        return;
    }
    int half_height = height / 2;
    color_t *dst = &copy->dst.pixels[(copy->dst.y + copy->rect.y_offset) * copy->dst.width +
        copy->dst.x + copy->rect.x_offset];
    const color_t *src = &copy->src.pixels[copy->src.y * copy->src.width + copy->src.x];

    for (int y = 0; y < height; y++) {
        int x_read = 2 + 4 * (y < half_height ? y : height - 1 - y);
        int x_skip = (copy->rect.width - x_read) / 2;
        if (x_skip > width) {
            continue;
        } else if (x_skip + x_read > width) {
            x_read = width - x_skip;
        }
        memcpy(&dst[y * copy->dst.width] + x_skip, &src[y * copy->src.width + x_skip], x_read * sizeof(color_t));
    }
}
