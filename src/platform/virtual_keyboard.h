#ifndef PLATFORM_VIRTUAL_KEYBOARD_H
#define PLATFORM_VIRTUAL_KEYBOARD_H

void platform_virtual_keyboard_start(void);
void platform_virtual_keyboard_resume(void);
void platform_virtual_keyboard_stop(void);
void platform_virtual_keyboard_show(void);
void platform_virtual_keyboard_hide(void);
int platform_virtual_keyboard_showing(void);

#endif // PLATFORM_VIRTUAL_KEYBOARD_H
