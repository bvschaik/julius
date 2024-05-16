#ifndef PLATFORM_PLATFORM_H
#define PLATFORM_PLATFORM_H

int platform_sdl_version_at_least(int major, int minor, int patch);
char *platform_get_pref_path(void);
void exit_with_status(int status);

#endif // PLATFORM_PLATFORM_H
