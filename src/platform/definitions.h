#ifndef PLATFORM_DEFINITIONS_H
#define PLATFORM_DEFINITIONS_H

#if defined(__ANDROID__) || defined(__SWITCH__) || defined(__vita__)
#define FORCE_FULLSCREEN 1
#define SHOW_VIRTUAL_KEYBOARD
#else
#define FORCE_FULLSCREEN 0
#endif

#endif // PLATFORM_DEFINITIONS_H
