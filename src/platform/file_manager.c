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

#if _MSC_VER
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
#define dir_entry_name(d) wchar_to_utf8(d->d_name)
typedef const wchar_t *dir_name;

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
#define fs_dir_entry struct dirent
#define fs_dir_open opendir
#define fs_dir_close closedir
#define fs_dir_read readdir
#define dir_entry_name(d) ((d)->d_name)
typedef const char *dir_name;
#endif

#ifndef S_ISLNK
#define S_ISLNK(m) 0
#endif

#ifndef S_ISSOCK
#define S_ISSOCK(m) 0
#endif

#ifdef __vita__
#define CURRENT_DIR VITA_PATH_PREFIX
#define set_dir_name(n) ( strncmp(n, "app0:", 5) ? vita_prepend_path(n) : n )
#define free_dir_name(n)
#elif defined(_WIN32)
#define CURRENT_DIR L"."
#define set_dir_name(n) utf8_to_wchar(n)
#define free_dir_name(n) free((void *) n)
#else
#define CURRENT_DIR "."
#define set_dir_name(n) (n)
#define free_dir_name(n)
#endif

#ifdef _WIN32
#include <direct.h>
#elif !defined(__vita__)
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
        dir_name result = set_dir_name(assets_directory);
        fs_dir_type *dir = fs_dir_open(result);
        if (dir) {
            fs_dir_close(dir);
            log_info("Asset path detected at", assets_directory, 0);
            free_dir_name(result);
            return;
        }
        free_dir_name(result);
    }
    strncpy(assets_directory, ".", FILE_NAME_MAX - 1);
#endif
}

int platform_file_manager_list_directory_contents(
    const char *dir, int type, const char *extension, int (*callback)(const char *))
{
    if (type == TYPE_NONE) {
        return LIST_ERROR;
    }

    dir_name current_dir;

    if (!dir || !*dir || strcmp(dir, ".") == 0) {
        current_dir = CURRENT_DIR;
    } else if (strcmp(dir, ASSETS_DIRECTORY) == 0) {
        set_assets_directory();
        current_dir = set_dir_name(assets_directory);
    } else {
        current_dir = set_dir_name(dir);
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
        match = callback(f->name);
        if (match == LIST_MATCH) {
            break;
        }
    }
#else
    fs_dir_type *d = fs_dir_open(current_dir);
    if (!d) {
        if (dir && *dir && strcmp(dir, ".") != 0) {
            free_dir_name(current_dir);
        }
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

int platform_file_manager_compare_filename(const char *a, const char *b)
{
#if _MSC_VER
    return _mbsicmp((const unsigned char *) a, (const unsigned char *) b);
#else
    return strcasecmp(a, b);
#endif
}

int platform_file_manager_compare_filename_prefix(const char *filename, const char *prefix, int prefix_len)
{
#if _MSC_VER
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
#elif defined(__vita__)
    return 1;
#elif defined(_WIN32)
    wchar_t *wpath = utf8_to_wchar(path);
    int result = _wchdir(wpath);
    free(wpath);
    return result == 0;
#else
    return chdir(path) == 0;
#endif
}

#ifdef __vita__
FILE *platform_file_manager_open_file(const char *filename, const char *mode)
{
    if (strchr(mode, 'w')) {
        char temp_filename[FILE_NAME_MAX];
        strncpy(temp_filename, filename, FILE_NAME_MAX - 1);
        if (!file_exists(temp_filename, NOT_LOCALIZED)) {
            platform_file_manager_cache_add_file_info(filename);
        }
    }
    filename = set_dir_name(filename);
    return fopen(filename, mode);
}

FILE *platform_file_manager_open_asset(const char *asset, const char *mode)
{
    set_assets_directory();
    const char *cased_asset_path = dir_get_asset(assets_directory, asset);
    return fopen(cased_asset_path, mode);
}

int platform_file_manager_remove_file(const char *filename)
{
    platform_file_manager_cache_delete_file_info(filename);
    return remove(vita_prepend_path(filename)) == 0;
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

FILE *platform_file_manager_open_asset(const char *asset, const char *mode)
{
    set_assets_directory();
    const char *cased_asset_path = dir_get_asset(assets_directory, asset);

    if (!cased_asset_path) {
        return 0;
    }

    wchar_t *wfile = utf8_to_wchar(cased_asset_path);
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

FILE *platform_file_manager_open_asset(const char *asset, const char *mode)
{
    return (FILE *) android_open_asset(asset, mode);
}

int platform_file_manager_remove_file(const char *filename)
{
    return android_remove_file(filename);
}

#elif defined(__EMSCRIPTEN__)

FILE *platform_file_manager_open_file(const char *filename, const char *mode)
{
    writing_to_file = strchr(mode, 'w') != 0;
    return fopen(filename, mode);
}

int platform_file_manager_remove_file(const char *filename)
{
    if (remove(filename) == 0) {
        EM_ASM(
            Module.syncFS();
        );
        return 1;
    }
    return 0;
}

FILE *platform_file_manager_open_asset(const char *asset, const char *mode)
{
    set_assets_directory();
    const char *cased_asset_path = dir_get_asset(assets_directory, asset);
    return fopen(cased_asset_path, mode);
}

#else

FILE *platform_file_manager_open_file(const char *filename, const char *mode)
{
#ifdef USE_FILE_CACHE
    if (strchr(mode, 'w')) {
        char temp_filename[FILE_NAME_MAX];
        strncpy(temp_filename, filename, FILE_NAME_MAX - 1);
        if (!file_exists(temp_filename, NOT_LOCALIZED)) {
            platform_file_manager_cache_add_file_info(filename);
        }
    }
#endif
    return fopen(filename, mode);
}

int platform_file_manager_remove_file(const char *filename)
{
#ifdef USE_FILE_CACHE
    platform_file_manager_cache_delete_file_info(filename);
#endif
    return remove(filename) == 0;
}

FILE *platform_file_manager_open_asset(const char *asset, const char *mode)
{
    set_assets_directory();
    const char *cased_asset_path = dir_get_asset(assets_directory, asset);
    return fopen(cased_asset_path, mode);
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

int platform_file_manager_create_directory(const char *name)
{
#ifdef _WIN32
    if (CreateDirectoryA(name, 0) != 0) {
        return 1;
    } else {
        return GetLastError() == ERROR_ALREADY_EXISTS;
    }
#else
    if (mkdir(name, 0744) == 0) {
        return 1;
    } else {
        return errno == EEXIST;
    }
#endif
}

