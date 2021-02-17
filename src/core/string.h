#ifndef CORE_STRING_H
#define CORE_STRING_H

#include <stdint.h>

/**
 * @file
 * String conversion functions.
 */

/**
 * Checks if the two strings are equal
 * @param a String A
 * @param b String B
 * @return Boolean true if the strings are equal, false if they differ
 */
int string_equals(const uint8_t *a, const uint8_t *b);

/**
 * Copies a string
 * @param src Source string
 * @param dst Destination string
 * @param maxlength Maximum length of the destination string
 */
void string_copy(const uint8_t *src, uint8_t *dst, int maxlength);

/**
 * Determines the length of the string
 * @param str String
 * @return Length of the string
 */
int string_length(const uint8_t *str);

/**
 * Convert (cast) C-string to internal string.
 * Only use this for known ASCII-only strings!
 * @param str C string
 * @return Game string, or NULL if non-ascii values are found in str
 */
const uint8_t *string_from_ascii(const char *str);

/**
 * Converts the string to integer
 * @return integer
 */
int string_to_int(const uint8_t *str);

/**
 * Converts integer to string
 * @param dst Output string
 * @param value Value to write
 * @param force_plus_sign Force plus sign in front of positive value
 * @return Total number of characters written to dst
 */
int string_from_int(uint8_t *dst, int value, int force_plus_sign);

#endif // CORE_STRING_H
