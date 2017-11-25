#include "core/buffer.h"

#include <string.h>

void buffer_init(buffer *buffer, void *data, int size)
{
    buffer->data = data;
    buffer->size = size;
    buffer->index = 0;
    buffer->overflow = 0;
}

void buffer_reset(buffer *buffer)
{
    buffer->index = 0;
    buffer->overflow = 0;
}

void buffer_set(buffer *buffer, int offset)
{
    buffer->index = offset;
}

static int check_size(buffer *buffer, int size)
{
    if (buffer->index + size > buffer->size)
    {
        buffer->overflow = 1;
        return 0;
    }
    return 1;
}

void buffer_write_u8(buffer *buffer, uint8_t value)
{
    if (check_size(buffer, 1))
    {
        buffer->data[buffer->index++] = value;
    }
}

void buffer_write_u16(buffer *buffer, uint16_t value)
{
    if (check_size(buffer, 2))
    {
        buffer->data[buffer->index++] = value & 0xff;
        buffer->data[buffer->index++] = (value >> 8) & 0xff;
    }
}

void buffer_write_u32(buffer *buffer, uint32_t value)
{
    if (check_size(buffer, 4))
    {
        buffer->data[buffer->index++] = value & 0xff;
        buffer->data[buffer->index++] = (value >> 8) & 0xff;
        buffer->data[buffer->index++] = (value >> 16) & 0xff;
        buffer->data[buffer->index++] = (value >> 24) & 0xff;
    }
}

void buffer_write_i8(buffer *buffer, int8_t value)
{
    if (check_size(buffer, 1))
    {
        buffer->data[buffer->index++] = value & 0xff;
    }
}

void buffer_write_i16(buffer *buffer, int16_t value)
{
    if (check_size(buffer, 2))
    {
        buffer->data[buffer->index++] = value & 0xff;
        buffer->data[buffer->index++] = (value >> 8) & 0xff;
    }
}

void buffer_write_i32(buffer *buffer, int32_t value)
{
    if (check_size(buffer, 4))
    {
        buffer->data[buffer->index++] = value & 0xff;
        buffer->data[buffer->index++] = (value >> 8) & 0xff;
        buffer->data[buffer->index++] = (value >> 16) & 0xff;
        buffer->data[buffer->index++] = (value >> 24) & 0xff;
    }
}

void buffer_write_raw(buffer *buffer, const void *value, int size)
{
    if (check_size(buffer, size))
    {
        memcpy(&buffer->data[buffer->index], value, size);
        buffer->index += size;
    }
}

uint8_t buffer_read_u8(buffer *buffer)
{
    if (check_size(buffer, 1))
    {
        return buffer->data[buffer->index++];
    }
    else
    {
        return 0;
    }
}

uint16_t buffer_read_u16(buffer *buffer)
{
    if (check_size(buffer, 2))
    {
        uint8_t b0 = buffer->data[buffer->index++];
        uint8_t b1 = buffer->data[buffer->index++];
        return (uint16_t) (b0 | (b1 << 8));
    }
    else
    {
        return 0;
    }
}

uint32_t buffer_read_u32(buffer *buffer)
{
    if (check_size(buffer, 4))
    {
        uint8_t b0 = buffer->data[buffer->index++];
        uint8_t b1 = buffer->data[buffer->index++];
        uint8_t b2 = buffer->data[buffer->index++];
        uint8_t b3 = buffer->data[buffer->index++];
        return (uint32_t) (b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
    }
    else
    {
        return 0;
    }
}

int8_t buffer_read_i8(buffer *buffer)
{
    if (check_size(buffer, 1))
    {
        return (int8_t) buffer->data[buffer->index++];
    }
    else
    {
        return 0;
    }
}

int16_t buffer_read_i16(buffer *buffer)
{
    if (check_size(buffer, 2))
    {
        uint8_t b0 = buffer->data[buffer->index++];
        uint8_t b1 = buffer->data[buffer->index++];
        return (int16_t) (b0 | (b1 << 8));
    }
    else
    {
        return 0;
    }
}

int32_t buffer_read_i32(buffer *buffer)
{
    if (check_size(buffer, 4))
    {
        uint8_t b0 = buffer->data[buffer->index++];
        uint8_t b1 = buffer->data[buffer->index++];
        uint8_t b2 = buffer->data[buffer->index++];
        uint8_t b3 = buffer->data[buffer->index++];
        return (int32_t) (b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
    }
    else
    {
        return 0;
    }
}

int buffer_read_raw(buffer *buffer, void *value, int max_size)
{
    int size = buffer->size - buffer->index;
    if (size > max_size)
    {
        size = max_size;
    }
    memcpy(value, &buffer->data[buffer->index], size);
    buffer->index += size;
    return size;
}

void buffer_skip(buffer *buffer, int size)
{
    buffer->index += size;
}
