#ifndef PLATFORM_MOUSE_H
#define PLATFORM_MOUSE_H

void platform_mouse_get_relative_state(int *x, int *y);
void platform_mouse_set_relative_mode(int enabled);

#endif //PLATFORM_MOUSE_H
