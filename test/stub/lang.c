#include "core/lang.h"
#include "core/encoding.h"

static uint8_t EMPTY[] = {0};

static lang_message msg;

int lang_load(int is_editor)
{
    return 1;
}

const uint8_t *lang_get_string(int group, int index)
{
    return EMPTY;
}

const lang_message *lang_get_message(int id)
{
    msg.content.text = EMPTY;
    return &msg;
}

void font_set_encoding(encoding_type encoding)
{}

void translation_load(language_type language)
{}
