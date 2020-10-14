/*-----------------------------------------------------------------------------
 * MurmurHash3 was written by Austin Appleby, and is placed in the public
 * domain.
 *
 * This implementation was written by Shane Day, and is also public domain.
 *
 * This is a portable ANSI C implementation of MurmurHash3_x86_32 (Murmur3A)
 * with support for progressive processing.
 * 
 * This version of MurmurHash3 was adapted for use in Augustus and does not
 * have all the features of the original implementation. Please refer to
 * https://raw.githubusercontent.com/aappleby/smhasher/master/src/PMurHash.h
 * for the original source.
 */

#include <stdint.h>

void PMurHash32_Process(uint32_t *ph1, uint32_t *pcarry, const void *key, int len);
uint32_t PMurHash32_Result(uint32_t h, uint32_t carry, uint32_t total_length);
