#include "core/file.hpp"

#include <ctype.h>
#include <stdlib.h>

#include "core/dir.hpp"
#include "core/string.hpp"

int file_has_extension(const char *filename, const char *extension)
{
    char c;
    do
    {
        c = *filename;
        filename++;
    }
    while (c != '.' && c);
    if (!c)
    {
        filename--;
    }
    return string_compare_case_insensitive(filename, extension) == 0;
}

void file_change_extension(char *filename, const char *new_extension)
{
    char c;
    do
    {
        c = *filename;
        filename++;
    }
    while (c != '.' && c);
    if (c == '.')
    {
        filename[0] = new_extension[0];
        filename[1] = new_extension[1];
        filename[2] = new_extension[2];
        filename[3] = 0;
    }
}

void file_append_extension(char *filename, const char *extension)
{
    char c;
    do
    {
        c = *filename;
        filename++;
    }
    while (c);
    filename--;
    filename[0] = '.';
    filename[1] = extension[0];
    filename[2] = extension[1];
    filename[3] = extension[2];
    filename[4] = 0;
}


void file_remove_extension(char *filename)
{
    char c;
    do
    {
        c = *filename;
        filename++;
    }
    while (c != '.' && c);
    if (c == '.')
    {
        filename--;
        *filename = 0;
    }
}

int file_exists(const char *filename)
{
    return NULL != dir_get_case_corrected_file(filename);
}
