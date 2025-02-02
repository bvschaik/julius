#include "core/file.h"

#include "core/string.h"
#include "platform/file_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *file_open(const char *filename, const char *mode)
{
    if (!filename) {
        return 0;
    }
    if (strncmp(filename, ASSETS_DIRECTORY, sizeof(ASSETS_DIRECTORY) - 1) == 0) {
        return platform_file_manager_open_asset(filename + sizeof(ASSETS_DIRECTORY), mode);
    }
    return platform_file_manager_open_file(filename, mode);
}

FILE *file_open_asset(const char *asset, const char *mode)
{
    return platform_file_manager_open_asset(asset, mode);
}

int file_close(FILE *stream)
{
    return platform_file_manager_close_file(stream);
}

int file_has_extension(const char *filename, const char *extension)
{
    if (!extension || !*extension) {
        return 1;
    }
    filename = strrchr(filename, '.');
    return filename ? platform_file_manager_compare_filename(filename + 1, extension) == 0 : 0;
}

void file_change_extension(char *filename, const char *new_extension)
{
    if (!new_extension || !*new_extension) {
        return;
    }
    filename = strrchr(filename, '.');
    if (!filename) {
        return;
    }
    filename++;
    snprintf(filename, strlen(filename) + 1, "%s", new_extension);
}

void file_append_extension(char *filename, const char *extension, size_t length)
{
    if (!extension || !*extension) {
        return;
    }
    size_t actual_length = strlen(filename);
    if (actual_length + strlen(extension) + 1 > length) {
        return;
    }
    snprintf(filename + actual_length, length - actual_length, ".%s", extension);
}

void file_remove_extension(char *filename)
{
    filename = strrchr(filename, '.');
    if (filename) {
        *filename = 0;
    }
}

const char *file_remove_path(const char *filename)
{
    char *filename_without_directory = strrchr(filename, '/');
    if (filename_without_directory) {
        return filename_without_directory + 1;
    }
    filename_without_directory = strrchr(filename, '\\');
    if (filename_without_directory) {
        return filename_without_directory + 1;
    }
    return filename;
}

int file_exists(const char *filename, int localizable)
{
    return NULL != dir_get_file(filename, localizable);
}

int file_remove(const char *filename)
{
    return platform_file_manager_remove_file(filename);
}
