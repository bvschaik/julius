#include "image.h"

#include "core/buffer.h"
#include "core/file.h"
#include "core/io.h"
#include "core/log.h"

#include <stdlib.h>
#include <string.h>

#define HEADER_SIZE 20680
#define ENTRY_SIZE 64

#define MAIN_ENTRIES 10000
#define ENEMY_ENTRIES 801
#define CYRILLIC_FONT_ENTRIES 2000
#define TRAD_CHINESE_FONT_ENTRIES (3 * IMAGE_FONT_MULTIBYTE_TRAD_CHINESE_MAX_CHARS)
#define SIMP_CHINESE_FONT_ENTRIES (3 * IMAGE_FONT_MULTIBYTE_SIMP_CHINESE_MAX_CHARS)
#define KOREAN_FONT_ENTRIES (3 * IMAGE_FONT_MULTIBYTE_KOREAN_MAX_CHARS)

#define MAIN_INDEX_SIZE 660680
#define ENEMY_INDEX_OFFSET HEADER_SIZE
#define ENEMY_INDEX_SIZE ENTRY_SIZE * ENEMY_ENTRIES
#define CYRILLIC_FONT_INDEX_OFFSET HEADER_SIZE
#define CYRILLIC_FONT_INDEX_SIZE ENTRY_SIZE * CYRILLIC_FONT_ENTRIES

#define MAIN_DATA_SIZE 30000000
#define EMPIRE_DATA_SIZE (2000*1000*4)
#define ENEMY_DATA_SIZE 2400000
#define CYRILLIC_FONT_DATA_SIZE 1500000
#define TRAD_CHINESE_FONT_DATA_SIZE 7200000
#define KOREAN_FONT_DATA_SIZE 7500000
#define SCRATCH_DATA_SIZE 12100000

#define CYRILLIC_FONT_BASE_OFFSET 201

#define NAME_SIZE 32

enum {
    NO_EXTRA_FONT = 0,
    FULL_CHARSET_IN_FONT = 1,
    MULTIBYTE_IN_FONT = 2
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
static const char EMPIRE_555[NAME_SIZE] = "The_empire.555";

static const char CYRILLIC_FONTS_SG2[NAME_SIZE] = "C3_fonts.sg2";
static const char CYRILLIC_FONTS_555[NAME_SIZE] = "C3_fonts.555";
static const char CHINESE_FONTS_555[NAME_SIZE] = "rome.555";
static const char CHINESE_FONTS_555_V2[NAME_SIZE] = "rome-v2.555";
static const char KOREAN_FONTS_555[NAME_SIZE] = "korean.555";
static const char KOREAN_FONTS_555_V2[NAME_SIZE] = "korean-v2.555";

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

static const image DUMMY_IMAGE;

static struct {
    int current_climate;
    int is_editor;
    int fonts_enabled;
    int font_base_offset;

