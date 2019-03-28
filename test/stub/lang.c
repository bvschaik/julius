#include "core/lang.h"

static uint8_t EMPTY[] = {0};

static lang_message msg;

int lang_load(const char *text_filename, const char *message_filename)
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

encoding_type lang_encoding(void)
{
    return ENCODING_WESTERN_EUROPE;
}

void font_set_encoding(int encoding)
{}
