#include "core/file.h"

#include "core/string.h"
#include "platform/file_manager.h"

#include <stdio.h>
#include <stdlib.h>

FILE *file_open(const char *filename, const char *mode)
{
    return platform_file_manager_open_file(filename, mode);
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
    char c;
    do {
        c = *filename;
        filename++;
    } while (c != '.' && c);
    if (!c) {
        filename--;
    }
    return platform_file_manager_compare_filename(filename, extension) == 0;
}

void file_change_extension(char *filename, const char *new_extension)
{
    char c;
    do {
        c = *filename;
        filename++;
    } while (c != '.' && c);
    if (c == '.') {
        filename[0] = new_extension[0];
        filename[1] = new_extension[1];
        filename[2] = new_extension[2];
        filename[3] = 0;
    }
}

void file_append_extension(char *filename, const char *extension)
{
    char c;
    do {
        c = *filename;
        filename++;
    } while (c);
    filename--;
    filename[0] = '.';
    filename[1] = extension[0];
    filename[2] = extension[1];
    filename[3] = extension[2];
    filename[4] = 0;
}

void file_remove_extension(uint8_t *filename)
{
    uint8_t c;
    do {
        c = *filename;
        filename++;
    } while (c != '.' && c);
    if (c == '.') {
        filename--;
        *filename = 0;
    }
}

int file_exists(const char *filename, int localizable)
{
    return NULL != dir_get_file(filename, localizable);
}

int file_remove(const char *filename)
{
    return platform_file_manager_remove_file(filename);
}
