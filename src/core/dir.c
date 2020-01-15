#include "core/dir.h"

#include "core/file.h"
#include "core/log.h"
#include "core/string.h"
#include "platform/android/android.h"

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
#define dir_entry_close_name(n) free((void*)n)

static const char *filename_to_utf8(const wchar_t *str)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    char *result = (char*) malloc(sizeof(char) * size_needed);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, result, size_needed, NULL, NULL);
    return result;
}

#else // not _WIN32
#define fs_dir_type DIR
#define fs_dir_entry struct dirent
#define fs_dir_open opendir
#define fs_dir_close closedir
#define fs_dir_read readdir
#define dir_entry_name(d) ((d)->d_name)
#define dir_entry_close_name(n)
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

#ifdef _WIN32
#include <windows.h>

#define fs_dir_type _WDIR
#define fs_dir_entry struct _wdirent
#define fs_dir_open _wopendir
#define fs_dir_close _wclosedir
#define fs_dir_read _wreaddir
#define dir_entry_name(d) filename_to_utf8(d->d_name)
#define dir_entry_close_name(n) free((void*)n)

static const char *filename_to_utf8(const wchar_t *str)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    char* result = (char*)malloc(sizeof(char) * size_needed);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, result, size_needed, NULL, NULL);
    return result;
}

#else // not _WIN32
#define fs_dir_type DIR
#define fs_dir_entry struct dirent
#define fs_dir_open opendir
#define fs_dir_close closedir
#define fs_dir_read readdir
#define dir_entry_name(d) ((d)->d_name)
#define dir_entry_close_name(n)
#endif

#ifdef __vita__
#define CURRENT_DIR VITA_PATH_PREFIX
#elif defined(_WIN32)
#define CURRENT_DIR L"."
#else
#define CURRENT_DIR "."
#endif

#ifdef _MSC_VER
#include <direct.h>
#define chdir _chdir
#elif !defined(__vita__) || !defined(__ANDROID__)
#include <unistd.h>
#endif

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

#ifdef __ANDROID__
    if (!android_get_directory_contents_by_extension(listing.files, &listing.num_files, extension, DIR_MAX_FILES)) {
        return &listing;
    }
#else
    fs_dir_type *d = fs_dir_open(CURRENT_DIR);
    if (!d) {
        return &listing;
    }
    fs_dir_entry *entry;
    struct stat file_info;
    while ((entry = fs_dir_read(d)) && listing.num_files < DIR_MAX_FILES) {
        const char *name = dir_entry_name(entry);
        if (stat(name, &file_info) != -1) {
            int m = file_info.st_mode;
            if (S_ISDIR(m) || S_ISCHR(m) || S_ISBLK(m) || S_ISFIFO(m) || S_ISSOCK(m)) {
                continue;
            }
        }
        if (file_has_extension(name, extension)) {
            strncpy(listing.files[listing.num_files], name, FILE_NAME_MAX);
            listing.files[listing.num_files][FILE_NAME_MAX - 1] = 0;
            ++listing.num_files;
        }
        dir_entry_close_name(name);
    }
    fs_dir_close(d);
#endif
    qsort(listing.files, listing.num_files, sizeof(char*), compare_lower);

    return &listing;
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

int dir_set_base_path(const char *path)
{
    if (!path) {
        log_error("set_base_path: path was not set. Julius will probably crash.", 0, 0);
        return 0;
    }
#ifdef __ANDROID__
    return android_set_base_path(path);
#else
    return chdir(path) == 0;
#endif
}

const char *dir_get_case_corrected_file(const char *filepath)
{
#ifdef __ANDROID__
    // Android checks for proper case in the Java side
    return filepath;
#else
    static char corrected_filename[2 * FILE_NAME_MAX];

    FILE* fp = file_open(filepath, "rb");
    if (fp) {
        file_close(fp);
        return filepath;
    }

    strncpy(corrected_filename, filepath, 2 * FILE_NAME_MAX);
    corrected_filename[2 * FILE_NAME_MAX - 1] = 0;

    char* slash = strchr(corrected_filename, '/');
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
#endif
}
