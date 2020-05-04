#include "smacker.h"

#include "core/file.h"
#include "core/log.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * SMK description from: https://wiki.multimedia.cx/index.php?title=Smacker
 */

#define HEADER_SIZE 104
#define MAX_TRACKS 7
#define MAX_PALETTE 256

#define FLAG_RING 0x01
#define FLAG_Y_INTERLACE 0x02
#define FLAG_Y_DOUBLE 0x04

#define AUDIO_MASK_RATE 0xFFFFFF
#define AUDIO_FLAG_STEREO 0x10000000
#define AUDIO_FLAG_16BIT 0x20000000
#define AUDIO_FLAG_HAS_TRACK 0x40000000
#define AUDIO_FLAG_COMPRESSED 0x80000000

#define BLOCK_MONO 0
#define BLOCK_FULL 1
#define BLOCK_VOID 2
#define BLOCK_SOLID 3

typedef struct {
    const uint8_t *data;
    int length;
    int index;
    int bit_index;
} bitstream;

typedef struct huffnode8_t {
    struct huffnode8_t *b[2];
    int is_leaf;
    uint8_t value;
} huffnode8;

typedef struct hufftree8_t {
    huffnode8 nodes[512];
    int size;
} hufftree8;

typedef struct huffnode16_t {
    struct huffnode16_t *b[2];
    int is_leaf;
    uint16_t value;
} huffnode16;

typedef struct hufftree16_t {
    huffnode16 *root;
    hufftree8 *low;
    hufftree8 *high;
    uint16_t escape_codes[3];
    huffnode16 *escape_nodes[3];
} hufftree16;

typedef struct {
    uint32_t palette[MAX_PALETTE];
    uint8_t *video;
    uint8_t *audio[MAX_TRACKS];
    int audio_len[MAX_TRACKS];
} frame_data_t;

struct smacker_t {
    FILE *fp;

    int32_t width;
    int32_t height;
    int32_t frames;
    int32_t us_per_frame;
    int32_t flags;
    int32_t trees_size;
    int32_t audio_size[7];
    int32_t audio_rate[7];

    long frame_data_offset_in_file;
    long *frame_offsets;
    int32_t *frame_sizes;
    uint8_t *frame_types;

    hufftree16 *mmap_tree;
    hufftree16 *mclr_tree;
    hufftree16 *full_tree;
    hufftree16 *type_tree;

    frame_data_t frame_data;
    int32_t current_frame;
};

static const uint8_t BIT_MASKS[] = {
    0x00,
    0x01,
    0x03,
    0x07,
    0x0f,
    0x1f,
    0x3f,
    0x7f,
    0xff,
};

static const uint8_t PALETTE_MAP[64] = {
    0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C,
    0x20, 0x24, 0x28, 0x2C, 0x30, 0x34, 0x38, 0x3C,
    0x41, 0x45, 0x49, 0x4D, 0x51, 0x55, 0x59, 0x5D,
    0x61, 0x65, 0x69, 0x6D, 0x71, 0x75, 0x79, 0x7D,
    0x82, 0x86, 0x8A, 0x8E, 0x92, 0x96, 0x9A, 0x9E,
    0xA2, 0xA6, 0xAA, 0xAE, 0xB2, 0xB6, 0xBA, 0xBE,
    0xC3, 0xC7, 0xCB, 0xCF, 0xD3, 0xD7, 0xDB, 0xDF,
    0xE3, 0xE7, 0xEB, 0xEF, 0xF3, 0xF7, 0xFB, 0xFF
};

static const int CHAIN_SIZE[64] = {
     1,    2,    3,    4,    5,    6,    7,    8,
     9,   10,   11,   12,   13,   14,   15,   16,
    17,   18,   19,   20,   21,   22,   23,   24,
    25,   26,   27,   28,   29,   30,   31,   32,
    33,   34,   35,   36,   37,   38,   39,   40,
    41,   42,   43,   44,   45,   46,   47,   48,
    49,   50,   51,   52,   53,   54,   55,   56,
    57,   58,   59,  128,  256,  512, 1024, 2048
};

