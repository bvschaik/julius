#include <stdio.h>

#include "core/locale.h"
#include "core/log.h"
#include "translation/translation.h"

static void check_language(const char *name, language_type language)
{
    log_info("Checking language:", name, 0);
    translation_load(language);
}

int main(void)
{
    check_language("French", LANGUAGE_FRENCH);
    check_language("German", LANGUAGE_GERMAN);
    check_language("Italian", LANGUAGE_ITALIAN);
    check_language("Spanish", LANGUAGE_SPANISH);
    check_language("Japanese", LANGUAGE_JAPANESE);
    check_language("Korean", LANGUAGE_KOREAN);
    check_language("Polish", LANGUAGE_POLISH);
    check_language("Portuguese", LANGUAGE_PORTUGUESE);
    check_language("Russian", LANGUAGE_RUSSIAN);
    check_language("Swedish", LANGUAGE_SWEDISH);
    check_language("Simplified Chinese", LANGUAGE_SIMPLIFIED_CHINESE);
    check_language("Traditional Chinese", LANGUAGE_TRADITIONAL_CHINESE);
    check_language("Czech", LANGUAGE_CZECH);
    check_language("Greek", LANGUAGE_GREEK);
    return 0;
}