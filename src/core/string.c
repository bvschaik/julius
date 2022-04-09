#include "core/string.h"

#include "core/calc.h"

int string_equals(const uint8_t *a, const uint8_t *b)
{
    while (*a && *b && *a == *b) {
        ++a;
        ++b;
    }
    if (*a == 0 && *b == 0) {
        return 1;
    } else {
        return 0;
    }
}

uint8_t* string_copy(const uint8_t *src, uint8_t *dst, int maxlength)
{
    int length = 0;
    while (length < maxlength && *src) {
        *dst = *src;
        src++;
        dst++;
        length++;
    }
    if (length == maxlength) {
        dst--;
    }
    *dst = 0;
    return dst;
}

int string_length(const uint8_t *str)
{
    int length = 0;
    while (*str) {
        length++;
        str++;
    }
    return length;
}

const uint8_t *string_from_ascii(const char *str)
{
    const char *s = str;
    while (*s) {
        if (*s & 0x80) {
            return 0;
        }
        s++;
    }
    return (const uint8_t *) str;
}

int string_to_int(const uint8_t *str)
{
    static const int multipliers[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000};
    const uint8_t *ptr = str;
    int negative = 0;
    int num_chars = 0;
    if (*ptr == '-') {
        negative = 1;
        ptr++;
    }
    while (*ptr >= '0' && *ptr <= '9') {
        num_chars++;
        ptr++;
    }

    if (num_chars > 8) {
        return 0;
    }
    ptr = str;
    if (*ptr == '-') {
        ptr++;
    }
    int result = 0;
    while (num_chars) {
        --num_chars;
        result += multipliers[num_chars] * (*ptr - '0');
        ptr++;
    }
    if (negative) {
        result = -result;
    }
    return result;
}

int string_from_int(uint8_t *dst, int value, int force_plus_sign)
{
    int total_chars = 0;
    if (value >= 0) {
        if (force_plus_sign) {
            dst[0] = '+';
            dst++;
            total_chars = 1;
        }
    } else {
        dst[0] = '-';
        dst++;
        value = -value;
        total_chars = 1;
    }
    int num_digits = calc_digits_in_number(value);
    total_chars += num_digits;

    dst[num_digits] = 0;
    int digits_written = 0;
    while (--num_digits >= 0) {
        dst[num_digits] = (uint8_t) (value % 10 + '0');
        value /= 10;
        ++digits_written;
    }

    return total_chars;
}