    uint16_t group_image_ids[300];
    char bitmaps[100][200];
    image main[MAIN_ENTRIES];
    image enemy[ENEMY_ENTRIES];
    image *font;
    color_t *main_data;
    color_t *empire_data;
    color_t *enemy_data;
    color_t *font_data;
    uint8_t *tmp_data;
} data = {.current_climate = -1};

int image_init(void)
{
    data.enemy_data = (color_t *) malloc(ENEMY_DATA_SIZE);
    data.main_data = (color_t *) malloc(MAIN_DATA_SIZE);
    data.empire_data = (color_t *) malloc(EMPIRE_DATA_SIZE);
    data.tmp_data = (uint8_t *) malloc(SCRATCH_DATA_SIZE);
    if (!data.main_data || !data.empire_data || !data.enemy_data || !data.tmp_data) {
        free(data.main_data);
        free(data.empire_data);
        free(data.enemy_data);
        free(data.tmp_data);
        return 0;
    }
    return 1;
}

static void prepare_index(image *images, int size)
{
    int offset = 4;
    for (int i = 1; i < size; i++) {
        image *img = &images[i];
        if (img->draw.is_external) {
            if (!img->draw.offset) {
                img->draw.offset = 1;
            }
        } else {
            img->draw.offset = offset;
            offset += img->draw.data_length;
        }
    }
}

static void read_index_entry(buffer *buf, image *img)
{
    img->draw.offset = buffer_read_i32(buf);
    img->draw.data_length = buffer_read_i32(buf);
    img->draw.uncompressed_length = buffer_read_i32(buf);
    buffer_skip(buf, 8);
    img->width = buffer_read_u16(buf);
    img->height = buffer_read_u16(buf);
    buffer_skip(buf, 6);
    img->num_animation_sprites = buffer_read_u16(buf);
    buffer_skip(buf, 2);
    img->sprite_offset_x = buffer_read_i16(buf);
    img->sprite_offset_y = buffer_read_i16(buf);
    buffer_skip(buf, 10);
    img->animation_can_reverse = buffer_read_i8(buf);
    buffer_skip(buf, 1);
    img->draw.type = buffer_read_u8(buf);
    img->draw.is_fully_compressed = buffer_read_i8(buf);
    img->draw.is_external = buffer_read_i8(buf);
    img->draw.has_compressed_part = buffer_read_i8(buf);
    buffer_skip(buf, 2);
    img->draw.bitmap_id = buffer_read_u8(buf);
    buffer_skip(buf, 1);
    img->animation_speed_id = buffer_read_u8(buf);
    buffer_skip(buf, 5);
}

static void read_index(buffer *buf, image *images, int size)
{
    for (int i = 0; i < size; i++) {
        read_index_entry(buf, &images[i]);
    }
    prepare_index(images, size);
}

static void read_header(buffer *buf)
{
    buffer_skip(buf, 80); // header integers
    for (int i = 0; i < 300; i++) {
        data.group_image_ids[i] = buffer_read_u16(buf);
    }
    buffer_read_raw(buf, data.bitmaps, 20000);
}

static color_t to_32_bit(uint16_t c)
{
    return ((c & 0x7c00) << 9) | ((c & 0x7000) << 4) |
           ((c & 0x3e0) << 6)  | ((c & 0x380) << 1) |
           ((c & 0x1f) << 3)   | ((c & 0x1c) >> 2);
}

static int convert_uncompressed(buffer *buf, int buf_length, color_t *dst)
{
    for (int i = 0; i < buf_length; i += 2) {
        *dst = to_32_bit(buffer_read_u16(buf));
        dst++;
    }
    return buf_length / 2;
}

static int convert_compressed(buffer *buf, int buf_length, color_t *dst)
{
    int dst_length = 0;
    while (buf_length > 0) {
        int control = buffer_read_u8(buf);
        if (control == 255) {
            // next byte = transparent pixels to skip
            *dst++ = 255;
            *dst++ = buffer_read_u8(buf);
            dst_length += 2;
            buf_length -= 2;
        } else {
            // control = number of concrete pixels
            *dst++ = control;
            for (int i = 0; i < control; i++) {
                *dst++ = to_32_bit(buffer_read_u16(buf));
            }
            dst_length += control + 1;
            buf_length -= control * 2 + 1;
        }
    }
    return dst_length;
}

static void convert_images(image *images, int size, buffer *buf, color_t *dst)
{
    color_t *start_dst = dst;
    dst++; // make sure img->offset > 0
    for (int i = 0; i < size; i++) {
        image *img = &images[i];
        if (img->draw.is_external) {
            continue;
        }
        buffer_set(buf, img->draw.offset);
        int img_offset = (int) (dst - start_dst);
        if (img->draw.is_fully_compressed) {
            dst += convert_compressed(buf, img->draw.data_length, dst);
        } else if (img->draw.has_compressed_part) { // isometric tile
            dst += convert_uncompressed(buf, img->draw.uncompressed_length, dst);
            dst += convert_compressed(buf, img->draw.data_length - img->draw.uncompressed_length, dst);
        } else {
            dst += convert_uncompressed(buf, img->draw.data_length, dst);
        }
        img->draw.offset = img_offset;
        img->draw.uncompressed_length /= 2;
    }
}

static void load_empire(void)
{
    int size = io_read_file_into_buffer(EMPIRE_555, MAY_BE_LOCALIZED, data.tmp_data, EMPIRE_DATA_SIZE);
    if (size != EMPIRE_DATA_SIZE / 2) {
        log_error("unable to load empire data", EMPIRE_555, 0);
        return;
    }
    buffer buf;
    buffer_init(&buf, data.tmp_data, size);
    convert_uncompressed(&buf, size, data.empire_data);
}

int image_load_climate(int climate_id, int is_editor, int force_reload)
{
    if (climate_id == data.current_climate && is_editor == data.is_editor && !force_reload) {
        return 1;
    }

    const char *filename_bmp = is_editor ? EDITOR_GRAPHICS_555[climate_id] : MAIN_GRAPHICS_555[climate_id];
    const char *filename_idx = is_editor ? EDITOR_GRAPHICS_SG2[climate_id] : MAIN_GRAPHICS_SG2[climate_id];

    if (MAIN_INDEX_SIZE != io_read_file_into_buffer(filename_idx, MAY_BE_LOCALIZED, data.tmp_data, MAIN_INDEX_SIZE)) {
        return 0;
    }

    buffer buf;
    buffer_init(&buf, data.tmp_data, HEADER_SIZE);
    read_header(&buf);
    buffer_init(&buf, &data.tmp_data[HEADER_SIZE], ENTRY_SIZE * MAIN_ENTRIES);
    read_index(&buf, data.main, MAIN_ENTRIES);

    int data_size = io_read_file_into_buffer(filename_bmp, MAY_BE_LOCALIZED, data.tmp_data, SCRATCH_DATA_SIZE);
    if (!data_size) {
        return 0;
    }
    buffer_init(&buf, data.tmp_data, data_size);
    convert_images(data.main, MAIN_ENTRIES, &buf, data.main_data);
    data.current_climate = climate_id;
    data.is_editor = is_editor;

    load_empire();
    return 1;
}

static void free_font_memory(void)
{
    free(data.font);
    free(data.font_data);
    data.font = 0;
    data.font_data = 0;
    data.fonts_enabled = NO_EXTRA_FONT;
}

static int alloc_font_memory(int font_entries, int font_data_size)
{
    free_font_memory();
    data.font = (image*) malloc(font_entries * sizeof(image));
    data.font_data = (color_t *) malloc(font_data_size);
    if (!data.font || !data.font_data) {
        free(data.font);
        free(data.font_data);
        return 0;
    }
    memset(data.font, 0, font_entries * sizeof(image));
    return 1;
}

static int load_cyrillic_fonts(void)
{
    if (!alloc_font_memory(CYRILLIC_FONT_ENTRIES, CYRILLIC_FONT_DATA_SIZE)) {
        return 0;
    }
    if (CYRILLIC_FONT_INDEX_SIZE != io_read_file_part_into_buffer(CYRILLIC_FONTS_SG2, MAY_BE_LOCALIZED,
        data.tmp_data, CYRILLIC_FONT_INDEX_SIZE, CYRILLIC_FONT_INDEX_OFFSET)) {
        return 0;
    }
    buffer buf;
    buffer_init(&buf, data.tmp_data, CYRILLIC_FONT_INDEX_SIZE);
    read_index(&buf, data.font, CYRILLIC_FONT_ENTRIES);

    int data_size = io_read_file_into_buffer(CYRILLIC_FONTS_555, MAY_BE_LOCALIZED, data.tmp_data, SCRATCH_DATA_SIZE);
    if (!data_size) {
        return 0;
    }
    buffer_init(&buf, data.tmp_data, data_size);
    convert_images(data.font, CYRILLIC_FONT_ENTRIES, &buf, data.font_data);

    data.fonts_enabled = FULL_CHARSET_IN_FONT;
    data.font_base_offset = CYRILLIC_FONT_BASE_OFFSET;
    return 1;
}

static int parse_multibyte_font(
    int num_chars, buffer *input, color_t *pixels, int pixel_offset,
    int char_size, int letter_spacing, int index_offset)
{
    for (int i = 0; i < num_chars; i++) {
        image *img = &data.font[index_offset + i];
        img->width = char_size + letter_spacing;
        img->height = char_size;
        img->draw.bitmap_id = 0;
        img->draw.offset = pixel_offset;
        img->draw.uncompressed_length = img->draw.data_length = char_size * char_size;
        for (int row = 0; row < char_size; row++) {
            uint8_t bits = 0;
            for (int col = 0; col < char_size; col++) {
                if (col % 2 == 0) {
                    bits = buffer_read_u8(input);
                }
                uint8_t value = bits & 0xf;
                if (value == 0) {
                    *pixels = COLOR_SG2_TRANSPARENT;
                } else {
                    uint32_t color_value = (value * 16 + value);
                    *pixels = color_value << 24;
                }
                pixels++;
                pixel_offset++;
                bits >>= 4;
            }
            for (int s = 0; s < letter_spacing; s++) {
                *pixels = COLOR_SG2_TRANSPARENT;
                pixels++;
                pixel_offset++;
            }
        }
    }
    return pixel_offset;
}

static int parse_chinese_font(
    int num_chars, buffer *input, color_t *pixels, int pixel_offset, int char_size, int index_offset)
{
    int bytes_per_row = char_size <= 16 ? 2 : 3;
    for (int i = 0; i < num_chars; i++) {
        image *img = &data.font[index_offset + i];
        img->width = char_size + 1;
        img->height = char_size - 1;
        img->draw.bitmap_id = 0;
        img->draw.offset = pixel_offset;
        img->draw.uncompressed_length = img->draw.data_length = img->width * img->height;
        for (int row = 0; row < img->height; row++) {
            unsigned int bits = buffer_read_u16(input);
            if (bytes_per_row == 3) {
                bits += buffer_read_u8(input) << 16;
            }
            int prev_set = 0;
            for (int col = 0; col < img->width; col++) {
                int set = bits & 1;
                if (set) {
                    *pixels = ALPHA_OPAQUE;
                } else if (prev_set) {
                    *pixels = ALPHA_FONT_SEMI_TRANSPARENT;
                } else {
                    *pixels = COLOR_SG2_TRANSPARENT;
                }
                pixels++;
                pixel_offset++;
                bits >>= 1;
                prev_set = set;
            }
        }
    }
    return pixel_offset;
}

static int load_traditional_chinese_fonts(void)
{
    if (!alloc_font_memory(TRAD_CHINESE_FONT_ENTRIES, TRAD_CHINESE_FONT_DATA_SIZE)) {
        return 0;
    }

    int file_version = 2;
    int data_size = io_read_file_into_buffer(CHINESE_FONTS_555_V2, MAY_BE_LOCALIZED, data.tmp_data, SCRATCH_DATA_SIZE);
    if (!data_size) {
        file_version = 1;
        data_size = io_read_file_into_buffer(CHINESE_FONTS_555, MAY_BE_LOCALIZED, data.tmp_data, SCRATCH_DATA_SIZE);
        if (!data_size) {
            log_error("Julius requires extra files for Chinese characters:", CHINESE_FONTS_555_V2, 0);
            return 0;
        }
    }

    buffer input;
    buffer_init(&input, data.tmp_data, data_size);
    color_t *pixels = data.font_data;
    int offset = 0;
    int num_chars = IMAGE_FONT_MULTIBYTE_TRAD_CHINESE_MAX_CHARS;

    log_info("Parsing Traditional Chinese font", 0, 0);
    if (file_version == 2) {
        // 4-bit font file
        offset = parse_multibyte_font(num_chars, &input, &pixels[offset], offset, 12, 1, 0);
        offset = parse_multibyte_font(num_chars, &input, &pixels[offset], offset, 15, 1, num_chars);
        offset = parse_multibyte_font(num_chars, &input, &pixels[offset], offset, 20, 1, num_chars*2);
    } else if (file_version == 1) {
        // Old 1-bit font file
        offset = parse_chinese_font(num_chars, &input, &pixels[offset], offset, 12, 0);
        offset = parse_chinese_font(num_chars, &input, &pixels[offset], offset, 16, num_chars);
        offset = parse_chinese_font(num_chars, &input, &pixels[offset], offset, 20, num_chars * 2);
    }
    log_info("Done parsing Traditional Chinese font", 0, 0);

    data.fonts_enabled = MULTIBYTE_IN_FONT;
    data.font_base_offset = 0;
    return 1;
}

static int load_simplified_chinese_fonts(void)
{
    if (!alloc_font_memory(TRAD_CHINESE_FONT_ENTRIES, TRAD_CHINESE_FONT_DATA_SIZE)) {
        return 0;
    }

    int file_version = 2;
    int data_size = io_read_file_into_buffer(CHINESE_FONTS_555_V2, MAY_BE_LOCALIZED, data.tmp_data, SCRATCH_DATA_SIZE);
    if (!data_size) {
        file_version = 1;
        data_size = io_read_file_into_buffer(CHINESE_FONTS_555, MAY_BE_LOCALIZED, data.tmp_data, SCRATCH_DATA_SIZE);
        if (!data_size) {
            log_error("Julius requires extra files for Chinese characters:", CHINESE_FONTS_555_V2, 0);
            return 0;
        }
    }

    buffer input;
    buffer_init(&input, data.tmp_data, data_size);
    color_t *pixels = data.font_data;
    int offset = 0;
    int num_chars = IMAGE_FONT_MULTIBYTE_SIMP_CHINESE_MAX_CHARS;

    log_info("Parsing Simplified Chinese font", 0, 0);
    if (file_version == 2) {
        // 4-bit font file
        offset = parse_multibyte_font(num_chars, &input, &pixels[offset], offset, 12, 1, 0);
        offset = parse_multibyte_font(num_chars, &input, &pixels[offset], offset, 15, 1, num_chars);
        offset = parse_multibyte_font(num_chars, &input, &pixels[offset], offset, 20, 1, num_chars*2);
    } else if (file_version == 1) {
        // Old 1-bit font file
        offset = parse_chinese_font(num_chars, &input, &pixels[offset], offset, 12, 0);
        offset = parse_chinese_font(num_chars, &input, &pixels[offset], offset, 16, num_chars);
        offset = parse_chinese_font(num_chars, &input, &pixels[offset], offset, 19, num_chars * 2);
    }
    log_info("Done parsing Simplified Chinese font", 0, 0);

    data.fonts_enabled = MULTIBYTE_IN_FONT;
    data.font_base_offset = 0;
    return 1;
}

static int parse_korean_font(buffer *input, color_t *pixels, int pixel_offset, int char_size, int index_offset)
{
    int bytes_per_row = char_size <= 16 ? 2 : 3;
    for (int i = 0; i < IMAGE_FONT_MULTIBYTE_KOREAN_MAX_CHARS; i++) {
        image *img = &data.font[index_offset + i];
        img->width = char_size;
        img->height = char_size;
        img->draw.bitmap_id = 0;
        img->draw.offset = pixel_offset;
        img->draw.uncompressed_length = img->draw.data_length = char_size * char_size;
        for (int row = 0; row < char_size; row++) {
            unsigned int bits = buffer_read_u16(input);
            if (bytes_per_row == 3) {
                bits += buffer_read_u8(input) << 16;
            }
            int prev_set = 0;
            for (int col = 0; col < char_size; col++) {
                int set = bits & 1;
                if (set) {
                    *pixels = ALPHA_OPAQUE;
                } else if (prev_set) {
                    *pixels = ALPHA_FONT_SEMI_TRANSPARENT;
                } else {
                    *pixels = COLOR_SG2_TRANSPARENT;
                }
                pixels++;
                pixel_offset++;
                bits >>= 1;
                prev_set = set;
            }
        }
    }
    return pixel_offset;
}

static int load_korean_fonts(void)
{
    if (!alloc_font_memory(KOREAN_FONT_ENTRIES, KOREAN_FONT_DATA_SIZE)) {
        return 0;
    }

    int file_version = 2;
    int data_size = io_read_file_into_buffer(KOREAN_FONTS_555_V2, MAY_BE_LOCALIZED, data.tmp_data, SCRATCH_DATA_SIZE);
    if (!data_size) {
        file_version = 1;
        data_size = io_read_file_into_buffer(KOREAN_FONTS_555, MAY_BE_LOCALIZED, data.tmp_data, SCRATCH_DATA_SIZE);
        if (!data_size) {
            log_error("Julius requires extra files for Korean characters:", KOREAN_FONTS_555, 0);
            return 0;
        }
    }

    buffer input;
    buffer_init(&input, data.tmp_data, data_size);
    color_t *pixels = data.font_data;
    int offset = 0;
    int num_chars = IMAGE_FONT_MULTIBYTE_KOREAN_MAX_CHARS;

    log_info("Parsing Korean font", 0, 0);
    if (file_version == 2) {
        // 4-bit font file
        offset = parse_multibyte_font(num_chars, &input, &pixels[offset], offset, 12, 0, 0);
        offset = parse_multibyte_font(num_chars, &input, &pixels[offset], offset, 15, 0, num_chars);
        offset = parse_multibyte_font(num_chars, &input, &pixels[offset], offset, 20, 0, num_chars*2);
    } else if (file_version == 1) {
        // Old 1-bit font file
        offset = parse_korean_font(&input, &pixels[offset], offset, 12, 0);
        offset = parse_korean_font(&input, &pixels[offset], offset, 15, num_chars);
        offset = parse_korean_font(&input, &pixels[offset], offset, 20, num_chars * 2);
    }
    log_info("Done parsing Korean font", 0, 0);

    data.fonts_enabled = MULTIBYTE_IN_FONT;
    data.font_base_offset = 0;
    return 1;
}

int image_load_fonts(encoding_type encoding)
{
    if (encoding == ENCODING_CYRILLIC) {
        return load_cyrillic_fonts();
    } else if (encoding == ENCODING_TRADITIONAL_CHINESE) {
        return load_traditional_chinese_fonts();
    } else if (encoding == ENCODING_SIMPLIFIED_CHINESE) {
        return load_simplified_chinese_fonts();
    } else if (encoding == ENCODING_KOREAN) {
        return load_korean_fonts();
    } else {
        free_font_memory();
        return 1;
    }
}

int image_load_enemy(int enemy_id)
{
    const char *filename_bmp = ENEMY_GRAPHICS_555[enemy_id];
    const char *filename_idx = ENEMY_GRAPHICS_SG2[enemy_id];

    if (ENEMY_INDEX_SIZE != io_read_file_part_into_buffer(
        filename_idx, MAY_BE_LOCALIZED, data.tmp_data, ENEMY_INDEX_SIZE, ENEMY_INDEX_OFFSET)) {
        return 0;
    }

    buffer buf;
    buffer_init(&buf, data.tmp_data, ENEMY_INDEX_SIZE);
    read_index(&buf, data.enemy, ENEMY_ENTRIES);

    int data_size = io_read_file_into_buffer(filename_bmp, MAY_BE_LOCALIZED, data.tmp_data, SCRATCH_DATA_SIZE);
    if (!data_size) {
        return 0;
    }
    buffer_init(&buf, data.tmp_data, data_size);
    convert_images(data.enemy, ENEMY_ENTRIES, &buf, data.enemy_data);
    return 1;
}

static const color_t *load_external_data(int image_id)
{
    image *img = &data.main[image_id];
    char filename[FILE_NAME_MAX] = "555/";
    strcpy(&filename[4], data.bitmaps[img->draw.bitmap_id]);
    file_change_extension(filename, "555");
    int size = io_read_file_part_into_buffer(
        &filename[4], MAY_BE_LOCALIZED, data.tmp_data,
        img->draw.data_length, img->draw.offset - 1
    );
    if (!size) {
        // try in 555 dir
        size = io_read_file_part_into_buffer(
            filename, MAY_BE_LOCALIZED, data.tmp_data,
            img->draw.data_length, img->draw.offset - 1
        );
        if (!size) {
            log_error("unable to load external image",
                      data.bitmaps[img->draw.bitmap_id], image_id);
            return NULL;
        }
    }
    buffer buf;
    buffer_init(&buf, data.tmp_data, size);
    color_t *dst = (color_t*) &data.tmp_data[4000000];
    // NB: isometric images are never external
    if (img->draw.is_fully_compressed) {
        convert_compressed(&buf, img->draw.data_length, dst);
    } else {
        convert_uncompressed(&buf, img->draw.data_length, dst);
    }
    return dst;
}

int image_group(int group)
{
    return data.group_image_ids[group];
}

const image *image_get(int id)
{
    if (id >= 0 && id < MAIN_ENTRIES) {
        return &data.main[id];
    } else {
        return NULL;
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

const color_t *image_data(int id)
{
    if (id < 0 || id >= MAIN_ENTRIES) {
        return NULL;
    }
    if (!data.main[id].draw.is_external) {
        return &data.main_data[data.main[id].draw.offset];
    } else if (id == image_group(GROUP_EMPIRE_MAP)) {
        return data.empire_data;
    } else {
        return load_external_data(id);
    }
}

const color_t *image_data_letter(int letter_id)
{
    if (data.fonts_enabled == FULL_CHARSET_IN_FONT) {
        return &data.font_data[data.font[data.font_base_offset + letter_id].draw.offset];
    } else if (data.fonts_enabled == MULTIBYTE_IN_FONT && letter_id >= IMAGE_FONT_MULTIBYTE_OFFSET) {
        return &data.font_data[data.font[data.font_base_offset + letter_id - IMAGE_FONT_MULTIBYTE_OFFSET].draw.offset];
    } else if (letter_id < IMAGE_FONT_MULTIBYTE_OFFSET) {
        int image_id = data.group_image_ids[GROUP_FONT] + letter_id;
        return &data.main_data[data.main[image_id].draw.offset];
    } else {
        return NULL;
    }
}

const color_t *image_data_enemy(int id)
{
    if (data.enemy[id].draw.offset > 0) {
        return &data.enemy_data[data.enemy[id].draw.offset];
    }
    return NULL;
}
