#include "core/dir.h"

#include "core/config.h"
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
    char current_dir[FILE_NAME_MAX];
} data;

static void allocate_listing_files(int min, int max)
{
    for (int i = min; i < max; i++) {
        data.listing.files[i].name = malloc(FILE_NAME_MAX * sizeof(char));
        data.listing.files[i].name[0] = 0;
    }
}

static void clear_dir_listing(void)
{
    data.listing.num_files = 0;
    data.current_dir[0] = 0;
    if (data.max_files <= 0) {
        data.listing.files = (dir_entry *) malloc(BASE_MAX_FILES * sizeof(dir_entry));
        allocate_listing_files(0, BASE_MAX_FILES);
        data.max_files = BASE_MAX_FILES;
    } else {
        for (int i = 0; i < data.max_files; i++) {
            data.listing.files[i].name[0] = 0;
            data.listing.files[i].modified_time = 0;
        }
    }
}

static void expand_dir_listing(void)
{
    int old_max_files = data.max_files;

    data.max_files = 2 * old_max_files;
    data.listing.files = (dir_entry *) realloc(data.listing.files, data.max_files * sizeof(dir_entry));
    allocate_listing_files(old_max_files, data.max_files);
}

static int compare_lower(const void *va, const void *vb)
{
    const dir_entry *a = (const dir_entry *) va;
    const dir_entry *b = (const dir_entry *) vb;

    return platform_file_manager_compare_filename(a->name, b->name);
}

static int add_to_listing(const char *filename, long modified_time)
{
    if (data.listing.num_files >= data.max_files) {
        expand_dir_listing();
    }
    snprintf(data.listing.files[data.listing.num_files].name, FILE_NAME_MAX, "%s", filename);
    data.listing.files[data.listing.num_files].modified_time = modified_time;
    ++data.listing.num_files;
    return LIST_CONTINUE;
}

const dir_listing *dir_find_files_with_extension(const char *dir, const char *extension)
{
    clear_dir_listing();
    snprintf(data.current_dir, FILE_NAME_MAX, "%s", dir);
    platform_file_manager_list_directory_contents(dir, TYPE_FILE, extension, add_to_listing);
    qsort(data.listing.files, data.listing.num_files, sizeof(dir_entry), compare_lower);
    return &data.listing;
}

const dir_listing *dir_find_files_with_extension_at_location(int location, const char *extension)
{
    return dir_find_files_with_extension(platform_file_manager_get_directory_for_location(location, 0), extension);
}

const dir_listing *dir_find_all_subdirectories(const char *dir)
{
    clear_dir_listing();
    snprintf(data.current_dir, FILE_NAME_MAX, "%s", dir);
    platform_file_manager_list_directory_contents(dir, TYPE_DIR, 0, add_to_listing);
    qsort(data.listing.files, data.listing.num_files, sizeof(dir_entry), compare_lower);
    return &data.listing;
}

const dir_listing *dir_find_all_subdirectories_at_location(int location)
{
    return dir_find_all_subdirectories(platform_file_manager_get_directory_for_location(location, 0));
}

static int compare_case(const char *filename, long unused)
{
    if (platform_file_manager_compare_filename(filename, data.cased_filename) == 0) {
        // We are copying anyway because the comparison is case insensitive, so we can't use the original filename
        strcpy(data.cased_filename, filename);
        return LIST_MATCH;
    }
    return LIST_NO_MATCH;
}

static int correct_case(const char *dir, char *filename, int type)
{
    data.cased_filename = filename;
    return platform_file_manager_list_directory_contents(dir, type, 0, compare_case) == LIST_MATCH;
}

static void move_left(char *str)
{
    while (*str) {
        str[0] = str[1];
        str++;
    }
    *str = 0;
}

