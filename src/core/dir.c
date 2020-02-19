#include "core/dir.h"

#include "core/file.h"
#include "core/string.h"
#include "platform/vita/vita.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>

#define fs_dir_type _WDIR
#define fs_dir_entry struct _wdirent
#define fs_dir_open _wopendir
#define fs_dir_close _wclosedir
#define fs_dir_read _wreaddir
#define dir_entry_name(d) filename_to_utf8(d->d_name)

static const char *filename_to_utf8(const wchar_t *str)
{
    static char *filename_buffer = 0;
    static int filename_buffer_size = 0;
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    if (size_needed > filename_buffer_size) {
        free(filename_buffer);
        filename_buffer = (char*) malloc(sizeof(char) * size_needed);
        filename_buffer_size = size_needed;
    }
    WideCharToMultiByte(CP_UTF8, 0, str, -1, filename_buffer, size_needed, NULL, NULL);
    return filename_buffer;
}

#else // not _WIN32
#define fs_dir_type DIR
#define fs_dir_entry struct dirent
#define fs_dir_open opendir
#define fs_dir_close closedir
#define fs_dir_read readdir
#define dir_entry_name(d) ((d)->d_name)
#endif

#ifndef S_ISLNK
#define S_ISLNK(m) 0
#endif

#ifndef S_ISSOCK
#define S_ISSOCK(m) 0
#endif

#ifdef __vita__
#define CURRENT_DIR VITA_PATH_PREFIX
#elif defined(_WIN32)
#define CURRENT_DIR L"."
#else
#define CURRENT_DIR "."
#endif

#define BASE_MAX_FILES 100

static struct {
    dir_listing listing;
    int max_files;
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

const dir_listing *dir_find_files_with_extension(const char *extension)
{
    clear_dir_listing();
    fs_dir_type *d = fs_dir_open(CURRENT_DIR);
    if (!d) {
        return &data.listing;
    }
    fs_dir_entry *entry;
    struct stat file_info;
    while ((entry = fs_dir_read(d))) {
        const char *name = dir_entry_name(entry);
        if (stat(name, &file_info) != -1) {
            int m = file_info.st_mode;
            if (S_ISDIR(m) || S_ISCHR(m) || S_ISBLK(m) || S_ISFIFO(m) || S_ISSOCK(m)) {
                continue;
            }
        }
        if (file_has_extension(name, extension)) {
            if (data.listing.num_files >= data.max_files) {
                expand_dir_listing();
            }
            strncpy(data.listing.files[data.listing.num_files], name, FILE_NAME_MAX);
            data.listing.files[data.listing.num_files][FILE_NAME_MAX - 1] = 0;
            ++data.listing.num_files;
        }
    }
    fs_dir_close(d);
    qsort(data.listing.files, data.listing.num_files, sizeof(char*), compare_lower);

    return &data.listing;
}

static int correct_case(const char *dir, char *filename)
{
    // Note: we do not use the _w* variants for Windows here, because the
    // Windows filesystem is case insensitive and doesn't need corrections
    DIR *d = opendir(dir);
    if (!d) {
        return 0;
    }
    struct dirent *entry;
    while ((entry = readdir(d))) {
        if (string_compare_case_insensitive(entry->d_name, filename) == 0) {
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

    FILE *fp = file_open(filepath, "rb");
    if (fp) {
        file_close(fp);
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
