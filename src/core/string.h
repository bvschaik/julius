#ifndef CORE_STRING_H
#define CORE_STRING_H

#include <stdint.h>
#include <cstring>

/**
 * Converts the string to integer
 * @return integer
 */
int string_to_int(const char *str);

/**
 * Converts integer to string
 * @param dst Output string
 * @param value Value to write
 * @param force_plus_sign Force plus sign in front of positive value
 * @return Total number of characters written to dst
 */
int string_from_int(char *dst, int value, int force_plus_sign);

/**
 * Compares the two strings case insensitively, used exclusively for filenames
 * @param a String A
 * @param b String B
 * @return Negative if A < B, positive if A > B, zero if A == B
 */
int string_compare_case_insensitive(const char *a, const char *b);

#endif // CORE_STRING_H
