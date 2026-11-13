#ifndef PLATFORM_WIIU_H
#define PLATFORM_WIIU_H

#ifdef __WIIU__

#define WIIU_PATH_PREFIX "fs:/vol/external01/wiiu/apps/julius/"

#define PLATFORM_ENABLE_INIT_CALLBACK
void platform_init_callback(void);

#define PLATFORM_ENABLE_PER_FRAME_CALLBACK
void platform_per_frame_callback(void);

#define PLATFORM_USE_VIRTUAL_KEYBOARD
void platform_show_virtual_keyboard(void);
void platform_hide_virtual_keyboard(void);

#define PLATFORM_USE_SOFTWARE_CURSOR

#endif // __WIIU__
#endif // PLATFORM_WIIU_H
