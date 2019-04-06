#include "core/file.h"

#include "core/dir.h"
#include "core/string.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef __vita__
#include "platform/vita/vita.h"
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#if defined(__vita__)

FILE *file_open(const char *filename, const char *mode)
{
    char *resolved_path = vita_prepend_path(filename);
    FILE *fp = fopen(resolved_path, mode);
    free(resolved_path);
    return fp;
}

#elif defined(_WIN32)

wchar_t *utf8_to_wchar(const char *str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    wchar_t *result = (wchar_t*) malloc(sizeof(wchar_t) * size_needed);
    MultiByteToWideChar(CP_UTF8, 0, str, -1, result, size_needed);
    return result;
}

FILE *file_open(const char *filename, const char *mode)
{
    wchar_t *wfile = utf8_to_wchar(filename);
    wchar_t *wmode = utf8_to_wchar(mode);

    FILE *fp = _wfopen(wfile, wmode);

    free(wfile);
    free(wmode);

    return fp;
}

#else

FILE *file_open(const char *filename, const char *mode)
{
    return fopen(filename, mode);
}

#endif

int file_close(FILE *stream)
{
    return fclose(stream);
}

int file_has_extension(const char *filename, const char *extension)
{
    char c;
    do {
        c = *filename;
        filename++;
    } while (c != '.' && c);
    if (!c) {
        filename--;
    }
    return string_compare_case_insensitive(filename, extension) == 0;
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

int file_exists(const char *filename)
{
    return NULL != dir_get_case_corrected_file(filename);
}
