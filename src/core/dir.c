#include "core/dir.h"

#include "core/file.h"
#include "core/string.h"
#include "platform/file_manager.h"

#include <stdlib.h>

static dir_listing listing;
static int listing_initialized = 0;

static void clear_dir_listing(void)
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
    return string_compare_case_insensitive(*(const char**)va, *(const char**)vb);
}

const dir_listing *dir_find_files_with_extension(const char *extension)
{
    clear_dir_listing();
    if (!platform_file_manager_get_directory_contents_by_extension(listing.files, &listing.num_files, extension, DIR_MAX_FILES)) {
        return &listing;
    }
    qsort(listing.files, listing.num_files, sizeof(char*), compare_lower);
    return &listing;
}

const char *dir_get_case_corrected_file(const char *filepath)
{
    FILE *fp = file_open(filepath, "rb");
    if (fp) {
        file_close(fp);
        return filepath;
    }
    return platform_file_manager_case_correct_file(filepath);
}
