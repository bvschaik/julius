#ifndef CORE_ENCODING_H
#define CORE_ENCODING_H

#include "core/locale.h"

#include <stdint.h>

/**
 * Encoding type
 */
typedef enum {
    ENCODING_WESTERN_EUROPE = 1252,
    ENCODING_EASTERN_EUROPE = 1250,
    ENCODING_CYRILLIC = 1251,
    ENCODING_TRADITIONAL_CHINESE = 950,
    ENCODING_KOREAN = 949
} encoding_type;

/**
 * Determines the encoding used for the language files (= game),
 * and sets up conversion routines for utf-8 strings
 * @param language Game language
 * @return encoding (best guess)
 */
encoding_type encoding_determine(language_type language);

/**
 * Returns the determined internal encoding type
 * @return encoding (best guess)
 */
encoding_type encoding_get(void);

/**
 * Checks whether the character can be displayed using the internal encoding
 * @param utf8_char UTF-8 encoded character
 * @return boolean true when the character can be displayed, false otherwise
 */
int encoding_can_display(const char *utf8_char);

/**
 * Converts the internally-encoded input to UTF-8 output
 * @param input Input to convert, encoded using encoding_get()
 * @param output Output buffer to store the UTF-8 encoded input
 * @param output_length Length of the output buffer
 * @param decompose Whether the unicode characters should be in decomposed form
 */
void encoding_to_utf8(const uint8_t *input, char *output, int output_length, int decompose);

/**
 * Converts the internally-encoded input to UTF-8 output
 * @param input Input to convert, UTF-8 encoded
 * @param output Output buffer to store the internally encoded input
 * @param output_length Length of the output buffer
 */
void encoding_from_utf8(const char *input, uint8_t *output, int output_length);

#endif // CORE_ENCODING_H
