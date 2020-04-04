#ifndef PLATFORM_JOYSTICK_H
#define PLATFORM_JOYSTICK_H

void platform_joystick_init(void);
void platform_joystick_device_changed(int id, int is_connected);
void platform_joystick_handle_axis(void);
void platform_joystick_handle_trackball(void);
void platform_joystick_handle_hat(void);
void platform_joystick_handle_button(int is_down);

#endif // PLATFORM_JOYSTICK_H
