#ifndef EMPIRE_EMPIRE_H
#define EMPIRE_EMPIRE_H

#include "core/buffer.h"

void empire_init_scroll(int x, int y);
void empire_scroll_map(int direction);
void empire_set_viewport(int width, int height);
void empire_adjust_scroll(int *x_offset, int *y_offset);

int empire_selected_object();

void empire_clear_selected_object();
void empire_select_object(int x, int y);


void empire_save_state(buffer *buf);
void empire_load_state(buffer *buf);

#endif // EMPIRE_EMPIRE_H
