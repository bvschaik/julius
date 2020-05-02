#include "translation.h"

#include "core/encoding.h"
#include "core/log.h"
#include "core/string.h"

#include <string.h>

#define BUFFER_SIZE 100000

static struct {
    const uint8_t *strings[TRANSLATION_MAX_KEY];
    uint8_t buffer[BUFFER_SIZE];
    int buf_index;
} data;

static void set_strings(const translation_string *strings, int num_strings)
{
    for (int i = 0; i < num_strings; i++) {
        const translation_string *string = &strings[i];
        if (data.strings[string->key]) {
            continue;
        }
        int length_left = BUFFER_SIZE - data.buf_index;
        encoding_from_utf8(string->string, &data.buffer[data.buf_index], length_left);
        data.strings[string->key] = &data.buffer[data.buf_index];
        data.buf_index += 1 + string_length(&data.buffer[data.buf_index]);
    }
}

void translation_load(language_type language)
{
    const translation_string *strings = 0;
    int num_strings = 0;
    const translation_string *default_strings;
    int num_default_strings;
    translation_english(&default_strings, &num_default_strings);

    switch (language) {
        case LANGUAGE_ENGLISH:
        case LANGUAGE_UNKNOWN:
        default:
            translation_english(&strings, &num_strings);
            break;
        case LANGUAGE_FRENCH:
            translation_french(&strings, &num_strings);
            break;
        case LANGUAGE_GERMAN:
            translation_german(&strings, &num_strings);
            break;
        case LANGUAGE_PORTUGUESE:
            translation_portuguese(&strings, &num_strings);
            break;
        case LANGUAGE_SPANISH:
            translation_spanish(&strings, &num_strings);
            break;
    }

    memset(data.strings, 0, sizeof(data.strings));
    data.buf_index = 0;
    set_strings(strings, num_strings);
    set_strings(default_strings, num_default_strings);

    log_info("Memory used for translation", 0, data.buf_index);
}

const uint8_t *translation_for(translation_key key)
{
    return data.strings[key];
}
