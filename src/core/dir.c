#include "core/dir.h"

#include "core/file.h"
#include "core/string.h"
#include "platform/file_manager.h"

#include <stdlib.h>
#include <string.h>

#define BASE_MAX_FILES 100

static struct {
    dir_listing listing;
    int max_files;
    char *cased_filename;
} data;

static void allocate_listing_files(int min, int max)
{
    for (int i = min; i < max; i++) {
        data.listing.files[i] = malloc(FILE_NAME_MAX * sizeof(char));
        data.listing.files[i][0] = 0;
    }
}

static void clear_dir_listing(void)
{
    data.listing.num_files = 0;
    if (data.max_files <= 0) {
        data.listing.files = (char **) malloc(BASE_MAX_FILES * sizeof(char *));
        allocate_listing_files(0, BASE_MAX_FILES);
        data.max_files = BASE_MAX_FILES;
    } else {
        for (int i = 0; i < data.max_files; i++) {
            data.listing.files[i][0] = 0;
        }
    }
}

static void expand_dir_listing(void)
{
    int old_max_files = data.max_files;

    data.max_files = 2 * old_max_files;
    data.listing.files = (char **) realloc(data.listing.files, data.max_files * sizeof(char *));
    allocate_listing_files(old_max_files, data.max_files);
}

static int compare_lower(const void *va, const void *vb)
{
    // arguments are pointers to char*
    return string_compare_case_insensitive(*(const char**)va, *(const char**)vb);
}

static int add_file_with_extension_to_listing(const char *filename)
{
    if (data.listing.num_files >= data.max_files) {
        expand_dir_listing();
    }
    strncpy(data.listing.files[data.listing.num_files], filename, FILE_NAME_MAX);
    data.listing.files[data.listing.num_files][FILE_NAME_MAX - 1] = 0;
    ++data.listing.num_files;
    return LIST_CONTINUE;
}

const dir_listing *dir_find_files_with_extension(const char *extension)
{
    clear_dir_listing();
    if (platform_file_manager_list_directory_contents(0, TYPE_FILE, extension, add_file_with_extension_to_listing) == LIST_ERROR) {
        return &data.listing;
    }
    qsort(data.listing.files, data.listing.num_files, sizeof(char*), compare_lower);

    return &data.listing;
}

static int compare_case(const char *filename)
{
    if (string_compare_case_insensitive(filename, data.cased_filename) == 0) {
        strcpy(data.cased_filename, filename);
        return LIST_MATCH;
    }
    return LIST_NO_MATCH;
}

static int correct_case(const char *dir, char *filename)
{
    data.cased_filename = filename;
    return platform_file_manager_list_directory_contents(dir, TYPE_FILE, 0, compare_case) == LIST_MATCH;
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

    FILE *fp = file_open(filepath, "rb");
    if (fp) {
        file_close(fp);
        return filepath;
    }

    if (!platform_file_manager_should_case_correct_file()) {
        return 0;
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
