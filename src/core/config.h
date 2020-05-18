#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

#define CONFIG_STRING_VALUE_MAX 64

typedef enum {
    CONFIG_GP_FIX_IMMIGRATION_BUG,
    CONFIG_GP_FIX_100_YEAR_GHOSTS,
    CONFIG_GP_FIX_EDITOR_EVENTS,
    CONFIG_UI_SIDEBAR_INFO,
    CONFIG_UI_SHOW_INTRO_VIDEO,
    CONFIG_UI_SMOOTH_SCROLLING,
    CONFIG_UI_DISABLE_MAP_DRAG,
    CONFIG_UI_VISUAL_FEEDBACK_ON_DELETE,
    CONFIG_UI_ALLOW_CYCLING_TEMPLES,
    CONFIG_UI_SHOW_WATER_STRUCTURE_RANGE,
    CONFIG_UI_SHOW_CONSTRUCTION_SIZE,
    CONFIG_UI_HIGHLIGHT_LEGIONS,
    CONFIG_UI_SHOW_MILITARY_SIDEBAR,
    CONFIG_MAX_ENTRIES
} config_key;

typedef enum {
    CONFIG_STRING_UI_LANGUAGE_DIR,
    CONFIG_STRING_MAX_ENTRIES
} config_string_key;

/**
 * Get an integer config value
 * @param key Integer key
 * @return Config value
 */
int config_get(config_key key);

/**
 * Set an integer config value
 * @param key Integer key
 * @param value Value to set
 */
void config_set(config_key key, int value);

/**
 * Get a string config value
 * @param key String key
 * @return Config value, is always non-NULL but may be an empty string
 */
const char *config_get_string(config_string_key key);

/**
 * Set a string config value
 * @param key String key
 * @param value Value to set
 */
void config_set_string(config_string_key key, const char *value);

/**
 * Set a default config value
 * @param key Integer key
 * @return Default config value
 */
int config_get_default_value(config_key key);

/**
 * Get a string default config value
 * @param key String key
 * @return Default config value, is always non-NULL but may be an empty string
 */
const char *config_get_default_string_value(config_string_key key);

/**
 * Load config from file
 */
void config_load(void);

/**
 * Save config to file
 */
void config_save(void);

#endif // CORE_CONFIG_H
