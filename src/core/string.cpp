#include "core/string.h"

#include <ctype.h>

int string_to_int(const char *str)
{
    static const int multipliers[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000};
    const char *ptr = str;
    int negative = 0;
    int num_chars = 0;
    if (*ptr == '-')
    {
        negative = 1;
        ptr++;
    }
    while (*ptr >= '0' && *ptr <= '9')
    {
        num_chars++;
        ptr++;
    }

    if (num_chars > 8)
    {
        return 0;
    }
    ptr = str;
    if (*ptr == '-')
    {
        ptr++;
    }
    int result = 0;
    while (num_chars)
    {
        --num_chars;
        result += multipliers[num_chars] * (*ptr - '0');
        ptr++;
    }
    if (negative)
    {
        result = -result;
    }
    return result;
}

int string_from_int(char *dst, int value, int force_plus_sign)
{
    int total_chars = 0;
    if (value >= 0)
    {
        if (force_plus_sign)
        {
            dst[0] = '+';
            dst++;
            total_chars = 1;
        }
    }
    else
    {
        dst[0] = '-';
        dst++;
        value = -value;
        total_chars = 1;
    }
    int num_digits;
    if (value < 10)
    {
        num_digits = 1;
    }
    else if (value < 100)
    {
        num_digits = 2;
    }
    else if (value < 1000)
    {
        num_digits = 3;
    }
    else if (value < 10000)
    {
        num_digits = 4;
    }
    else if (value < 100000)
    {
        num_digits = 5;
    }
    else if (value < 1000000)
    {
        num_digits = 6;
    }
    else if (value < 10000000)
    {
        num_digits = 7;
    }
    else if (value < 100000000)
    {
        num_digits = 8;
    }
    else if (value < 1000000000)
    {
        num_digits = 9;
    }
    else
    {
        num_digits = 0;
    }

    total_chars += num_digits;

    dst[num_digits] = 0;
    while (--num_digits >= 0)
    {
        dst[num_digits] = (uint8_t) (value % 10 + '0');
        value /= 10;
    }

    return total_chars;
}

int string_compare_case_insensitive(const char *a, const char *b)
{
    while (*a && *b)
    {
        int aa = tolower(*a);
        int bb = tolower(*b);
        if (aa != bb)
        {
            return aa - bb;
        }
        ++a;
        ++b;
    }
    if (*a)
    {
        return 1;
    }
    if (*b)
    {
        return -1;
    }
    return 0;
}
