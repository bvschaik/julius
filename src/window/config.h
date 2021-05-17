#ifndef WINDOW_CONFIG_H
#define WINDOW_CONFIG_H

typedef enum {
    CONFIG_FIRST_PAGE = 0,
    CONFIG_PAGE_GENERAL = 0,
    CONFIG_PAGE_UI_CHANGES = 1,
    CONFIG_PAGE_GAMEPLAY_CHANGES = 2,
    CONFIG_PAGE_CITY_MANAGEMENT_CHANGES = 3,
    CONFIG_PAGES = 4
} window_config_page;

void window_config_show(window_config_page page, int show_background_image);

#endif // WINDOW_CONFIG_H
