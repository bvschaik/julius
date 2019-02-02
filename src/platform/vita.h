#ifndef PLATFORM_VITA_H
#define PLATFORM_VITA_H

#define VITA_PATH_PREFIX "ux0:/data/julius/"

int chdir(char *path);

char* vita_prepend_path(char *path);

#endif // PLATFORM_VITA_H