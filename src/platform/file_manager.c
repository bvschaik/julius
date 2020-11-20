#include "file_manager.h"

#include "core/file.h"
#include "core/log.h"
#include "core/string.h"
#include "platform/android/android.h"
#include "platform/vita/vita.h"

#include <dirent.h>
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
#define dir_entry_name(d) wchar_to_utf8(d->d_name)
#define dir_to_utf8(d) wchar_to_utf8(d)
typedef const wchar_t * dir_name;

static const char *wchar_to_utf8(const wchar_t *str)
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

static wchar_t *utf8_to_wchar(const char *str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    wchar_t *result = (wchar_t *) malloc(sizeof(wchar_t) * size_needed);
    MultiByteToWideChar(CP_UTF8, 0, str, -1, result, size_needed);
    return result;
}

#else // not _WIN32
#define fs_dir_type DIR
#define fs_dir_entry struct dirent
#define fs_dir_open opendir
#define fs_dir_close closedir
#define fs_dir_read readdir
#define dir_entry_name(d) ((d)->d_name)
#define dir_to_utf8(d) (d)
typedef const char * dir_name;
#endif

#ifndef S_ISLNK
#define S_ISLNK(m) 0
#endif

#ifndef S_ISSOCK
#define S_ISSOCK(m) 0
#endif

#ifdef __vita__
#define CURRENT_DIR VITA_PATH_PREFIX
#define set_dir_name(n) vita_prepend_path(n)
#define free_dir_name(n) free((void*)n)
#elif defined(_WIN32)
#define CURRENT_DIR L"."
#define set_dir_name(n) utf8_to_wchar(n)
#define free_dir_name(n) free((void*)n)
#else
#define CURRENT_DIR "."
#define set_dir_name(n) (n)
#define free_dir_name(n)
#endif

#ifdef _MSC_VER
#include <direct.h>
#define chdir _chdir
#elif !defined(__vita__)
#include <unistd.h>
#endif

#ifdef __vita__
typedef struct file_info {
    char name[FILE_NAME_MAX];
    const char *extension;
    int type;
    struct file_info *next;
} file_info;

typedef struct dir_info {
    char name[FILE_NAME_MAX];
    file_info *first_file;
    struct dir_info *next;
} dir_info;

static dir_info *base_dir_info;

static const dir_info *get_or_create_dir_info(dir_name dir)
{
    dir_info *info = base_dir_info;
    const char *dir_utf8 = dir_to_utf8(dir);
    while (info) {
        if (strcmp(info->name, dir_utf8) == 0) {
            return info;
        }
        if (!info->next) {
            break;
        }
        info = info->next;
    }
    fs_dir_type *d = fs_dir_open(dir);
    if (!d) {
        return 0;
    }
    if (!info) {
        info = malloc(sizeof(dir_info));
        base_dir_info = info;
    } else {
        info->next = malloc(sizeof(dir_info));
        info = info->next;
    }
    strncpy(info->name, dir_utf8, FILE_NAME_MAX - 1);
    info->name[FILE_NAME_MAX - 1] = 0;
    info->first_file = 0;
    info->next = 0;
    fs_dir_entry *entry;
    file_info *file_item = 0;
    int dir_name_offset = 0;
    while ((entry = fs_dir_read(d))) {
        const char *name = dir_entry_name(entry);
        if (name[0] == '.') {
            // Skip hidden files
            continue;
        }
        if (!file_item) {
            file_item = malloc(sizeof(file_info));
            info->first_file = file_item;
        } else {
            file_item->next = malloc(sizeof(file_info));
            file_item = file_item->next;
        }
        file_item->next = 0;

        // Copy name
        strncpy(file_item->name, name, FILE_NAME_MAX - 1);
        file_item->name[FILE_NAME_MAX - 1] = 0;

        // Copy extension
        char c;
        const char *extension = file_item->name;
        do {
            c = *extension;
            extension++;
        } while (c != '.' && c);
        file_item->extension = extension;

        // Check type
        int type = TYPE_FILE;
        // Stat does not work for Vita, so we check if a file is a directory by trying to open it as a dir
        // For performance reasons, we only check for a directory if the name has no extension
        // This is effectively a hack, and definitely not full-proof, but the performance gains are well worth it
        if (!c) {
            static char full_name[FILE_NAME_MAX];
            if (!dir_name_offset) {
                strncpy(full_name, info->name, FILE_NAME_MAX);
                dir_name_offset = strlen(info->name);
            }
            strncpy(full_name + dir_name_offset, name, FILE_NAME_MAX - 1 - dir_name_offset);
            fs_dir_type *file_d = fs_dir_open(full_name);
            if (file_d) {
                type = TYPE_DIR;
                fs_dir_close(file_d);
            }
        }
        file_item->type = type;
    }
    fs_dir_close(d);
    return info;
}

static void add_file_info_to_cache(const char *filename, const char *mode)
{
    // Julius only creates files to the base dir
    if (base_dir_info && strchr(mode, 'w') && !file_exists(filename, NOT_LOCALIZED)) {
        file_info *f = malloc(sizeof(file_info));
        strncpy(f->name, filename, FILE_NAME_MAX - 1);
        f->name[FILE_NAME_MAX - 1] = 0;
        f->type = TYPE_FILE;
        char c;
        const char *name = f->name;
        do {
            c = *name;
            name++;
        } while (c != '.' && c);
        f->extension = name;
        f->next = base_dir_info->first_file;
        base_dir_info->first_file = f;
    }
}

