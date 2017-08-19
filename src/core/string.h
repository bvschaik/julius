#ifndef STRING_H
#define STRING_H

#include <stdint.h>

/**
 * @file
 * String conversion functions.
 */

/**
 * Copies a string
 * @param str Source string
 * @param dst Destination string
 * @param maxlength Maximum length of the destination string
 */
void string_copy(const uint8_t *src, uint8_t *dst, int maxlength);

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

/**
 * Compares the two strings case insensitively
 * @param a String A
 * @param b String B
 * @return Negative if A < B, positive if A > B, zero if A == B
 */
int string_compare_case_insensitive(const char *a, const char *b);

#endif // STRING_H
