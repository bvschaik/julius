#ifndef TRANSLATION_TRANSLATION_H
#define TRANSLATION_TRANSLATION_H

#include "core/locale.h"

#include <stdint.h>

typedef enum {
    TR_NO_PATCH_TITLE,
    TR_NO_PATCH_MESSAGE,
    TR_MISSING_FONTS_TITLE,
    TR_MISSING_FONTS_MESSAGE,
    TR_NO_EDITOR_TITLE,
    TR_NO_EDITOR_MESSAGE,
    TR_INVALID_LANGUAGE_TITLE,
    TR_INVALID_LANGUAGE_MESSAGE,
    TR_BUTTON_OK,
    TR_BUTTON_CANCEL,
    TR_BUTTON_RESET_DEFAULTS,
    TR_BUTTON_CONFIGURE_HOTKEYS,
    TRANSLATION_MAX_KEY
} translation_key;

typedef struct {
    translation_key key;
    const char *string;
} translation_string;

void translation_load(language_type language);

const uint8_t *translation_for(translation_key key);

void translation_english(const translation_string **strings, int *num_strings);

#endif // TRANSLATION_TRANSLATION_H
