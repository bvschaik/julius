#include "translation/common.h"
#include "translation/translation.h"

static translation_string all_strings[] = {
};

void translation_japanese(const translation_string **strings, int *num_strings)
{
    *strings = all_strings;
    *num_strings = sizeof(all_strings) / sizeof(translation_string);
}
