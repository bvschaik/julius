#include "core/zip.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "core/log.h"

enum {
    PK_SUCCESS = 0,
    PK_INVALID_WINDOWSIZE = 1,
    PK_LITERAL_ENCODING_UNSUPPORTED = 2,
    PK_TOO_FEW_INPUT_BYTES = 3,
    PK_ERROR_DECODING = 4,
    PK_ERROR_VALUE = 774,
    PK_EOF = 773,
};

struct pk_token {
    int stop;

    const uint8_t *input_data;
    int input_ptr;
    int input_length;

    uint8_t *output_data;
    int output_ptr;
    int output_length;
};

typedef int pk_input_func(uint8_t *buffer, int length, struct pk_token *token);
typedef void pk_output_func(uint8_t *buffer, int length, struct pk_token *token);

struct pk_comp_buffer {
    pk_input_func *input_func;
    pk_output_func *output_func;
    struct pk_token *token;

    int window_size;
    int dictionary_size;
    unsigned int copy_offset_extra_mask;
    int current_output_bits_used;

    uint8_t input_data[8708];
    uint8_t output_data[2050];
    int output_ptr;

    uint16_t analyze_offset_table[2304];
    uint16_t analyze_index[8708];
    signed short long_matcher[518];

    uint16_t codeword_values[774];
    uint8_t codeword_bits[774];
};

struct pk_decomp_buffer {
    pk_input_func *input_func;
    pk_output_func *output_func;
    struct pk_token *token;

    int window_size;
    int dictionary_size;
    uint16_t current_input_byte;
    int current_input_bits_available;

    int input_buffer_ptr;
    int input_buffer_end;
    int output_buffer_ptr;
    uint8_t input_buffer[2048];
    uint8_t output_buffer[8708]; // 2x 4096 (max dict size) + 516 for copying

    uint8_t copy_offset_jump_table[256];
    uint8_t copy_length_jump_table[256];
};

struct pk_copy_length_offset {
    int length;
    uint16_t offset;
};

static const uint8_t pk_copy_offset_bits[64] = {
    2, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
};

static const uint8_t pk_copy_offset_code[64] = {
    0x03, 0x0D, 0x05, 0x19, 0x09, 0x11, 0x01, 0x3E,
    0x1E, 0x2E, 0x0E, 0x36, 0x16, 0x26, 0x06, 0x3A,
    0x1A, 0x2A, 0x0A, 0x32, 0x12, 0x22, 0x42, 0x02,
    0x7C, 0x3C, 0x5C, 0x1C, 0x6C, 0x2C, 0x4C, 0x0C,
    0x74, 0x34, 0x54, 0x14, 0x64, 0x24, 0x44, 0x04,
    0x78, 0x38, 0x58, 0x18, 0x68, 0x28, 0x48, 0x08,
    0xF0, 0x70, 0xB0, 0x30, 0xD0, 0x50, 0x90, 0x10,
    0xE0, 0x60, 0xA0, 0x20, 0xC0, 0x40, 0x80, 0x00,
};

static const uint8_t pk_copy_length_base_bits[16] = {
    3, 2, 3, 3, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 7, 7,
};

static const uint16_t pk_copy_length_base_value[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x0A, 0x0E, 0x16, 0x26, 0x46, 0x86, 0x106,
};

static const uint8_t pk_copy_length_base_code[16] = {
    0x05, 0x03, 0x01, 0x06, 0x0A, 0x02, 0x0C, 0x14,
    0x04, 0x18, 0x08, 0x30, 0x10, 0x20, 0x40, 0x00,
};

static const uint8_t pk_copy_length_extra_bits[16] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
};

static void pk_memcpy(uint8_t *dst, const uint8_t *src, int length)
{
    for (int i = 0; i < length; i++) {
        dst[i] = src[i];
    }
}

static void pk_explode_construct_jump_table(int size, const uint8_t *bits, const uint8_t *codes, uint8_t *jump)
{
    for (int i = size - 1; i >= 0; i--) {
        uint8_t bit = bits[i];
        int code = codes[i];
        do {
            jump[code] = (uint8_t) i;
            code += 1 << bit;
        } while (code < 0x100);
    }
}

