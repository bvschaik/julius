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
    CONFIG_UI_WALKER_WAYPOINTS,
    CONFIG_UI_VISUAL_FEEDBACK_ON_DELETE,
    CONFIG_UI_SHOW_WATER_STRUCTURE_RANGE,
    CONFIG_UI_SHOW_CONSTRUCTION_SIZE,
    CONFIG_GP_CH_GRANDFESTIVAL,
    CONFIG_GP_CH_JEALOUS_GODS,
    CONFIG_GP_CH_GLOBAL_LABOUR,
    CONFIG_GP_CH_SCHOOL_WALKERS,
    CONFIG_GP_CH_RETIRE_AT_60,
    CONFIG_GP_CH_FIXED_WORKERS,
    CONFIG_GP_CH_EXTRA_FORTS,
    CONFIG_GP_CH_WOLVES_BLOCK,
    CONFIG_GP_CH_DYNAMIC_GRANARIES,
    CONFIG_GP_CH_MORE_STOCKPILE,
    CONFIG_GP_CH_NO_BUYER_DISTRIBUTION,
    CONFIG_GP_CH_IMMEDIATELY_DELETE_BUILDINGS,
    CONFIG_GP_CH_GETTING_GRANARIES_GO_OFFROAD,
    CONFIG_MAX_ENTRIES
} config_key;

typedef enum {
    CONFIG_STRING_UI_LANGUAGE_DIR,
    CONFIG_STRING_MAX_ENTRIES
} config_string_key;

/**
 * Get an integer config value
 * @param key Integer key
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
 * Reset all settings to their defaults
 */
void config_set_defaults(void);

/**
 * Load config from file
 */
void config_load(void);

/**
 * Save config to file
 */
void config_save(void);

#endif // CORE_CONFIG_H
