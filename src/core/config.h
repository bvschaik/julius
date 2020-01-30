#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

typedef enum {
    CONFIG_GP_FIX_IMMIGRATION_BUG,
    CONFIG_UI_SIDEBAR_INFO,
    CONFIG_MAX_ENTRIES
} config_key;

int config_get(config_key key);
void config_set(config_key key, int value);
void config_set_defaults(void);

void config_load(void);
void config_save(void);

#endif // CORE_CONFIG_H
