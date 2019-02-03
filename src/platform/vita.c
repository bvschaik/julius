#include "vita.h"
#include "core/file.h"
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include "SDL.h"

int _newlib_heap_size_user = 192 * 1024 * 1024;

int chdir(char *path) {
    return 0;
}

char* vita_prepend_path(const char *path) {
    char *new_path = malloc(2 * FILE_NAME_MAX * sizeof(char));
    snprintf(new_path, 2 * FILE_NAME_MAX * sizeof(char), "%s%s", VITA_PATH_PREFIX, path);

    SDL_Log("vita_prepend_path: %s", new_path);

    return new_path;
}