static int pk_explode_set_bits_used(struct pk_decomp_buffer *buf, int num_bits)
{
    if (buf->current_input_bits_available >= num_bits) {
        buf->current_input_bits_available -= num_bits;
        buf->current_input_byte = (uint16_t) (buf->current_input_byte >> num_bits);
        return 0;
    }
    buf->current_input_byte = (uint16_t) (buf->current_input_byte >> buf->current_input_bits_available);
    if (buf->input_buffer_ptr == buf->input_buffer_end) {
        // Fill buffer
        buf->input_buffer_ptr = 2048;
        buf->input_buffer_end = buf->input_func(buf->input_buffer, buf->input_buffer_ptr, buf->token);
        if (!buf->input_buffer_end) {
            return 1;
        }
        buf->input_buffer_ptr = 0;
    }

    buf->current_input_byte |= (uint16_t) (buf->input_buffer[buf->input_buffer_ptr++] << 8);
    buf->current_input_byte = (uint16_t) (buf->current_input_byte >> (num_bits - buf->current_input_bits_available));
    buf->current_input_bits_available += 8 - num_bits;
    return 0;
}

static int pk_explode_decode_next_token(struct pk_decomp_buffer *buf)
{
    if (buf->current_input_byte & 1) {
        // copy
        if (pk_explode_set_bits_used(buf, 1)) {
            return PK_ERROR_VALUE;
        }
        int index = buf->copy_length_jump_table[buf->current_input_byte & 0xff];
        if (pk_explode_set_bits_used(buf, pk_copy_length_base_bits[index])) {
            return PK_ERROR_VALUE;
        }
        int extra_bits = pk_copy_length_extra_bits[index];
        if (extra_bits) {
            int extra_bits_value = buf->current_input_byte & ((1 << extra_bits) - 1);
            if (pk_explode_set_bits_used(buf, extra_bits) && index + extra_bits_value != 270) {
                return PK_ERROR_VALUE;
            }
            index = pk_copy_length_base_value[index] + extra_bits_value;
        }
        return index + 256;
    } else {
        // literal token
        if (pk_explode_set_bits_used(buf, 1)) {
            return PK_ERROR_VALUE;
        }
        int result = buf->current_input_byte & 0xff;
        if (pk_explode_set_bits_used(buf, 8)) {
            return PK_ERROR_VALUE;
        }
        return result;
    }
}

static int pk_explode_get_copy_offset(struct pk_decomp_buffer *buf, int copy_length)
{
    int index = buf->copy_offset_jump_table[buf->current_input_byte & 0xff];
    if (pk_explode_set_bits_used(buf, pk_copy_offset_bits[index])) {
        return 0;
    }
    int offset;
    if (copy_length == 2) {
        offset = (buf->current_input_byte & 3) | (index << 2);
        if (pk_explode_set_bits_used(buf, 2)) {
            return 0;
        }
    } else {
        offset = (buf->current_input_byte & buf->dictionary_size) | (index << buf->window_size);
        if (pk_explode_set_bits_used(buf, buf->window_size)) {
            return 0;
        }
    }
    return offset + 1;
}

static int pk_explode_data(struct pk_decomp_buffer *buf)
{
    int token;
    buf->output_buffer_ptr = 4096;
    while (1) {
        token = pk_explode_decode_next_token(buf);
        if (token >= PK_ERROR_VALUE - 1) {
            break;
        }
        if (token >= 256) {
            // copy offset
            int length = token - 254;
            int offset = pk_explode_get_copy_offset(buf, length);
            if (!offset) {
                token = PK_ERROR_VALUE;
                break;
            }
            uint8_t *src = &buf->output_buffer[buf->output_buffer_ptr - offset];
            uint8_t *dst = &buf->output_buffer[buf->output_buffer_ptr];
            buf->output_buffer_ptr += length;
            do {
                *dst = *src;
                src++;
                dst++;
            } while (--length > 0);
        } else {
            // literal byte
            buf->output_buffer[buf->output_buffer_ptr++] = (uint8_t) token;
        }
        if (buf->output_buffer_ptr >= 8192) {
            // Flush buffer
            buf->output_func(&buf->output_buffer[4096], 4096, buf->token);
            pk_memcpy(buf->output_buffer, &buf->output_buffer[4096], buf->output_buffer_ptr - 4096);
            buf->output_buffer_ptr -= 4096;
        }
    }
    int remaining_bytes = buf->output_buffer_ptr - 4096;
    if (remaining_bytes > 0) {
        // Flush buffer if needed
        buf->output_func(&buf->output_buffer[4096], remaining_bytes, buf->token);
    }
    return token;
}

