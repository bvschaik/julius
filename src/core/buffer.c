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

static int check_size(buffer *buf, size_t size)
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

void buffer_write_raw(buffer *buf, const void *value, size_t size)
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
        uint16_t b0 = buf->data[buf->index++];
        uint16_t b1 = buf->data[buf->index++];
        return (int16_t) (b0 | (b1 << 8));
    } else {
        return 0;
    }
}

int32_t buffer_read_i32(buffer *buf)
{
    if (check_size(buf, 4)) {
        uint32_t b0 = buf->data[buf->index++];
        uint32_t b1 = buf->data[buf->index++];
        uint32_t b2 = buf->data[buf->index++];
        uint32_t b3 = buf->data[buf->index++];
        return (int32_t) (b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
    } else {
        return 0;
    }
}

size_t buffer_read_raw(buffer *buf, void *value, size_t max_size)
{
    size_t size = buf->size - buf->index;
    if (size > max_size) {
        size = max_size;
    }
    memcpy(value, &buf->data[buf->index], size);
    buf->index += size;
    return size;
}

void buffer_skip(buffer *buf, size_t size)
{
    buf->index += size;
}

int buffer_at_end(buffer *buf)
{
    return buf->index >= buf->size;
}

void buffer_init_dynamic(buffer *buf, uint32_t size)
{
    size += sizeof(uint32_t); // Add space for the buffer size
    uint8_t *buf_data = malloc(size);
    buffer_init(buf, buf_data, size);
    buffer_write_u32(buf, size);
}

uint32_t buffer_load_dynamic(buffer *buf)
{
    buffer_set(buf, 0);
    uint32_t size = buffer_read_u32(buf) - sizeof(uint32_t);
    return size;
}

void buffer_init_dynamic_array(buffer *buf, uint32_t array_size, uint32_t element_size)
{
    uint32_t buf_size = (3 * sizeof(uint32_t)) + (array_size * element_size);
    buffer_init_dynamic(buf, buf_size);

    buffer_write_i32(buf, 0); // Skip
    buffer_write_u32(buf, array_size);
    buffer_write_u32(buf, element_size);
}

uint32_t buffer_load_dynamic_array(buffer *buf)
{
    buffer_set(buf, 0);
    buffer_skip(buf, 8); // Skip the buffer size and version
    uint32_t array_size = buffer_read_u32(buf);
    buffer_skip(buf, 4); // Skip the element size
    return array_size;
}
