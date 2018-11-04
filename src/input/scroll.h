#ifndef INPUT_SCROLL_H
#define INPUT_SCROLL_H

#include "input/mouse.h"

int scroll_in_progress(void);

int scroll_get_direction(const mouse *m);

void scroll_arrow_left(void);
void scroll_arrow_right(void);
void scroll_arrow_up(void);
void scroll_arrow_down(void);

#endif // INPUT_SCROLL_H
