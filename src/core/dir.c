#include "core/dir.h"

#include "core/file.h"
#include "core/string.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

static dir_listing listing;
static int listing_initialized = 0;

static void clear_dir_listing()
{
    if (!listing_initialized) {
        for (int i = 0; i < DIR_MAX_FILES; i++) {
            listing.files[i] = malloc(FILE_NAME_MAX * sizeof(char));
        }
        listing_initialized = 1;
    }
    listing.num_files = 0;
    for (int i = 0; i < DIR_MAX_FILES; i++) {
        listing.files[i][0] = 0;
    }
}

static int compare_lower(const void *va, const void *vb)
{
    // arguments are pointers to char*
    return string_compare_case_insensitive(*(char**)va, *(char**)vb);
}

const dir_listing *dir_find_files_with_extension(const char *extension)
{
    clear_dir_listing();
    DIR *d = opendir(".");
    if (!d) {
        return &listing;
    }
    struct dirent *entry;
    while ((entry = readdir(d)) && listing.num_files < DIR_MAX_FILES) {
        if (file_has_extension(entry->d_name, extension)) {
            strncpy(listing.files[listing.num_files], entry->d_name, FILE_NAME_MAX - 1);
            listing.files[listing.num_files][FILE_NAME_MAX - 1] = 0;
            ++listing.num_files;
        }
    }
    closedir(d);
    qsort(listing.files, listing.num_files, sizeof(char*), compare_lower);

    return &listing;
}

static int correct_case(const char *dir, char *filename)
{
    DIR *d = opendir(dir);
    if (!d) {
        return 0;
    }
    struct dirent *entry;
    while ((entry = readdir(d))) {
        if (strcasecmp(entry->d_name, filename) == 0) {
            strcpy(filename, entry->d_name);
            closedir(d);
            return 1;
        }
    }
    closedir(d);
    return 0;
}

static void move_left(char *str)
{
    while (*str) {
        str[0] = str[1];
        str++;
    }
    *str = 0;
}

const char *dir_get_case_corrected_file(const char *filepath)
{
    static char corrected_filename[2 * FILE_NAME_MAX];

    FILE *fp = fopen(filepath, "rb");
    if (fp) {
        fclose(fp);
        return filepath;
    }

    strncpy(corrected_filename, filepath, 2 * FILE_NAME_MAX);
    corrected_filename[2 * FILE_NAME_MAX - 1] = 0;

    char *slash = strchr(corrected_filename, '/');
    if (!slash) {
        slash = strchr(corrected_filename, '\\');
    }
    if (slash) {
        *slash = 0;
        if (correct_case(".", corrected_filename)) {
            char *path = slash + 1;
            if (*path == '\\') {
                // double backslash: move everything to the left
                move_left(path);
            }
            if (correct_case(corrected_filename, path)) {
                *slash = '/';
                return corrected_filename;
            }
        }
    } else {
        if (correct_case(".", corrected_filename)) {
            return corrected_filename;
        }
    }
    return 0;
}
