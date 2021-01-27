#ifndef WINDOW_SELECT_LIST_H
#define WINDOW_SELECT_LIST_H

#include <stdint.h>

void window_select_list_show(int x, int y, int group, int num_items, void (*callback)(int));
void window_select_list_show_text(int x, int y, const uint8_t **items, int num_items, void (*callback)(int));

#endif // WINDOW_SELECT_LIST_H
