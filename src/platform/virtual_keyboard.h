#ifndef PLATFORM_VIRTUAL_KEYBOARD_H
#define PLATFORM_VIRTUAL_KEYBOARD_H

#include "input/mouse.h"
#include "window/input_box.h"

void platform_start_virtual_keyboard(const input_box *capture_box);
void platform_resume_virtual_keyboard(void);
void virtual_keyboard_handle_mouse(const mouse *m);
void platform_stop_virtual_keyboard(void);

#endif // PLATFORM_VIRTUAL_KEYBOARD_H
