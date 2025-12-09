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
    LANGUAGE_JAPANESE,
    LANGUAGE_KOREAN,
    LANGUAGE_POLISH,
    LANGUAGE_PORTUGUESE,
    LANGUAGE_RUSSIAN,
    LANGUAGE_SWEDISH,
    LANGUAGE_SIMPLIFIED_CHINESE,
    LANGUAGE_TRADITIONAL_CHINESE,
    // Fan translations
    LANGUAGE_CZECH,
    LANGUAGE_GREEK,
    LANGUAGE_MAX_ITEMS
} language_type;

/**
 * Determines language based on the language file
 * @return Guessed language
 */
language_type locale_determine_language(void);

/**
 * if no language set before, returns LANGUAGE_UNKNOWN
 * @return Last determined language
 */
language_type locale_last_determined_language(void);

/**
 * Check whether to write the year before the 'AD' part or the other way around
 * @return Boolean true if years should be written as "200 AD", false for "AD 200"
 */
int locale_year_before_ad(void);

/**
 * Check whether to use the string 'Dn' from the 'funds' top menu when used for money amounts
 * @return Boolean true for the translated version, false to use fixed string 'Dn'
 */
int locale_translate_money_dn(void);

/**
 * Returns the indent size to use for paragraphs in rich text
 * @return Indent in pixels
 */
int locale_paragraph_indent(void);

/**
 * Returns whether we should use localized filenames for the "Citizen.sav", etc autosaves
 * @return Boolean true if the filenames should be translated, false if we should use English
 */
int locale_translate_rank_autosaves(void);

#endif // CORE_LOCALE_H
