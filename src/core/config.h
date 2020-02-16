#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

typedef enum {
    CONFIG_GP_FIX_IMMIGRATION_BUG,
    CONFIG_GP_FIX_100_YEAR_GHOSTS,
    CONFIG_GP_FIX_EDITOR_EVENTS,
    CONFIG_UI_SIDEBAR_INFO,
    CONFIG_UI_SHOW_INTRO_VIDEO,
    CONFIG_UI_SMOOTH_SCROLLING,
    CONFIG_UI_VISUAL_FEEDBACK_ON_DELETE,
    CONFIG_UI_ALLOW_CYCLING_TEMPLES,
    CONFIG_UI_SHOW_WATER_STRUCTURE_RANGE,
    CONFIG_MAX_ENTRIES
} config_key;

typedef enum {
    CONFIG_STRING_UI_LANGUAGE_DIR,
    CONFIG_STRING_MAX_ENTRIES
} config_string_key;

int config_get(config_key key);
void config_set(config_key key, int value);

const char *config_get_string(config_string_key key);
void config_set_string(config_string_key key, const char *value);

void config_set_defaults(void);

void config_load(void);
void config_save(void);

#endif // CORE_CONFIG_H
