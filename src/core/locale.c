#include "locale.h"

#include "core/lang.h"
#include "core/log.h"
#include "core/string.h"

#include <stdint.h>

static const uint8_t NEW_GAME_ENGLISH[] = { 0x4e, 0x65, 0x77, 0x20, 0x47, 0x61, 0x6d, 0x65, 0 }; // New Game
static const uint8_t NEW_GAME_FRENCH[] = { 0x4e, 0x6f, 0x75, 0x76, 0x65, 0x6c, 0x6c, 0x65, 0x20, 0x70, 0x61, 0x72, 0x74, 0x69, 0x65, 0 }; // Nouvelle partie
static const uint8_t NEW_GAME_GERMAN[] = { 0x4e, 0x65, 0x75, 0x65, 0x73, 0x20, 0x53, 0x70, 0x69, 0x65, 0x6c, 0 }; // Neues Spiel
static const uint8_t NEW_GAME_ITALIAN[] = { 0x4e, 0x75, 0x6f, 0x76, 0x61, 0x20, 0x70, 0x61, 0x72, 0x74, 0x69, 0x74, 0x61, 0 }; // Nuova partita
static const uint8_t NEW_GAME_SPANISH[] = { 0x4e, 0x75, 0x65, 0x76, 0x61, 0x20, 0x70, 0x61, 0x72, 0x74, 0x69, 0x64, 0x61, 0 }; // Nueva partida
static const uint8_t NEW_GAME_PORTUGUESE[] = { 0x4e, 0x6f, 0x76, 0x6f, 0x20, 0x6a, 0x6f, 0x67, 0x6f, 0 }; // Novo jogo
static const uint8_t NEW_GAME_POLISH[] = { 0x4e, 0x6f, 0x77, 0x61, 0x20, 0x67, 0x72, 0x61, 0 }; // Nowa gra
static const uint8_t NEW_GAME_RUSSIAN[] = { 0xcd, 0xee, 0xe2, 0xe0, 0xff, 0x20, 0xe8, 0xe3, 0xf0, 0xe0, 0 };
static const uint8_t NEW_GAME_SWEDISH[] = { 0x4e, 0x79, 0x74, 0x74, 0x20, 0x73, 0x70, 0x65, 0x6c, 0}; // Nytt spel
static const uint8_t NEW_GAME_TRADITIONAL_CHINESE[] = { 0x83, 0x80, 0x20, 0x84, 0x80, 0x20, 0x85, 0x80, 0 };
static const uint8_t NEW_GAME_SIMPLIFIED_CHINESE[] = { 0x82, 0x80, 0x20, 0x83, 0x80, 0x20, 0x84, 0x80, 0 };
static const uint8_t NEW_GAME_KOREAN[] = { 0xbb, 0xf5, 0x20, 0xb0, 0xd4, 0xc0, 0xd3, 0 };

static struct {
    language_type last_determined_language;
} data;

static language_type determine_language(void)
{
    // Dirty way to check the language, but there's not really another way:
    // Check if the string for "New game" is in one of the supported languages
    const uint8_t *new_game_string = lang_get_string(1, 1);
    if (string_equals(NEW_GAME_ENGLISH, new_game_string)) {
        return LANGUAGE_ENGLISH;
    } else if (string_equals(NEW_GAME_FRENCH, new_game_string)) {
        return LANGUAGE_FRENCH;
    } else if (string_equals(NEW_GAME_GERMAN, new_game_string)) {
        return LANGUAGE_GERMAN;
    } else if (string_equals(NEW_GAME_ITALIAN, new_game_string)) {
        return LANGUAGE_ITALIAN;
    } else if (string_equals(NEW_GAME_SPANISH, new_game_string)) {
        return LANGUAGE_SPANISH;
    } else if (string_equals(NEW_GAME_PORTUGUESE, new_game_string)) {
        return LANGUAGE_PORTUGUESE;
    } else if (string_equals(NEW_GAME_POLISH, new_game_string)) {
        return LANGUAGE_POLISH;
    } else if (string_equals(NEW_GAME_RUSSIAN, new_game_string)) {
        return LANGUAGE_RUSSIAN;
    } else if (string_equals(NEW_GAME_SWEDISH, new_game_string)) {
        return LANGUAGE_SWEDISH;
    } else if (string_equals(NEW_GAME_TRADITIONAL_CHINESE, new_game_string)) {
        return LANGUAGE_TRADITIONAL_CHINESE;
    } else if (string_equals(NEW_GAME_SIMPLIFIED_CHINESE, new_game_string)) {
        return LANGUAGE_SIMPLIFIED_CHINESE;
    } else if (string_equals(NEW_GAME_KOREAN, new_game_string)) {
        return LANGUAGE_KOREAN;
    } else {
        return LANGUAGE_UNKNOWN;
    }
}

static void log_language(void)
{
    const char *desc;
    switch (data.last_determined_language) {
        case LANGUAGE_ENGLISH: desc = "English"; break;
        case LANGUAGE_FRENCH: desc = "French"; break;
        case LANGUAGE_GERMAN: desc = "German"; break;
        case LANGUAGE_ITALIAN: desc = "Italian"; break;
        case LANGUAGE_SPANISH: desc = "Spanish"; break;
        case LANGUAGE_POLISH: desc = "Polish"; break;
        case LANGUAGE_PORTUGUESE: desc = "Portuguese"; break;
        case LANGUAGE_RUSSIAN: desc = "Russian"; break;
        case LANGUAGE_SWEDISH: desc = "Swedish"; break;
        case LANGUAGE_TRADITIONAL_CHINESE: desc = "Traditional Chinese"; break;
        case LANGUAGE_SIMPLIFIED_CHINESE: desc = "Simplified Chinese"; break;
        case LANGUAGE_KOREAN: desc = "Korean"; break;
        default: desc = "Unknown"; break;
    }
    log_info("Detected language:", desc, 0);
}

language_type locale_determine_language(void)
{
    data.last_determined_language = determine_language();
    log_language();
    return data.last_determined_language;
}

int locale_year_before_ad(void)
{
    // In all languages it's "200 AD" except for English
    return data.last_determined_language != LANGUAGE_ENGLISH;
}

int locale_translate_rank_autosaves(void)
{
    switch (data.last_determined_language) {
        case LANGUAGE_ENGLISH:
        case LANGUAGE_FRENCH:
        case LANGUAGE_GERMAN:
        case LANGUAGE_ITALIAN:
        case LANGUAGE_POLISH:
        case LANGUAGE_PORTUGUESE:
        case LANGUAGE_SPANISH:
        case LANGUAGE_SWEDISH:
        case LANGUAGE_RUSSIAN:
            return 1;

        case LANGUAGE_KOREAN:
        case LANGUAGE_TRADITIONAL_CHINESE: // original adds 01_ prefixes
        case LANGUAGE_SIMPLIFIED_CHINESE:
        default:
            return 0;
    }
}
