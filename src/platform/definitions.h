#ifndef PLATFORM_DEFINITIONS_H
#define PLATFORM_DEFINITIONS_H

#if defined(__ANDROID__) || defined(__SWITCH__) || defined(__vita__)
#define FORCE_FULLSCREEN 1
#else
#define FORCE_FULLSCREEN 0
#endif

#ifdef __ANDROID__
#define USE_AUTO_SCALE
#define DEFAULT_DPI 160
#endif

#endif // PLATFORM_DEFINITIONS_H
