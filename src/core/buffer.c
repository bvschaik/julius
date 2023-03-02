#include "core/buffer.h"

#include <stdlib.h>
#include <string.h>

void buffer_init(buffer *buf, void *data, int size)
{
    buf->data = data;
    buf->size = size;
    buf->index = 0;
    buf->overflow = 0;
}

void buffer_reset(buffer *buf)
{
    buf->index = 0;
    buf->overflow = 0;
}

void buffer_set(buffer *buf, int offset)
{
    buf->index = offset;
}

static int check_size(buffer *buf, int size)
{
    if (buf->index + size > buf->size) {
        buf->overflow = 1;
        return 0;
    }
    return 1;
}

void buffer_write_u8(buffer *buf, uint8_t value)
{
    if (check_size(buf, 1)) {
        buf->data[buf->index++] = value;
    }
}

void buffer_write_u16(buffer *buf, uint16_t value)
{
    if (check_size(buf, 2)) {
        buf->data[buf->index++] = value & 0xff;
        buf->data[buf->index++] = (value >> 8) & 0xff;
    }
}

void buffer_write_u32(buffer *buf, uint32_t value)
{
    if (check_size(buf, 4)) {
        buf->data[buf->index++] = value & 0xff;
        buf->data[buf->index++] = (value >> 8) & 0xff;
        buf->data[buf->index++] = (value >> 16) & 0xff;
        buf->data[buf->index++] = (value >> 24) & 0xff;
    }
}

void buffer_write_i8(buffer *buf, int8_t value)
{
    if (check_size(buf, 1)) {
        buf->data[buf->index++] = value & 0xff;
    }
}

void buffer_write_i16(buffer *buf, int16_t value)
{
    if (check_size(buf, 2)) {
        buf->data[buf->index++] = value & 0xff;
        buf->data[buf->index++] = (value >> 8) & 0xff;
    }
}

void buffer_write_i32(buffer *buf, int32_t value)
{
    if (check_size(buf, 4)) {
        buf->data[buf->index++] = value & 0xff;
        buf->data[buf->index++] = (value >> 8) & 0xff;
        buf->data[buf->index++] = (value >> 16) & 0xff;
        buf->data[buf->index++] = (value >> 24) & 0xff;
    }
}

void buffer_write_raw(buffer *buf, const void *value, int size)
{
    if (check_size(buf, size)) {
        memcpy(&buf->data[buf->index], value, size);
        buf->index += size;
    }
}

uint8_t buffer_read_u8(buffer *buf)
{
    if (check_size(buf, 1)) {
        return buf->data[buf->index++];
    } else {
        return 0;
    }
}

uint16_t buffer_read_u16(buffer *buf)
{
    if (check_size(buf, 2)) {
        uint8_t b0 = buf->data[buf->index++];
        uint8_t b1 = buf->data[buf->index++];
        return (uint16_t) (b0 | (b1 << 8));
    } else {
        return 0;
    }
}

uint32_t buffer_read_u32(buffer *buf)
{
    if (check_size(buf, 4)) {
        uint8_t b0 = buf->data[buf->index++];
        uint8_t b1 = buf->data[buf->index++];
        uint8_t b2 = buf->data[buf->index++];
        uint8_t b3 = buf->data[buf->index++];
        return (uint32_t) (b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
    } else {
        return 0;
    }
}

int8_t buffer_read_i8(buffer *buf)
{
    if (check_size(buf, 1)) {
        return (int8_t) buf->data[buf->index++];
    } else {
        return 0;
    }
}

int16_t buffer_read_i16(buffer *buf)
{
    if (check_size(buf, 2)) {
        uint8_t b0 = buf->data[buf->index++];
        uint8_t b1 = buf->data[buf->index++];
        return (int16_t) (b0 | (b1 << 8));
    } else {
        return 0;
    }
}

int32_t buffer_read_i32(buffer *buf)
{
    if (check_size(buf, 4)) {
        uint8_t b0 = buf->data[buf->index++];
        uint8_t b1 = buf->data[buf->index++];
        uint8_t b2 = buf->data[buf->index++];
        uint8_t b3 = buf->data[buf->index++];
        return (int32_t) (b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
    } else {
        return 0;
    }
}

int buffer_read_raw(buffer *buf, void *value, int max_size)
{
    int size = buf->size - buf->index;
    if (size > max_size) {
        size = max_size;
    }
    memcpy(value, &buf->data[buf->index], size);
    buf->index += size;
    return size;
}

void buffer_skip(buffer *buf, int size)
{
    buf->index += size;
}

int buffer_at_end(buffer *buf)
{
    return buf->index >= buf->size;
}

void buffer_init_dynamic_piece(buffer *buffer, int32_t version, int32_t array_size, int32_t struct_size)
{
    int32_t buf_size = (4 * sizeof(int32_t)) + (array_size * struct_size);
    uint8_t *buf_data = malloc(buf_size);
    buffer_init(buffer, buf_data, buf_size);

    buffer_write_i32(buffer, buf_size);
    buffer_write_i32(buffer, version);
    buffer_write_i32(buffer, array_size);
    buffer_write_i32(buffer, struct_size);
}

void buffer_load_dynamic_piece_header_data(buffer *buffer, int32_t *size, int32_t *version, int32_t *array_size, int32_t *struct_size)
{
    buffer_set(buffer, 0);
    *size = buffer_read_i32(buffer);
    *version = buffer_read_i32(buffer);
    *array_size = buffer_read_i32(buffer);
    *struct_size = buffer_read_i32(buffer);
}