static const char *get_case_corrected_file(const char *dir, const char *filepath)
{
    static char corrected_filename[2 * FILE_NAME_MAX];
    char backup[2 * FILE_NAME_MAX];
    size_t backup_offset = 0;

    // Prevent writing to the same buffer
    if (filepath >= corrected_filename && filepath < corrected_filename + 2 * FILE_NAME_MAX) {
        // File location already corrected, skip
        if (!dir || !*dir) {
            return filepath;
        }
        backup_offset = filepath - corrected_filename;
        snprintf(backup, 2 * FILE_NAME_MAX - backup_offset, "%s", filepath);
        filepath = backup;
    }

    size_t dir_len = 0;
    size_t dir_skip = 0;
    if (!dir || !*dir) {
        dir = ".";
        dir_skip = 2;
    }
    dir_len = strlen(dir);
    snprintf(corrected_filename, 2 * FILE_NAME_MAX, "%s", dir);
    if (dir_len) {
        if (dir[dir_len - 1] != '/') {
            corrected_filename[dir_len] = '/';
            dir_len++;
        }
    }

    snprintf(&corrected_filename[dir_len], 2 * FILE_NAME_MAX - dir_len, "%s", filepath);

    FILE *fp = file_open(corrected_filename, "rb");
    if (fp) {
        file_close(fp);
        return corrected_filename + dir_skip;
    }

    if (!platform_file_manager_should_case_correct_file()) {
        if (filepath == backup) {
            snprintf(corrected_filename + backup_offset, 2 * FILE_NAME_MAX - backup_offset, "%s", backup);
        }
        return 0;
    }

    size_t path_offset = dir_len;
    corrected_filename[path_offset - 1] = 0;

    while (1) {
        char *slash = strchr(&corrected_filename[path_offset], '/');
        if (!slash) {
            slash = strchr(&corrected_filename[path_offset], '\\');
        }
        if (!slash) {
            break;
        }
        *slash = 0;
        if (!correct_case(corrected_filename, &corrected_filename[path_offset], TYPE_DIR)) {
            if (filepath == backup) {
                snprintf(corrected_filename + backup_offset, 2 * FILE_NAME_MAX - backup_offset, "%s", backup);
            }
            return 0;
        }
        char *path = slash + 1;
        if (*path == '\\') {
            // double backslash: move everything to the left
            move_left(path);
        }
        corrected_filename[path_offset - 1] = '/';
        path_offset += strlen(&corrected_filename[path_offset]) + 1;
    }
    if (!correct_case(corrected_filename, &corrected_filename[path_offset], TYPE_FILE)) {
        if (filepath == backup) {
            snprintf(corrected_filename + backup_offset, 2 * FILE_NAME_MAX - backup_offset, "%s", backup);
        }
        return 0;
    }
    corrected_filename[path_offset - 1] = '/';
    return corrected_filename + dir_skip;
}

const dir_listing *dir_append_files_with_extension(const char *extension)
{
    platform_file_manager_list_directory_contents(data.current_dir, TYPE_FILE, extension, add_to_listing);
    qsort(data.listing.files, data.listing.num_files, sizeof(dir_entry), compare_lower);
    return &data.listing;
}

const char *dir_get_file(const char *filepath, int localizable)
{
    if (strncmp(ASSETS_DIRECTORY, filepath, sizeof(ASSETS_DIRECTORY) - 1) == 0) {
        return dir_get_file_at_location(filepath + sizeof(ASSETS_DIRECTORY), PATH_LOCATION_ASSET);
    }
    if (localizable != NOT_LOCALIZED) {
        const char *custom_dir = config_get_string(CONFIG_STRING_UI_LANGUAGE_DIR);
        if (*custom_dir) {
            const char *path = get_case_corrected_file(custom_dir, filepath);
            if (path) {
                return path;
            } else if (localizable == MUST_BE_LOCALIZED) {
                return 0;
            }
        }
    }

    return get_case_corrected_file(0, filepath);
}

const char *dir_get_file_at_location(const char *filename, int location)
{
    return get_case_corrected_file(platform_file_manager_get_directory_for_location(location, 0), filename);
}

const char *dir_append_location(const char *filename, int location)
{
    static char corrected_filename[FILE_NAME_MAX];
    snprintf(corrected_filename, FILE_NAME_MAX, "%s%s",
        platform_file_manager_get_directory_for_location(location, 0), filename);
    return corrected_filename;
}
