#include "core/buffer.h"

#include <string.h>

#define CHECK_WRITE(b, s) if ((b->index + s) > b->size) return
#define CHECK_READ(b, s) if ((b->index + s) > b->size) return 0

void buffer_init(buffer *buffer, void *data, int size)
{
    buffer->data = data;
    buffer->size = size;
    buffer->index = 0;
}

void buffer_write_u8(buffer *buffer, uint8_t value)
{
    CHECK_WRITE(buffer, 1);
    buffer->data[buffer->index++] = value;
}

void buffer_write_u16(buffer *buffer, uint16_t value)
{
    CHECK_WRITE(buffer, 2);
    buffer->data[buffer->index++] = value & 0xff;
    buffer->data[buffer->index++] = (value >> 8) & 0xff;
}

void buffer_write_u32(buffer *buffer, uint32_t value)
{
    CHECK_WRITE(buffer, 4);
    buffer->data[buffer->index++] = value & 0xff;
    buffer->data[buffer->index++] = (value >> 8) & 0xff;
    buffer->data[buffer->index++] = (value >> 16) & 0xff;
    buffer->data[buffer->index++] = (value >> 24) & 0xff;
}

void buffer_write_i8(buffer *buffer, int8_t value)
{
    CHECK_WRITE(buffer, 1);
    buffer->data[buffer->index++] = value & 0xff;
}

void buffer_write_i16(buffer *buffer, int16_t value)
{
    CHECK_WRITE(buffer, 2);
    buffer->data[buffer->index++] = value & 0xff;
    buffer->data[buffer->index++] = (value >> 8) & 0xff;
}

void buffer_write_i32(buffer *buffer, int32_t value)
{
    CHECK_WRITE(buffer, 4);
    buffer->data[buffer->index++] = value & 0xff;
    buffer->data[buffer->index++] = (value >> 8) & 0xff;
    buffer->data[buffer->index++] = (value >> 16) & 0xff;
    buffer->data[buffer->index++] = (value >> 24) & 0xff;
}

void buffer_write_raw(buffer *buffer, const void *value, int size)
{
    CHECK_WRITE(buffer, size);
    memcpy(&buffer->data[buffer->index], value, size);
    buffer->index += size;
}

uint8_t buffer_read_u8(buffer *buffer)
{
    CHECK_READ(buffer, 1);
    return buffer->data[buffer->index++];
}

uint16_t buffer_read_u16(buffer *buffer)
{
    CHECK_READ(buffer, 2);
    return (uint16_t) (
        buffer->data[buffer->index++] |
        (buffer->data[buffer->index++] << 8)
    );
}

uint32_t buffer_read_u32(buffer *buffer)
{
    CHECK_READ(buffer, 4);
    return (uint32_t) (
        buffer->data[buffer->index++] |
        (buffer->data[buffer->index++] << 8) |
        (buffer->data[buffer->index++] << 16) |
        (buffer->data[buffer->index++] << 24)
    );
}

int8_t buffer_read_i8(buffer *buffer)
{
    CHECK_READ(buffer, 1);
    return (int8_t) buffer->data[buffer->index++];
}

int16_t buffer_read_i16(buffer *buffer)
{
    CHECK_READ(buffer, 2);
    return (int16_t) (
        buffer->data[buffer->index++] |
        (buffer->data[buffer->index++] << 8)
    );
}

int32_t buffer_read_i32(buffer *buffer)
{
    CHECK_READ(buffer, 4);
    return (int32_t) (
        buffer->data[buffer->index++] |
        (buffer->data[buffer->index++] << 8) |
        (buffer->data[buffer->index++] << 16) |
        (buffer->data[buffer->index++] << 24)
    );
}

void buffer_read_raw(buffer *buffer, void *value, int size)
{
    CHECK_WRITE(buffer, size);
    memcpy(value, &buffer->data[buffer->index], size);
    buffer->index += size;
}

void buffer_skip(buffer *buffer, int size)
{
    buffer->index += size;
}

