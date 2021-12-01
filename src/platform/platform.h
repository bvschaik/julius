#ifndef PLATFORM_PLATFORM_H
#define PLATFORM_PLATFORM_H

#if defined(_WIN32) || defined(__vita__) || defined(__SWITCH__) || defined(__ANDROID__)
#define LOG_TO_FILE
#endif

int platform_sdl_version_at_least(int major, int minor, int patch);
void exit_with_status(int status);

#endif // PLATFORM_PLATFORM_H
