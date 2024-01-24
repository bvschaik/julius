#include "file_manager.h"

#include "assets/assets.h"
#include "core/file.h"
#include "core/log.h"
#include "core/string.h"
#include "platform/android/android.h"
#include "platform/emscripten/emscripten.h"
#include "platform/file_manager_cache.h"
#include "platform/platform.h"
#include "platform/vita/vita.h"

#ifndef BUILDING_ASSET_PACKER
#include "SDL.h"
#else
#define SDL_VERSION_ATLEAST(x, y, z) 0
#endif

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _MSC_VER
// Of course MSVC is the only compiler that doesn't have POSIX strcasecmp...
#include <mbstring.h>
#else
#include <strings.h>
#endif

#ifdef _WIN32
#include <windows.h>

#define fs_dir_type _WDIR
#define fs_dir_entry struct _wdirent
#define fs_dir_open _wopendir
#define fs_dir_close _wclosedir
#define fs_dir_read _wreaddir
#define fs_stat _wstat
#define fs_chdir _wchdir
#define fs_fopen _wfopen
#define fs_remove _wremove
#define dir_entry_name(d) wchar_to_utf8(d)
typedef struct _stat stat_info;
typedef wchar_t file_name;

static const char *wchar_to_utf8(const wchar_t *str)
{
    static char *filename_buffer = 0;
    static int filename_buffer_size = 0;
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    if (size_needed > filename_buffer_size) {
        free(filename_buffer);
        filename_buffer = (char *) malloc(sizeof(char) * size_needed);
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
#include <errno.h>
#include <libgen.h>
#define fs_dir_type DIR
#define fs_dir_open opendir
#define fs_dir_close closedir
#ifndef USE_FILE_CACHE
#define fs_dir_entry struct dirent
#define fs_dir_read readdir
#define dir_entry_name(d) (d)
#define fs_stat stat
#endif
#define fs_chdir chdir
#define fs_fopen fopen
#define fs_remove remove
typedef struct stat stat_info;
typedef char file_name;
#endif

#ifndef S_ISLNK
#define S_ISLNK(m) 0
#endif

#ifndef S_ISSOCK
#define S_ISSOCK(m) 0
#endif

#if defined(_WIN32)
#define CURRENT_DIR L"."
#define set_file_name(n) utf8_to_wchar(n)
#define free_file_name(n) free((void *) n)
#else
#define CURRENT_DIR "."
#define set_file_name(n) (n)
#define free_file_name(n)
#endif

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#ifdef __EMSCRIPTEN__
static int writing_to_file;
#endif

#ifndef USE_FILE_CACHE
static int is_file(int mode)
{
    return S_ISREG(mode) || S_ISLNK(mode);
}
#endif

#ifdef __ANDROID__
static const char *assets_directory = ASSETS_DIRECTORY;
#else
#define MAX_ASSET_DIRS 10

#ifndef CUSTOM_ASSETS_DIR
#define CUSTOM_ASSETS_DIR 0
#endif

static const char *ASSET_DIRS[MAX_ASSET_DIRS] = {
#ifdef _WIN32
    "***SDL_BASE_PATH***",
#endif
#ifdef __EMSCRIPTEN__
    "",
#endif
    ".",
#ifdef __vita__
    "app0:",
#elif defined (__SWITCH__)
    "romfs:",
#elif defined (__APPLE__)
    "***SDL_BASE_PATH***",
#elif !defined (_WIN32)
    "***RELATIVE_APPIMG_PATH***",
    "***EXEC_PATH***",
    "***RELATIVE_EXEC_PATH***",
    "~/.local/share/augustus-game",
    "/usr/share/augustus-game",
    "/usr/local/share/augustus-game",
    "/opt/augustus-game",
#endif
    CUSTOM_ASSETS_DIR
};

static char assets_directory[FILE_NAME_MAX];

static int write_base_path_to(char *dest)
{
#if !defined(BUILDING_ASSET_PACKER) && SDL_VERSION_ATLEAST(2, 0, 1)
    if (!platform_sdl_version_at_least(2, 0, 1)) {
        return 0;
    }
    char *base_path = SDL_GetBasePath();
    if (!base_path) {
        return 0;
    }
    strncpy(dest, base_path, FILE_NAME_MAX - 1);
    SDL_free(base_path);
    return 1;
#else
    return 0;
#endif
}
#endif

static struct {
    char current_src_path[FILE_NAME_MAX];
    char current_dst_path[FILE_NAME_MAX];
} directory_copy_data;

static void set_assets_directory(void)
{
#ifndef __ANDROID__
    if (*assets_directory) {
        return;
    }
    // Find assets directory from list
    for (int i = 0; i < MAX_ASSET_DIRS && ASSET_DIRS[i]; ++i) {
        // Special case - home directory
        if (*ASSET_DIRS[i] == '~') {
            const char *home_dir = getenv("HOME");
            if (!home_dir) {
                continue;
            }
            size_t home_dir_length = strlen(home_dir);
            strncpy(assets_directory, home_dir, FILE_NAME_MAX);
            strncpy(assets_directory + home_dir_length, &ASSET_DIRS[i][1], FILE_NAME_MAX - home_dir_length);
            // Special case - SDL base path
        } else if (strcmp(ASSET_DIRS[i], "***SDL_BASE_PATH***") == 0) {
            if (!write_base_path_to(assets_directory)) {
                continue;
            }
            // Special case - Path relative to executable location (AppImage)
        } else if (strcmp(ASSET_DIRS[i], "***RELATIVE_APPIMG_PATH***") == 0) {
#if defined(_WIN32) || defined(__vita__) || defined(__SWITCH__) || defined(__APPLE__)
            log_error("***RELATIVE_APPIMG_PATH*** is not available on your platform.", 0, 0);
            continue;
#else
            if (!write_base_path_to(assets_directory)) {
                continue;
            }
            char *parent = strstr(assets_directory, "/bin");
            if (!parent) {
                continue;
            }
            strncpy(parent, "/share/augustus-game", FILE_NAME_MAX - (parent - assets_directory) - 1);
#endif
        } else if (strcmp(ASSET_DIRS[i], "***EXEC_PATH***") == 0) {
#if defined(_WIN32) || defined(__vita__) || defined(__SWITCH__) || defined(__APPLE__)
            log_error("***EXEC_PATH*** is not available on your platform.", 0, 0);
            continue;
#else
            char arg0_dir[FILE_NAME_MAX] = { 0 };
            if (readlink("/proc/self/exe" /* Linux */, arg0_dir, FILE_NAME_MAX) == -1) {
                if (readlink("/proc/curproc/file" /* FreeBSD */, arg0_dir, FILE_NAME_MAX) == -1) {
                    if (readlink("/proc/self/path/a.out" /* Solaris */, arg0_dir, FILE_NAME_MAX) == -1) {
                        continue;
                    }
                }
            }
            dirname(arg0_dir);
            strncpy(assets_directory, arg0_dir, FILE_NAME_MAX);
#endif
        } else if (strcmp(ASSET_DIRS[i], "***RELATIVE_EXEC_PATH***") == 0) {
#if defined(_WIN32) || defined(__vita__) || defined(__SWITCH__) || defined(__APPLE__)
            log_error("***RELATIVE_EXEC_PATH*** is not available on your platform.", 0, 0);
            continue;
#else
            char arg0_dir[FILE_NAME_MAX];
            if (readlink("/proc/self/exe" /* Linux */, arg0_dir, FILE_NAME_MAX) == -1) {
                if (readlink("/proc/curproc/file" /* FreeBSD */, arg0_dir, FILE_NAME_MAX) == -1) {
                    if (readlink("/proc/self/path/a.out" /* Solaris */, arg0_dir, FILE_NAME_MAX) == -1) {
                        continue;
                    }
                }
            }
            dirname(arg0_dir);
            size_t arg0_dir_length = strlen(arg0_dir);
            strncpy(assets_directory, arg0_dir, FILE_NAME_MAX);
            strncpy(assets_directory + arg0_dir_length, "/../share/augustus-game",
                    FILE_NAME_MAX - arg0_dir_length);
#endif
        } else {
            strncpy(assets_directory, ASSET_DIRS[i], FILE_NAME_MAX - 1);
        }
        size_t offset = strlen(assets_directory);
        assets_directory[offset++] = '/';
        // Special case for romfs on switch
#ifdef __SWITCH__
        if (strcmp(assets_directory, "romfs:/") != 0) {
#endif
            strncpy(&assets_directory[offset], ASSETS_DIR_NAME, FILE_NAME_MAX - offset);
#ifdef __SWITCH__
        }
#endif
        log_info("Trying asset path at", assets_directory, 0);
        const file_name *result = set_file_name(assets_directory);
        fs_dir_type *dir = fs_dir_open(result);
        if (dir) {
            fs_dir_close(dir);
            log_info("Asset path detected at", assets_directory, 0);
            free_file_name(result);
            return;
        }
        free_file_name(result);
    }
    strncpy(assets_directory, ".", FILE_NAME_MAX - 1);
#endif
}

int platform_file_manager_list_directory_contents(
    const char *dir, int type, const char *extension, int (*callback)(const char *, long))
{
    if (type == TYPE_NONE) {
        return LIST_ERROR;
    }

    const file_name *current_dir;
    size_t assets_directory_length = strlen(ASSETS_DIRECTORY);

    if (!dir || !*dir || strcmp(dir, ".") == 0) {
        current_dir = CURRENT_DIR;
    } else if (strncmp(dir, ASSETS_DIRECTORY, assets_directory_length) == 0) {
        set_assets_directory();
        if (strlen(dir) == assets_directory_length) {
            current_dir = set_file_name(assets_directory);
        } else {
            char full_asset_path[FILE_NAME_MAX];
            // Prevent double slashes as they may not work
            if (*assets_directory && assets_directory[strlen(assets_directory) - 1] == '/' && dir[assets_directory_length] == '/') {
                assets_directory_length++;
            }
            snprintf(full_asset_path, FILE_NAME_MAX, "%s%s", assets_directory, dir + assets_directory_length);
            current_dir = set_file_name(full_asset_path);
        }
    } else {
        current_dir = set_file_name(dir);
    }
#ifdef __ANDROID__
    int match = android_get_directory_contents(current_dir, type, extension, callback);
#elif defined(USE_FILE_CACHE)
    const dir_info *d = platform_file_manager_cache_get_dir_info(current_dir);
    if (!d) {
        return LIST_ERROR;
    }
    int match = LIST_NO_MATCH;
    for (file_info *f = d->first_file; f; f = f->next) {
        if (!(type & f->type)) {
            continue;
        }
        if (!platform_file_manager_cache_file_has_extension(f, extension)) {
            continue;
        }
        match = callback(f->name, f->modified_time);
        if (match == LIST_MATCH) {
            break;
        }
    }
#else
    fs_dir_type *d = fs_dir_open(current_dir);
    if (!d) {
        if (dir && *dir && strcmp(dir, ".") != 0) {
            free_file_name(current_dir);
        }
        return LIST_ERROR;
    }
    int match = LIST_NO_MATCH;
    fs_dir_entry *entry;
    stat_info file_info;
    while ((entry = fs_dir_read(d)) != 0) {
        const char *name = dir_entry_name(entry->d_name);
        const file_name *full_path = 0;
        if (dir && *dir && strcmp(dir, ".") != 0) {
            char full_name[FILE_NAME_MAX];
            snprintf(full_name, FILE_NAME_MAX, "%s/%s", dir, name);
            full_path = set_file_name(full_name);
        }
        if (fs_stat(full_path ? full_path : entry->d_name, &file_info) != -1) {
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
            match = callback(name, (long) file_info.st_mtime);
        } else if (file_has_extension(name, extension)) {
            match = callback(name, (long) file_info.st_mtime);
        }
        free_file_name(full_path);
        if (match == LIST_MATCH) {
            break;
        }
    }
    fs_dir_close(d);
#endif
    if (dir && *dir && strcmp(dir, ".") != 0) {
        free_file_name(current_dir);
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

int platform_file_manager_filename_contains(const char *filename, const char *expression)
{
    int has_filename = filename && *filename;
    int has_expression = expression && *expression;
    if (!has_filename) {
        return !has_expression;
    } else if (!has_expression) {
        return 1;
    }
    size_t filename_length = strlen(filename);
    size_t expression_length = strlen(expression);
    if (filename_length < expression_length) {
        return 0;
    }
    size_t limit = filename_length - expression_length;
    for (size_t i = 0; i <= limit; i++) {
        if (platform_file_manager_compare_filename_prefix(filename, expression, (int) expression_length) == 0) {
            return 1;
        }
        filename++;
    }
    return 0;
}

int platform_file_manager_compare_filename(const char *a, const char *b)
{
#ifdef _MSC_VER
    return _mbsicmp((const unsigned char *) a, (const unsigned char *) b);
#else
    return strcasecmp(a, b);
#endif
}

int platform_file_manager_compare_filename_prefix(const char *filename, const char *prefix, int prefix_len)
{
#ifdef _MSC_VER
    return _mbsnicmp((const unsigned char *) filename, (const unsigned char *) prefix, prefix_len);
#else
    return strncasecmp(filename, prefix, prefix_len);
#endif
}

int platform_file_manager_set_base_path(const char *path)
{
    if (!path) {
        log_error("set_base_path: path was not set. Augustus will probably crash.", 0, 0);
        return 0;
    }
#ifdef __ANDROID__
    return android_set_base_path(path);
#else
    const file_name *set_path = set_file_name(path);
    int result = fs_chdir(set_path);
    free_file_name(set_path);
    if (result == 0) {
#ifdef USE_FILE_CACHE
        platform_file_manager_cache_invalidate();
#endif
        return 1;
    }
    return 0;
#endif
}

#if defined(__ANDROID__)

FILE *platform_file_manager_open_file(const char *filename, const char *mode)
{
    int fd = android_get_file_descriptor(filename, mode);
    if (!fd) {
        return NULL;
    }
    return fdopen(fd, mode);
}

FILE *platform_file_manager_open_asset(const char *asset, const char *mode)
{
    return (FILE *) android_open_asset(asset, mode);
}

int platform_file_manager_remove_file(const char *filename)
{
    return android_remove_file(filename);
}

#else

FILE *platform_file_manager_open_file(const char *filename, const char *mode)
{
#ifdef USE_FILE_CACHE
    if (strchr(mode, 'w')) {
        platform_file_manager_cache_update_file_info(filename);
    }
#endif

#if defined(__EMSCRIPTEN__)
    writing_to_file = strchr(mode, 'w') != 0;
#endif

    const file_name *wfile = set_file_name(filename);
    const file_name *wmode = set_file_name(mode);

    FILE *fp = fs_fopen(wfile, wmode);

    free_file_name(wfile);
    free_file_name(wmode);

    return fp;
}

int platform_file_manager_remove_file(const char *filename)
{
#ifdef USE_FILE_CACHE
    platform_file_manager_cache_delete_file_info(filename);
#endif
    const file_name *wfile = set_file_name(filename);
    int result = fs_remove(wfile);
    free_file_name(wfile);
#if defined(__EMSCRIPTEN__)
    if (result == 0) {
        EM_ASM(
            Module.syncFS();
        );
    }
#endif
    return result == 0;
}

FILE *platform_file_manager_open_asset(const char *asset, const char *mode)
{
    set_assets_directory();
    const char *cased_asset_path = dir_get_asset(assets_directory, asset);
    return platform_file_manager_open_file(cased_asset_path, mode);
}
#endif

int platform_file_manager_close_file(FILE *stream)
{
    int result = fclose(stream);
#ifdef __EMSCRIPTEN__
    if (writing_to_file) {
        writing_to_file = 0;
        EM_ASM(
            Module.syncFS();
        );
    }
#endif
    return result == 0;
}

int platform_file_manager_create_directory(const char *name, int overwrite)
{
    char tokenized_name[FILE_NAME_MAX] = { 0 };
    char temporary_path[FILE_NAME_MAX] = { 0 };
    strncpy(tokenized_name, name, FILE_NAME_MAX - 1);
    char *token = strtok(tokenized_name, "/\\");
    int overwrite_last = 0;
    while (token) {
        overwrite_last = 0;
        strncat(temporary_path, token, FILE_NAME_MAX - 1);
#ifdef _WIN32
        wchar_t *wpath = utf8_to_wchar(temporary_path);
        if (CreateDirectoryW(wpath, 0) == 0) {
            if (GetLastError() != ERROR_ALREADY_EXISTS) {
                free(wpath);
                return 0;
            } else if (!overwrite) {
                overwrite_last = 1;
            }
        }
        free(wpath);
#else
        if (mkdir(temporary_path, 0744) != 0) {
            if (errno != EEXIST) {
                return 0;
            } else if (!overwrite) {
                overwrite_last = 1;
            }
        }
#endif
        strncat(temporary_path, "/", FILE_NAME_MAX - 1);
        token = strtok(0, "/\\");
    }
    return !overwrite_last;
}

int platform_file_manager_copy_file(const char *src, const char *dst)
{
    FILE *in = platform_file_manager_open_file(src, "rb");
    if (!in) {
        return 0;
    }
    FILE *out = platform_file_manager_open_file(dst, "wb");
    if (!out) {
        fclose(in);
        return 0;
    }

    char buf[1024];
    size_t read = 0;

    while ((read = fread(buf, 1, 1024, in)) == 1024) {
        fwrite(buf, 1, 1024, out);
    }
    fwrite(buf, 1, read, out);

    file_close(out);
    file_close(in);

    return 1;
}

static void append_name_to_path(const char *name)
{
    strncat(directory_copy_data.current_src_path, "/", FILE_NAME_MAX - 1);
    strncat(directory_copy_data.current_dst_path, "/", FILE_NAME_MAX - 1);

    strncat(directory_copy_data.current_src_path, name, FILE_NAME_MAX - 1);
    strncat(directory_copy_data.current_dst_path, name, FILE_NAME_MAX - 1);
}

static void move_up_path(void)
{
    char *last_src_path_delimiter = strrchr(directory_copy_data.current_src_path, '/');
    char *last_dst_path_delimiter = strrchr(directory_copy_data.current_dst_path, '/');
    if (last_src_path_delimiter) {
        *last_src_path_delimiter = 0;
    }
    if (last_dst_path_delimiter) {
        *last_dst_path_delimiter = 0;
    }
}


static int copy_file(const char *name, long unused)
{
    append_name_to_path(name);
    int result = platform_file_manager_copy_file(directory_copy_data.current_src_path,
        directory_copy_data.current_dst_path) != 0;

    move_up_path();

    return result;
}

static int copy_directory(const char *name, long unused)
{
#ifdef __ANDROID__
    return LIST_ERROR;
#else
    if (name) {
        append_name_to_path(name);
    }
    if (!platform_file_manager_create_directory(directory_copy_data.current_dst_path, 0)) {
        if (name) {
            move_up_path();
            return LIST_CONTINUE;
        }
    }

    // Create subdirs
    int result = platform_file_manager_list_directory_contents(directory_copy_data.current_src_path,
            TYPE_DIR, 0, copy_directory) != LIST_ERROR &&
        platform_file_manager_list_directory_contents(directory_copy_data.current_src_path,
            TYPE_FILE, 0, copy_file) != LIST_ERROR;

    move_up_path();

    return result;
#endif
}

static void copy_directory_name(const char *name, char *dst)
{
    if (strncmp(name, ASSETS_DIRECTORY, strlen(ASSETS_DIRECTORY)) == 0) {
        set_assets_directory();
        if (strlen(name) == strlen(ASSETS_DIRECTORY)) {
            strncpy(dst, assets_directory, FILE_NAME_MAX);
        } else {
            snprintf(dst, FILE_NAME_MAX, "%s%s", assets_directory, name + strlen(assets_directory));
        }
    } else {
        strncpy(dst, name, FILE_NAME_MAX);
    }

    char *cursor = dst;

    for (size_t i = 0; i < FILE_NAME_MAX; i++) {
        if (!*cursor) {
            break;
        } else if (*cursor == '\\') {
            *cursor = '/';
        }
        cursor++;
    }
}

int platform_file_manager_copy_directory(const char *src, const char *dst)
{
    copy_directory_name(src, directory_copy_data.current_src_path);
    copy_directory_name(dst, directory_copy_data.current_dst_path);
    return copy_directory(0, 0);
}

static int remove_file(const char *name, long unused)
{
    append_name_to_path(name);
    int result = platform_file_manager_remove_file(directory_copy_data.current_src_path);
    move_up_path();
    return result;
}

static int remove_directory(const char *name, long unused)
{
#ifdef __ANDROID__
    return LIST_ERROR;
#else
    if (name) {
        append_name_to_path(name);
    }

    // Create subdirs
    int result = platform_file_manager_list_directory_contents(directory_copy_data.current_src_path,
            TYPE_FILE, 0, remove_file) != LIST_ERROR &&
        platform_file_manager_list_directory_contents(directory_copy_data.current_src_path,
            TYPE_DIR, 0, remove_directory) != LIST_ERROR;

    if (result) {
#ifdef _WIN32
        wchar_t *wdir = utf8_to_wchar(directory_copy_data.current_src_path);
        result = RemoveDirectoryW(wdir) != 0;
        free(wdir);
#else
        result = remove(directory_copy_data.current_src_path);
#endif
    }

    move_up_path();

    return result;
#endif
}

int platform_file_manager_remove_directory(const char *path)
{
    copy_directory_name(path, directory_copy_data.current_src_path);
    return remove_directory(0, 0);
}
