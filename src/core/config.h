#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

#define CONFIG_STRING_VALUE_MAX 64

typedef enum {
    CONFIG_GENERAL_ENABLE_AUDIO,
    CONFIG_GENERAL_MASTER_VOLUME,
    CONFIG_GENERAL_ENABLE_VIDEO_SOUND,
    CONFIG_GENERAL_VIDEO_VOLUME,
    CONFIG_GP_FIX_IMMIGRATION_BUG,
    CONFIG_GP_FIX_100_YEAR_GHOSTS,
    CONFIG_SCREEN_DISPLAY_SCALE,
    CONFIG_SCREEN_CURSOR_SCALE,
    CONFIG_SCREEN_COLOR_CURSORS,
    CONFIG_UI_SIDEBAR_INFO,
    CONFIG_UI_SHOW_INTRO_VIDEO,
    CONFIG_UI_SMOOTH_SCROLLING,
    CONFIG_UI_DISABLE_MOUSE_EDGE_SCROLLING,
    CONFIG_UI_VISUAL_FEEDBACK_ON_DELETE,
    CONFIG_UI_ALLOW_CYCLING_TEMPLES,
    CONFIG_UI_SHOW_WATER_STRUCTURE_RANGE,
    CONFIG_UI_SHOW_WATER_STRUCTURE_RANGE_HOUSES,
    CONFIG_UI_SHOW_CONSTRUCTION_SIZE,
    CONFIG_UI_HIGHLIGHT_LEGIONS,
    CONFIG_UI_SHOW_MILITARY_SIDEBAR,
    CONFIG_UI_DISABLE_RIGHT_CLICK_MAP_DRAG,
    CONFIG_UI_SHOW_MAX_PROSPERITY,
    CONFIG_UI_DIGIT_SEPARATOR,
    CONFIG_UI_INVERSE_MAP_DRAG,
    CONFIG_UI_MESSAGE_ALERTS,
    CONFIG_UI_SHOW_GRID,
    CONFIG_UI_SHOW_PARTIAL_GRID_AROUND_CONSTRUCTION,
    CONFIG_UI_ALWAYS_SHOW_ROTATION_BUTTONS,
    CONFIG_UI_SHOW_ROAMING_PATH,
    CONFIG_GP_CH_MAX_GRAND_TEMPLES,
    CONFIG_GP_CH_JEALOUS_GODS,
    CONFIG_GP_CH_GLOBAL_LABOUR,
    CONFIG_GP_CH_RETIRE_AT_60,
    CONFIG_GP_CH_FIXED_WORKERS,
    CONFIG_GP_CH_WOLVES_BLOCK,
    CONFIG_GP_CH_NO_SUPPLIER_DISTRIBUTION,
    CONFIG_GP_CH_GETTING_GRANARIES_GO_OFFROAD,
    CONFIG_GP_CH_GRANARIES_GET_DOUBLE,
    CONFIG_GP_CH_ALLOW_EXPORTING_FROM_GRANARIES,
    CONFIG_GP_CH_TOWER_SENTRIES_GO_OFFROAD,
    CONFIG_GP_CH_FARMS_DELIVER_CLOSE,
    CONFIG_GP_CH_DELIVER_ONLY_TO_ACCEPTING_GRANARIES,
    CONFIG_GP_CH_ALL_HOUSES_MERGE,
    CONFIG_GP_CH_RANDOM_COLLAPSES_TAKE_MONEY,
    CONFIG_GP_CH_MULTIPLE_BARRACKS,
    CONFIG_GP_CH_WAREHOUSES_DONT_ACCEPT,
    CONFIG_GP_CH_HOUSES_DONT_EXPAND_INTO_GARDENS,
    CONFIG_GP_CH_MONUMENTS_BOOST_CULTURE_RATING,
    CONFIG_GP_CH_DISABLE_INFINITE_WOLVES_SPAWNING,
    CONFIG_GP_CH_ROAMERS_DONT_SKIP_CORNERS,
    CONFIG_GP_CH_YEARLY_AUTOSAVE,
    CONFIG_GP_CH_AUTO_KILL_ANIMALS,
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
