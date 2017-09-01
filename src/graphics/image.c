#include "image.h"

#include <stdlib.h>
#include <string.h>

#include "core/buffer.h"
#include "core/file.h"
#include "core/io.h"

#define MAIN_INDEX_SIZE 660680
#define ENEMY_INDEX_OFFSET 20680
#define ENEMY_INDEX_SIZE 51264

#define HEADER_SIZE 20680
#define ENTRY_SIZE 64

#define MAIN_ENTRIES 10000
#define ENEMY_ENTRIES 801

#define MAIN_DATA_SIZE 12100000
#define ENEMY_DATA_SIZE 4900000

#define NAME_SIZE 32

static const char main_graphics_sg2[][NAME_SIZE] = {
    "c3.sg2",
    "c3_north.sg2",
    "c3_south.sg2"
};
static const char main_graphics_555[][NAME_SIZE] = {
    "c3.555",
    "c3_north.555",
    "c3_south.555"
};
static const char enemy_graphics_sg2[][NAME_SIZE] = {
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
static const char enemy_graphics_555[][NAME_SIZE] = {
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

static struct {
    int current_climate;

    uint16_t group_image_ids[300];
    char bitmaps[100][200];
    image main[MAIN_ENTRIES];
    image enemy[ENEMY_ENTRIES];
    uint8_t *main_data;
    uint8_t *enemy_data;
    uint8_t *tmp_data;
} data = {.current_climate = -1};

int image_init()
{
    data.enemy_data = (uint8_t *) malloc(ENEMY_DATA_SIZE);
    data.main_data = (uint8_t *) malloc(MAIN_DATA_SIZE);
    data.tmp_data = (uint8_t *) malloc(2000 * 1000 * 2);
    if (!data.enemy_data || !data.main_data || !data.tmp_data) {
        return 0;
    }
    return 1;
}

static void prepare_index(image *images, int size)
{
    int32_t offset = 4;
    for (int i = 1; i < size; i++) {
        image *img = &images[i];
        if (img->is_external) {
            if (!img->offset) {
                img->offset = 1;
            }
        } else {
            img->offset = offset;
            offset += img->data_length;
        }
    }
}

static void read_index_entry(buffer *buf, image *img)
{
    img->offset = buffer_read_i32(buf);
    img->data_length = buffer_read_i32(buf);
    img->uncompressed_length = buffer_read_i32(buf);
    buffer_skip(buf, 4);
    img->inverted_image_offset = buffer_read_i32(buf);
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
    img->type = buffer_read_u8(buf);
    img->is_fully_compressed = buffer_read_i8(buf);
    img->is_external = buffer_read_i8(buf);
    img->has_compressed_part = buffer_read_i8(buf);
    buffer_skip(buf, 1);
    img->building_size = buffer_read_u8(buf);
    img->bitmap_id = buffer_read_u8(buf);
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

int image_load_climate(int climate_id)
{
    if (climate_id == data.current_climate) {
        return 1;
    }

    const char *filename_bmp = main_graphics_555[climate_id];
    const char *filename_idx = main_graphics_sg2[climate_id];

    if (MAIN_INDEX_SIZE != io_read_file_into_buffer(filename_idx, data.tmp_data, MAIN_INDEX_SIZE)) {
        return 0;
    }
    if (!io_read_file_into_buffer(filename_bmp, data.main_data, MAIN_DATA_SIZE)) {
        return 0;
    }

    buffer buf;
    buffer_init(&buf, data.tmp_data, HEADER_SIZE);
    read_header(&buf);
    buffer_init(&buf, &data.tmp_data[HEADER_SIZE], ENTRY_SIZE * MAIN_ENTRIES);
    read_index(&buf, data.main, MAIN_ENTRIES);
    data.current_climate = climate_id;
    return 1;
}

int image_load_enemy(int enemy_id)
{
    const char *filename_bmp = enemy_graphics_555[enemy_id];
    const char *filename_idx = enemy_graphics_sg2[enemy_id];

    if (ENEMY_INDEX_SIZE != io_read_file_part_into_buffer(filename_idx, data.tmp_data, ENEMY_INDEX_SIZE, ENEMY_INDEX_OFFSET)) {
        return 0;
    }
    if (!io_read_file_into_buffer(filename_bmp, data.enemy_data, ENEMY_DATA_SIZE)) {
        return 0;
    }

    buffer buf;
    buffer_init(&buf, data.tmp_data, ENTRY_SIZE * ENEMY_ENTRIES);
    read_index(&buf, data.enemy, ENEMY_ENTRIES);
    return 1;
}

const uint8_t *image_load_external_data(int image_id)
{
    image *img = &data.main[image_id];
    char filename[200] = "555/";
    strcpy(&filename[4], data.bitmaps[img->bitmap_id]);
    file_change_extension(filename, "555");
    if (!io_read_file_part_into_buffer(&filename[4], data.tmp_data,
                                       img->data_length, img->offset - 1)) {
        // try in 555 dir
        if (!io_read_file_part_into_buffer(filename, data.tmp_data,
                                           img->data_length, img->offset - 1)) {
            return NULL;
        }
    }
    return data.tmp_data;
}

int image_group(int group)
{
    return data.group_image_ids[group];
}