static int pk_explode(pk_input_func *input_func, pk_output_func *output_func,
                      struct pk_decomp_buffer *buf, struct pk_token *token)
{
    buf->input_func = input_func;
    buf->output_func = output_func;
    buf->token = token;
    buf->input_buffer_ptr = 2048;
    int bytes_read = buf->input_func(buf->input_buffer, buf->input_buffer_ptr, buf->token);
    buf->input_buffer_end = bytes_read;
    if (bytes_read <= 4) {
        return PK_TOO_FEW_INPUT_BYTES;
    }
    int has_literal_encoding = buf->input_buffer[0];
    buf->window_size = buf->input_buffer[1];
    buf->current_input_byte = buf->input_buffer[2];
    buf->current_input_bits_available = 0;
    buf->input_buffer_ptr = 3;

    if (buf->window_size < 4 || buf->window_size > 6) {
        return PK_INVALID_WINDOWSIZE;
    }

    buf->dictionary_size = 0xFFFF >> (16 - buf->window_size);
    if (has_literal_encoding) {
        return PK_LITERAL_ENCODING_UNSUPPORTED;
    }

    // Decode data for copying bytes
    pk_explode_construct_jump_table(16,
        pk_copy_length_base_bits, pk_copy_length_base_code, buf->copy_length_jump_table);
    pk_explode_construct_jump_table(64,
        pk_copy_offset_bits, pk_copy_offset_code, buf->copy_offset_jump_table);

    int result = pk_explode_data(buf);
    if (result != PK_EOF) {
        return PK_ERROR_DECODING;
    }
    return PK_SUCCESS;
}

static int zip_input_func(uint8_t *buffer, int length, struct pk_token *token)
{
    if (token->stop) {
        return 0;
    }
    if (token->input_ptr >= token->input_length) {
        return 0;
    }

    if (token->input_length - token->input_ptr < length) {
        length = token->input_length - token->input_ptr;
    }
    memcpy(buffer, &token->input_data[token->input_ptr], (size_t) length);
    token->input_ptr += length;
    return length;
}

static void zip_output_func(uint8_t *buffer, int length, struct pk_token *token)
{
    if (token->stop) {
        return;
    }
    if (token->output_ptr >= token->output_length) {
        log_error("COMP2 Out of buffer space.", 0, 0);
        token->stop = 1;
        return;
    }

    if (token->output_length - token->output_ptr >= length) {
        memcpy(&token->output_data[token->output_ptr], buffer, (size_t) length);
        token->output_ptr += length;
    } else {
        log_error("COMP1 Corrupt.", 0, 0);
        token->stop = 1;
    }
}

int zip_decompress(const void *input_buffer, int input_length,
                   void *output_buffer, int *output_length)
{
    struct pk_token token;
    struct pk_decomp_buffer *buf = (struct pk_decomp_buffer *) malloc(sizeof(struct pk_decomp_buffer));
    if (!buf) {
        return 0;
    }
    memset(buf, 0, sizeof(struct pk_decomp_buffer));
    memset(&token, 0, sizeof(struct pk_token));
    token.input_data = (const uint8_t *) input_buffer;
    token.input_length = input_length;
    token.output_data = (uint8_t *) output_buffer;
    token.output_length = *output_length;

    int ok = 1;
    int pk_error = pk_explode(zip_input_func, zip_output_func, buf, &token);
    if (pk_error || token.stop) {
        log_error("COMP Error uncompressing.", 0, 0);
        ok = 0;
    } else {
        *output_length = token.output_ptr;
    }
    free(buf);
    return ok;
}
