#ifndef CORE_LOCALE_H
#define CORE_LOCALE_H

/**
 * Language type
 */
typedef enum {
    LANGUAGE_UNKNOWN,
    // Official Sierra versions
    LANGUAGE_ENGLISH,
    LANGUAGE_FRENCH,
    LANGUAGE_GERMAN,
    LANGUAGE_ITALIAN,
    LANGUAGE_SPANISH,
    // Translations done by other publishers
    LANGUAGE_KOREAN,
    LANGUAGE_POLISH,
    LANGUAGE_PORTUGUESE,
    LANGUAGE_RUSSIAN,
    LANGUAGE_SWEDISH,
    LANGUAGE_SIMPLIFIED_CHINESE,
    LANGUAGE_TRADITIONAL_CHINESE
} language_type;

/**
 * Determines language based on the language file
 * @return Guessed language
 */
language_type locale_determine_language(void);

/**
 * Check whether to write the year before the 'AD' part or the other way around
 * @return Boolean true if years should be written as "200 AD", false for "AD 200"
 */
int locale_year_before_ad(void);

/**
 * Returns whether we should use localized filenames for the "Citizen.sav", etc autosaves
 * @return Boolean true if the filenames should be translated, false if we should use English
 */
int locale_translate_rank_autosaves(void);

#endif // CORE_LOCALE_H
