#ifndef INPUT_SCROLL_H
#define INPUT_SCROLL_H

#include "graphics/mouse.h"

int scroll_in_progress();

int scroll_get_direction(const mouse *m);

void scroll_arrow_left();
void scroll_arrow_right();
void scroll_arrow_up();
void scroll_arrow_down();

#endif // INPUT_SCROLL_H
