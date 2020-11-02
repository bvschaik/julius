#ifndef WINDOW_CITY_H
#define WINDOW_CITY_H

int window_city_military_is_cursor_in_menu(void);

void window_city_draw_all(void);
void window_city_draw_panels(void);
void window_city_draw(void);

void window_city_show(void);

void window_city_military_show(int legion_formation_id);

void window_city_return(void);

#endif // WINDOW_CITY_H
