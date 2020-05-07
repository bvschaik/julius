#ifndef EMPIRE_EMPIRE_H
#define EMPIRE_EMPIRE_H

#include "core/buffer.h"

void empire_load(int is_custom_scenario, int empire_id);
void empire_load_editor(int empire_id, int viewport_width, int viewport_height);

void empire_init_scenario(void);
void empire_scroll_map(int x, int y);
void empire_set_viewport(int width, int height);
void empire_get_scroll(int *x_scroll, int *y_scroll);
void empire_set_scroll(int x, int y);
void empire_adjust_scroll(int *x_offset, int *y_offset);

int empire_selected_object(void);

void empire_clear_selected_object(void);
void empire_select_object(int x, int y);

int empire_can_export_resource_to_city(int city_id, int resource);
int empire_can_import_resource_from_city(int city_id, int resource);

void empire_save_state(buffer *buf);
void empire_load_state(buffer *buf);

#endif // EMPIRE_EMPIRE_H