static void delete_file_info_from_cache(const char *filename)
{
    // Julius only deletes files from the base dir
    if (!base_dir_info) {
        return;
    }
    file_info *prev = 0;
    for (file_info *f = base_dir_info->first_file; f; f = f->next) {
        if (strcmp(filename, f->name) == 0) {
            if (prev) {
                prev->next = f->next;
            } else {
                base_dir_info->first_file = f->next;
            }
            free(f);
            return;
        }
        prev = f;
    }
}

static int file_info_has_extension(const file_info *f, const char *extension)
{
    if (!(f->type & TYPE_FILE) || !extension || !*extension) {
        return 1;
    }
    return string_compare_case_insensitive(f->extension, extension) == 0;
}
#else
static int is_file(int mode)
{
    return S_ISREG(mode) || S_ISLNK(mode);
}
#endif

int platform_file_manager_list_directory_contents(
    const char *dir, int type, const char *extension, int (*callback)(const char *))
{
    if (type == TYPE_NONE) {
        return LIST_ERROR;
    }

    dir_name current_dir;

    if (!dir || !*dir || strcmp(dir, ".") == 0) {
        current_dir = CURRENT_DIR;
    } else {
        current_dir = set_dir_name(dir);
    }
#ifdef __ANDROID__
    int match = android_get_directory_contents(current_dir, type, extension, callback);
#elif defined(__vita__)
    const dir_info *d = get_or_create_dir_info(current_dir);
    if (!d) {
        return LIST_ERROR;
    }
    int match = LIST_NO_MATCH;
    for(file_info *f = d->first_file; f; f = f->next) {
        if (!(type & f->type)) {
            continue;
        }
        if (!file_info_has_extension(f, extension)) {
            continue;
        }
        match = callback(f->name);
        if (match == LIST_MATCH) {
            break;
        }
    }
#else
    fs_dir_type *d = fs_dir_open(current_dir);
    if (!d) {
        return LIST_ERROR;
    }
    int match = LIST_NO_MATCH;
    fs_dir_entry *entry;
    struct stat file_info;
    while ((entry = fs_dir_read(d))) {
        const char *name = dir_entry_name(entry);
        if (stat(name, &file_info) != -1) {
            int m = file_info.st_mode;
            if ((!(type & TYPE_FILE) && is_file(m)) ||
                (!(type & TYPE_DIR) && S_ISDIR(m)) ||
                S_ISCHR(m) || S_ISBLK(m) || S_ISFIFO(m) || S_ISSOCK(m)) {
                continue;
            }
            if (is_file(m) && !file_has_extension(name, extension)) {
                continue;
            }
            if (type & TYPE_DIR && name[0] == '.') {
                // Skip current (.), parent (..) and hidden directories (.*)
                continue;
            }
            match = callback(name);
        } else if (file_has_extension(name, extension)) {
            match = callback(name);
        }
        if (match == LIST_MATCH) {
            break;
        }
    }
    fs_dir_close(d);
#endif
    if (dir && *dir && strcmp(dir, ".") != 0) {
        free_dir_name(current_dir);
    }
    return match;
}

int platform_file_manager_should_case_correct_file(void)
{
#if defined(_WIN32) || defined(__ANDROID__)
    return 0;
#else
    return 1;
#endif
}

int platform_file_manager_set_base_path(const char *path)
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

#if defined(__vita__)

FILE *platform_file_manager_open_file(const char *filename, const char *mode)
{
    add_file_info_to_cache(filename, mode);
    char *resolved_path = vita_prepend_path(filename);
    FILE *fp = fopen(resolved_path, mode);
    free(resolved_path);
    return fp;
}

int platform_file_manager_remove_file(const char *filename)
{
    delete_file_info_from_cache(filename);
    char *resolved_path = vita_prepend_path(filename);
    int result = remove(resolved_path);
    free(resolved_path);
    return result == 0;
}

#elif defined(_WIN32)

FILE *platform_file_manager_open_file(const char *filename, const char *mode)
{
    wchar_t *wfile = utf8_to_wchar(filename);
    wchar_t *wmode = utf8_to_wchar(mode);

    FILE *fp = _wfopen(wfile, wmode);

    free(wfile);
    free(wmode);

    return fp;
}

int platform_file_manager_remove_file(const char *filename)
{
    wchar_t *wfile = utf8_to_wchar(filename);
    int result = _wremove(wfile);
    free(wfile);
    return result == 0;
}

#elif defined(__ANDROID__)

FILE *platform_file_manager_open_file(const char *filename, const char *mode)
{
    int fd = android_get_file_descriptor(filename, mode);
    if (!fd) {
        return NULL;
    }
    return fdopen(fd, mode);
}

int platform_file_manager_remove_file(const char *filename)
{
    return android_remove_file(filename);
}

#else

FILE *platform_file_manager_open_file(const char *filename, const char *mode)
{
    return fopen(filename, mode);
}

int platform_file_manager_remove_file(const char *filename)
{
    return remove(filename) == 0;
}

#endif