static inline void *clear_malloc(size_t s)
{
    void *buf = malloc(s);
    if (buf) {
        memset(buf, 0, s);
    }
    return buf;
}

static int32_t read_i32(uint8_t *data)
{
    return (int32_t) (data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}

// Bitstream functions

static bitstream *bitstream_init(bitstream *bs, const uint8_t *data, int len)
{
    bs->data = data;
    bs->length = len;
    bs->index = 0;
    bs->bit_index = 0;
    return bs;
}

static inline int read_bit(bitstream *bs)
{
    if (bs->index >= bs->length) {
        return 0;
    }
    int result = bs->data[bs->index] & (1 << bs->bit_index);
    if (++bs->bit_index >= 8) {
        bs->index++;
        bs->bit_index = 0;
    }
    return result ? 1 : 0;
}

static inline uint8_t read_byte(bitstream *bs)
{
    if (bs->bit_index == 0) {
        // special case: on exact byte boundary
        if (bs->index < bs->length) {
            return bs->data[bs->index++];
        } else {
            return 0;
        }
    }
    if (bs->index + 1 >= bs->length) {
        return 0;
    }
    uint8_t value = bs->data[bs->index] >> bs->bit_index;
    bs->index++;
    value |= (bs->data[bs->index] & BIT_MASKS[bs->bit_index]) << (8 - bs->bit_index);
    return value;
}

// 8-bit huffman tree functions

static huffnode8 *build_tree8_nodes(bitstream *bs, hufftree8 *tree)
{
    huffnode8 *node = &tree->nodes[tree->size++];
    if (read_bit(bs)) {
        node->is_leaf = 0;
        node->b[0] = build_tree8_nodes(bs, tree);
        node->b[1] = build_tree8_nodes(bs, tree);
    } else {
        node->is_leaf = 1;
        node->value = read_byte(bs);
    }
    return node;
}

static hufftree8 *create_tree8(bitstream *bs)
{
    if (read_bit(bs)) {
        hufftree8 *tree = (hufftree8 *) clear_malloc(sizeof(hufftree8));
        if (!tree) {
            log_error("SMK: no memory for 8-bit tree", 0, 0);
            return NULL;
        }
        build_tree8_nodes(bs, tree);
        if (read_bit(bs) != 0) {
            log_error("SMK: 8-bit tree not closed", 0, 0);
            free(tree);
            return NULL;
        }
        return tree;
    } else {
        log_info("SMK: WARN: no 8-bit tree found", 0, 0);
        return NULL;
    }
}

static void free_tree8(hufftree8 *tree)
{
    free(tree);
}

static uint8_t lookup_tree8(bitstream *bs, hufftree8 *tree)
{
    huffnode8 *node = &tree->nodes[0];
    while (!node->is_leaf) {
        node = node->b[read_bit(bs)];
    }
    return node->value;
}

// 16-bit huffman tree functions

static void free_node16(huffnode16 *node)
{
    if (!node) {
        return;
    }
    if (!node->is_leaf) {
        free_node16(node->b[0]);
        free_node16(node->b[1]);
    }
    free(node);
}

static void free_tree16(hufftree16 *tree)
{
    if (!tree) {
        return;
    }
    for (int i = 0; i < 3; i++) {
        if (!tree->escape_nodes[i]->is_leaf) {
            // Free manually allocated node because it's not in the tree
            free(tree->escape_nodes[i]);
        }
    }
    free_node16(tree->root);
    free_tree8(tree->low);
    free_tree8(tree->high);
    free(tree);
}

static huffnode16 *build_tree16_nodes(bitstream *bs, hufftree16 *tree)
{
    huffnode16 *node = (huffnode16 *) clear_malloc(sizeof(huffnode16));
    if (!node) {
        log_error("SMK: no memory for 16-bit tree node", 0, 0);
        return NULL;
    }
    if (read_bit(bs)) {
        node->is_leaf = 0;
        node->b[0] = build_tree16_nodes(bs, tree);
        if (!node->b[0]) {
            free(node);
            return NULL;
        }
        node->b[1] = build_tree16_nodes(bs, tree);
        if (!node->b[1]) {
            free_node16(node->b[0]);
            free(node);
            return NULL;
        }
    } else {
        node->is_leaf = 1;
        uint8_t lo_val = lookup_tree8(bs, tree->low);
        uint8_t hi_val = lookup_tree8(bs, tree->high);
        uint16_t leaf_value = lo_val | (hi_val << 8);
        node->value = leaf_value;

        for (int i = 0; i < 3; i++) {
            if (leaf_value == tree->escape_codes[i]) {
                tree->escape_nodes[i] = node;
            }
        }
    }
    return node;
}

static hufftree16 *create_tree16(bitstream *bs, hufftree8 *low, hufftree8 *high)
{
    hufftree16 *tree = (hufftree16 *) clear_malloc(sizeof(hufftree16));
    if (!tree) {
        log_error("SMK: no memory for 16-bit tree", 0, 0);
        return NULL;
    }
    tree->low = low;
    tree->high = high;
    for (int i = 0; i < 3; i++) {
        // Do not join the following two lines as it results in an optimization bug for MSVC. See PR #215
        tree->escape_codes[i] = read_byte(bs);
        tree->escape_codes[i] |= read_byte(bs) << 8;
    }
    tree->root = build_tree16_nodes(bs, tree);
    if (!tree->root) {
        free(tree);
        return NULL;
    }
    if (read_bit(bs) != 0) {
        log_error("SMK: 16-bit tree not closed", 0, 0);
        free_tree16(tree);
        return NULL;
    }
    for (int i = 0; i < 3; i++) {
        if (!tree->escape_nodes[i]) {
            // Escape node is not in the tree: create a dummy node
            tree->escape_nodes[i] = (huffnode16 *) clear_malloc(sizeof(huffnode16));
            tree->escape_nodes[i]->is_leaf = 0;
            tree->escape_nodes[i]->value = 0;
        }
    }
    return tree;
}

static void reset_escape16(hufftree16 *tree)
{
    if (tree) {
        for (int i = 0; i < 3; i++) {
            tree->escape_nodes[i]->value = 0;
        }
    }
}

static uint16_t lookup_tree16(bitstream *bs, hufftree16 *tree)
{
    if (!tree) {
        return 0;
    }
    huffnode16 *node = tree->root;
    while (!node->is_leaf) {
        node = node->b[read_bit(bs)];
    }

    uint16_t value = node->value;
    if (value != tree->escape_nodes[0]->value) {
        tree->escape_nodes[2]->value = tree->escape_nodes[1]->value;
        tree->escape_nodes[1]->value = tree->escape_nodes[0]->value;
        tree->escape_nodes[0]->value = value;
    }
    return value;
}

static hufftree16 *read_header_tree(bitstream *bs)
{
    if (read_bit(bs)) {
        hufftree8 *low = create_tree8(bs);
        hufftree8 *high = create_tree8(bs);
        if (!low || !high) {
            free_tree8(low);
            free_tree8(high);
            return NULL;
        }
        return create_tree16(bs, low, high);
    } else {
        return NULL;
    }
}

static void read_header_trees(smacker s, uint8_t *data)
{
    bitstream bstream;
    bitstream *bs = bitstream_init(&bstream, data, s->trees_size);

    s->mmap_tree = read_header_tree(bs);
    s->mclr_tree = read_header_tree(bs);
    s->full_tree = read_header_tree(bs);
    s->type_tree = read_header_tree(bs);
}

// Smacker I/O functions

static int read_header(smacker s)
{
    uint8_t header[HEADER_SIZE];
    if (fread(header, 1, HEADER_SIZE, s->fp) != HEADER_SIZE) {
        log_error("SMK: unable to read header", 0, 0);
        return 0;
    }
    // check signature
    if (header[0] != 'S' || header[1] != 'M' || header[2] != 'K' || header[3] != '2') {
        log_error("SMK: file is not an SMK2 video", 0, 0);
        return 0;
    }
    s->width = read_i32(&header[4]);
    s->height = read_i32(&header[8]);
    s->frames = read_i32(&header[12]);
    int32_t frame_rate = read_i32(&header[16]);
    if (frame_rate > 0) {
        // frame rate is given in ms
        s->us_per_frame = frame_rate * 1000;
    } else if (frame_rate < 0) {
        // frame rate is given in 10us
        s->us_per_frame = -10 * frame_rate;
    } else {
        // 10 FPS = 0.1 sec per frame = 100,000 microseconds
        s->us_per_frame = 100000;
    }
    s->flags = read_i32(&header[20]);
    for (int i = 0; i < MAX_TRACKS; i++) {
        s->audio_size[i] = read_i32(&header[24 + 4 * i]);
    }
    s->trees_size = read_i32(&header[52]);
    // 56 - 72: skip mmap_size, mclr_size, full_size, type_size
    for (int i = 0; i < MAX_TRACKS; i++) {
        s->audio_rate[i] = read_i32(&header[72 + 4 * i]);
    }
    return 1;
}

static int read_frame_info(smacker s)
{
    int sizes_length = sizeof(int32_t) * s->frames;
    int types_length = sizeof(uint8_t) * s->frames;

    s->frame_sizes = (int32_t *) clear_malloc(sizes_length);
    s->frame_offsets = (long *) clear_malloc(sizeof(long) * s->frames);
    s->frame_types = (uint8_t *) clear_malloc(types_length);

    if (!s->frame_sizes || !s->frame_offsets || !s->frame_types) {
        log_error("SMK: no memory for frame info", 0, 0);
        free(s->frame_sizes);
        free(s->frame_offsets);
        free(s->frame_types);
        return 0;
    }

    if (fread(s->frame_sizes, 1, sizes_length, s->fp) != sizes_length ||
        fread(s->frame_types, 1, types_length, s->fp) != types_length) {
        log_error("SMK: unable to read frame info from file", 0, 0);
        free(s->frame_sizes);
        free(s->frame_offsets);
        free(s->frame_types);
        return 0;
    }

    uint8_t *data = (uint8_t *) s->frame_sizes;
    long offset = 0;
    for (int i = 0; i < s->frames; i++) {
        // Clear first two flag bits in-place (and flip endian-ness if necessary)
        s->frame_sizes[i] = read_i32(&data[4 * i]) & 0xfffffffc;
        s->frame_offsets[i] = offset;
        offset += s->frame_sizes[i];
    }
    return 1;
}

static int read_trees_data(smacker s)
{
    uint8_t *trees_data = (uint8_t *) clear_malloc(s->trees_size);
    if (!trees_data) {
        log_error("SMK: no memory for tree input data", 0, 0);
        return 0;
    }
    if (fread(trees_data, 1, s->trees_size, s->fp) != s->trees_size) {
        log_error("SMK: unable to read tree data from file", 0, 0);
        free(trees_data);
        return 0;
    }
    read_header_trees(s, trees_data);
    free(trees_data);
    return 1;
}

int allocate_frame_memory(smacker s)
{
    s->frame_data.video = clear_malloc(sizeof(uint8_t) * s->width * s->height);
    if (!s->frame_data.video) {
        log_error("SMK: no memory for video frame", 0, 0);
        return 0;
    }
    for (int i = 0; i < MAX_TRACKS; i++) {
        if (s->audio_rate[i] & AUDIO_FLAG_HAS_TRACK) {
            s->frame_data.audio[i] = clear_malloc(s->audio_size[i]);
            if (!s->frame_data.audio[i]) {
                log_error("SMK: no memory for audio track", 0, i);
                return 0;
            }
        }
    }
    return 1;
}

smacker smacker_open(FILE *fp)
{
    if (!fp) {
        log_error("SMK: file does not exist", 0, 0);
        return NULL;
    }
    smacker s = (struct smacker_t *) clear_malloc(sizeof(struct smacker_t));
    memset(s, 0, sizeof(struct smacker_t));
    s->fp = fp;

    if (!read_header(s)) {
        smacker_close(s);
        return NULL;
    }
    if (!read_frame_info(s)) {
        smacker_close(s);
        return NULL;
    }
    if (!read_trees_data(s)) {
        smacker_close(s);
        return NULL;
    }
    if (!allocate_frame_memory(s)) {
        smacker_close(s);
        return NULL;
    }
    s->frame_data_offset_in_file = ftell(s->fp);
    return s;
}

void smacker_close(smacker s)
{
    file_close(s->fp);
    free(s->frame_offsets);
    free(s->frame_sizes);
    free(s->frame_types);
    free_tree16(s->mclr_tree);
    free_tree16(s->mmap_tree);
    free_tree16(s->full_tree);
    free_tree16(s->type_tree);
    for (int i = 0; i < MAX_TRACKS; i++) {
        free(s->frame_data.audio[i]);
    }
    free(s->frame_data.video);
    free(s);
}

// Smacker info functions

void smacker_get_frames_info(const smacker s, int *frame_count, int *usf)
{
    if (frame_count) {
        *frame_count = s->frames;
    }
    if (usf) {
        *usf = s->us_per_frame;
    }
}

void smacker_get_video_info(const smacker s, int *width, int *height, int *y_scale_mode)
{
    if (width) {
        *width = s->width;
    }
    if (height) {
        *height = s->height;
    }
    if (y_scale_mode) {
        if (s->flags & FLAG_Y_INTERLACE) {
            *y_scale_mode = SMACKER_Y_SCALE_INTERLACE;
        } else if (s->flags & FLAG_Y_DOUBLE) {
            *y_scale_mode = SMACKER_Y_SCALE_DOUBLE;
        } else {
            *y_scale_mode = SMACKER_Y_SCALE_NONE;
        }
    }
}

void smacker_get_audio_info(const smacker s, int track, int *enabled, int *channels, int *bitdepth, int *audio_rate)
{
    int has_track = (s->audio_rate[track] & AUDIO_FLAG_HAS_TRACK) ? 1 : 0;
    if (enabled) {
        *enabled = has_track;
    }
    if (channels) {
        if (has_track) {
            *channels = (s->audio_rate[track] & AUDIO_FLAG_STEREO) ? 2 : 1;
        } else {
            *channels = 0;
        }
    }
    if (bitdepth) {
        if (has_track) {
            *bitdepth = (s->audio_rate[track] & AUDIO_FLAG_16BIT) ? 16 : 8;
        } else {
            *bitdepth = 0;
        }
    }
    if (audio_rate) {
        *audio_rate = s->audio_rate[track] & AUDIO_MASK_RATE;
    }
}

// Smacker decoding functions

static int read_audio_frame_trees(bitstream *bs, hufftree8 **trees, int num_trees)
{
    for (int i = 0; i < num_trees; i++) {
        trees[i] = create_tree8(bs);
        if (!trees[i]) {
            for (int j = 0; j < i; j++) {
                free_tree8(trees[j]);
            }
            return 0;
        }
    }
    return 1;
}

static int decode_audio_track(smacker s, int track, uint8_t *data, int length)
{
    if ((s->audio_rate[track] & AUDIO_FLAG_COMPRESSED) == 0) {
        // Uncompressed data, just copy and return
        memcpy(s->frame_data.audio[track], data, length);
        s->frame_data.audio_len[track] = length;
        return 0;
    }

    int32_t uncompressed_length = read_i32(data);

    bitstream bstream;
    bitstream *bs = bitstream_init(&bstream, &data[4], length - 4);
    if (!read_bit(bs)) {
        s->frame_data.audio_len[track] = 0;
        return 0;
    }

    int is_stereo = read_bit(bs);
    int is_16bit = read_bit(bs);
    int header_is_stereo = (s->audio_rate[track] & AUDIO_FLAG_STEREO) ? 1 : 0;
    int header_is_16bit = (s->audio_rate[track] & AUDIO_FLAG_16BIT) ? 1 : 0;
    if (is_stereo != header_is_stereo) {
        log_error("SMK: stereo flag in frame does not match header", 0, 0);
        return 0;
    }
    if (is_16bit != header_is_16bit) {
        log_error("SMK: 16-bit flag in frame does not match header", 0, 0);
        return 0;
    }

    int channels = is_stereo ? 2 : 1;
    int rate_bytes = is_16bit ? 2 : 1;
    int num_trees = channels * rate_bytes;
    hufftree8 *trees[4];
    if (!read_audio_frame_trees(bs, trees, num_trees)) {
        log_error("SMK: unable to read audio huffman trees", 0, 0);
        return 0;
    }

    if (is_16bit) {
        uint16_t *audio_data = (uint16_t *) s->frame_data.audio[track];
        int index = 0;
        for (int c = 0; c < channels; c++) {
            // Base of channels is reversed
            audio_data[channels - c - 1] = read_byte(bs) << 8 | read_byte(bs);
        }
        index = channels;

        while (index < uncompressed_length / 2) {
            for (int c = 0; c < channels; c++) {
                // Do not join the following two lines as it results in an optimization bug for MSVC. See PR #215
                uint16_t value = lookup_tree8(bs, trees[c * 2]);
                value |= lookup_tree8(bs, trees[c * 2 + 1]) << 8;
                audio_data[index] = value + audio_data[index - channels];
                index++;
            }
        }
        s->frame_data.audio_len[track] = index * 2;
    } else {
        uint8_t *audio_data = s->frame_data.audio[track];
        int index = 0;
        for (int c = 0; c < channels; c++) {
            // Base of channels is reversed
            audio_data[channels - c - 1] = read_byte(bs);
        }
        index = channels;

        while (index < uncompressed_length) {
            for (int c = 0; c < channels; c++) {
                audio_data[index] = lookup_tree8(bs, trees[c]) + audio_data[index - channels];
                index++;
            }
        }
        s->frame_data.audio_len[track] = index;
    }
    return 1;
}

static int decode_palette(smacker s, uint8_t *data, int length)
{
    color_t new_palette[MAX_PALETTE];
    int index = 0;
    int color_index = 0;
    while (index < length && color_index < MAX_PALETTE) {
        if (data[index] & 0x80) {
            // Copy from same position in previous palette
            int num_entries = 1 + (data[index] & 0x7f);
            if (num_entries + color_index > MAX_PALETTE) {
                log_error("SMK: invalid palette data", 0, 0);
                return 0;
            }
            memcpy(&new_palette[color_index], &s->frame_data.palette[color_index], sizeof(int32_t) * num_entries);
            color_index += num_entries;
            index++;
        } else if (data[index] & 0x40) {
            // Copy from 'offset' position in previous palette
            int num_entries = 1 + (data[index] & 0x3f);
            int offset = data[index + 1];
            if (num_entries + color_index > MAX_PALETTE || num_entries + offset > MAX_PALETTE) {
                log_error("SMK: invalid palette data", 0, 0);
                return 0;
            }
            memcpy(&new_palette[color_index], &s->frame_data.palette[offset], sizeof(int32_t) * num_entries);
            color_index += num_entries;
            index += 2;
        } else {
            // Literal color
            new_palette[color_index] =
                    (PALETTE_MAP[data[index] & 0x3f] << 16) |
                    (PALETTE_MAP[data[index + 1] & 0x3f] << 8) |
                    (PALETTE_MAP[data[index + 2] & 0x3f]);
            color_index++;
            index += 3;
        }
    }
    memcpy(s->frame_data.palette, new_palette, sizeof(color_t) * MAX_PALETTE);
    return 1;
}

static int decode_video(smacker s, uint8_t *frame_data, int length)
{
    reset_escape16(s->mclr_tree);
    reset_escape16(s->mmap_tree);
    reset_escape16(s->full_tree);
    reset_escape16(s->type_tree);

    bitstream bstream;
    bitstream *bs = bitstream_init(&bstream, frame_data, length);

    uint8_t *video = s->frame_data.video;

    int block_type = 0;
    int chain = 0;
    uint8_t solid_color = 0;
    for (int row = 0; row < s->height; row += 4) {
        for (int col = 0; col < s->width; col += 4) {
            if (chain <= 0) {
                uint16_t type = lookup_tree16(bs, s->type_tree);
                block_type = type & 0x03;
                chain = CHAIN_SIZE[(type >> 2) & 0x3f];
                solid_color = type >> 8;
            }
            if (block_type == BLOCK_MONO) {
                uint16_t colors = lookup_tree16(bs, s->mclr_tree);
                uint8_t color1 = colors & 0xff;
                uint8_t color2 = colors >> 8;
                uint16_t map = lookup_tree16(bs, s->mmap_tree);
                for (int y = 0; y < 4; y++) {
                    uint8_t *pixel = &video[(row + y) * s->width + col];
                    pixel[0] = (map & 1) ? color2 : color1;
                    pixel[1] = (map & 2) ? color2 : color1;
                    pixel[2] = (map & 4) ? color2 : color1;
                    pixel[3] = (map & 8) ? color2 : color1;
                    map >>= 4;
                }
            } else if (block_type == BLOCK_FULL) {
                for (int y = 0; y < 4; y++) {
                    uint8_t *pixel = &video[(row + y) * s->width + col];
                    uint16_t colors = lookup_tree16(bs, s->full_tree);
                    pixel[2] = colors & 0xff;
                    pixel[3] = colors >> 8;
                    colors = lookup_tree16(bs, s->full_tree);
                    pixel[0] = colors & 0xff;
                    pixel[1] = colors >> 8;
                }
            } else if (block_type == BLOCK_SOLID) {
                for (int y = 0; y < 4; y++) {
                    uint8_t *pixel = &video[(row + y) * s->width + col];
                    pixel[0] = pixel[1] = pixel[2] = pixel[3] = solid_color;
                }
            }
            chain--;
        }
    }
    return 1;
}

static uint8_t *read_frame_data(smacker s, int frame_id)
{
    if (fseek(s->fp, s->frame_data_offset_in_file + s->frame_offsets[frame_id], SEEK_SET) != 0) {
        log_error("SMK: unable to seek to frame data", 0, frame_id);
        return NULL;
    }
    int frame_size = s->frame_sizes[frame_id];
    uint8_t *frame_data = (uint8_t *) clear_malloc(frame_size);
    if (!frame_data) {
        log_error("SMK: no memory for frame data", 0, frame_id);
        return NULL;
    }
    if (fread(frame_data, 1, frame_size, s->fp) != frame_size) {
        log_error("SMK: unable to read data for frame", 0, frame_id);
        free(frame_data);
        return NULL;
    }
    return frame_data;
}

static void free_frame_data(const smacker s, uint8_t *frame_data)
{
    free(frame_data);
}

static smacker_frame_status decode_frame(smacker s)
{
    int frame_id = s->current_frame;
    if (frame_id >= s->frames) {
        return SMACKER_FRAME_DONE;
    }

    uint8_t *frame_data = read_frame_data(s, frame_id);
    if (!frame_data) {
        return SMACKER_FRAME_ERROR;
    }

    uint8_t frame_type = s->frame_types[frame_id];
    int data_index = 0;
    if (frame_type & 0x01) {
        int palette_size = frame_data[0] * 4;
        if (!decode_palette(s, &frame_data[1], palette_size - 1)) {
            free_frame_data(s, frame_data);
            return SMACKER_FRAME_ERROR;
        }
        data_index += palette_size;
    }
    for (int i = 0; i < MAX_TRACKS; i++) {
        if (frame_type & (1 << (i + 1))) {
            int track_length = read_i32(&frame_data[data_index]);
            decode_audio_track(s, i, &frame_data[data_index + 4], track_length - 4);
            data_index += track_length;
        } else {
            s->frame_data.audio_len[i] = 0;
        }
    }
    if (!decode_video(s, &frame_data[data_index], s->frame_sizes[frame_id] - data_index)) {
        free_frame_data(s, frame_data);
        return SMACKER_FRAME_ERROR;
    }

    free_frame_data(s, frame_data);
    return SMACKER_FRAME_OK;
}

smacker_frame_status smacker_first_frame(smacker s)
{
    s->current_frame = 0;
    return decode_frame(s);
}

smacker_frame_status smacker_next_frame(smacker s)
{
    s->current_frame++;
    return decode_frame(s);
}

// Smacker get frame data functions

const uint32_t *smacker_get_frame_palette(const smacker s)
{
    return s->frame_data.palette;
}

const uint8_t *smacker_get_frame_video(const smacker s)
{
    return s->frame_data.video;
}

int smacker_get_frame_audio_size(const smacker s, int track)
{
    return s->frame_data.audio_len[track];
}

const uint8_t *smacker_get_frame_audio(const smacker s, int track)
{
    return s->frame_data.audio[track];
}
