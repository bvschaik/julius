#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

typedef enum {
    CONFIG_GP_FIX_IMMIGRATION_BUG,
    CONFIG_GP_FIX_100_YEAR_GHOSTS,
    CONFIG_UI_SIDEBAR_INFO,
    CONFIG_UI_SHOW_INTRO_VIDEO,
    CONFIG_UI_SMOOTH_SCROLLING,
    CONFIG_UI_VISUAL_FEEDBACK_ON_DELETE,
    CONFIG_UI_ALLOW_CYCLING_TEMPLES,
    CONFIG_MISC_SCREENSHOT_PNG_FORMAT,
    CONFIG_MAX_ENTRIES
} config_key;

int config_get(config_key key);
void config_set(config_key key, int value);
void config_set_defaults(void);

void config_load(void);
void config_save(void);

#endif // CORE_CONFIG_H
