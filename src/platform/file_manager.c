#include "file_manager.h"

#include "assets/assets.h"
#include "core/config.h"
#include "core/file.h"
#include "core/log.h"
#include "core/random.h"
#include "core/string.h"
#include "platform/android/android.h"
#include "platform/emscripten/emscripten.h"
#include "platform/file_manager_cache.h"
#include "platform/platform.h"
#include "platform/prefs.h"
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
#include <sys/utime.h>

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

static void copy_file_times(FILE *src, FILE *dst)
{
    stat_info file_info;
    if (_fstat(_fileno(src), &file_info) == -1) {
        return;
    }
    struct _utimbuf ut;
    ut.actime = file_info.st_atime;
    ut.modtime = file_info.st_mtime;
    _futime(_fileno(dst), &ut);
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

#ifdef __SWITCH__
static void copy_file_times(const char *src, const char *dst)
{
    stat_info file_info;
    if (stat(src, &file_info) == -1) {
        return;
    }
    struct timeval times[2] = { 0 };
    times[0].tv_sec = file_info.st_atime;
    times[1].tv_sec = file_info.st_mtime;
    utimes(dst, times);
}
#else
static void copy_file_times(FILE *src, FILE *dst)
{
    stat_info file_info;
    if (fstat(fileno(src), &file_info) == -1) {
        return;
    }
    struct timespec times[2] = { 0 };
    times[0].tv_sec = file_info.st_atime;
    times[1].tv_sec = file_info.st_mtime;
    futimens(fileno(dst), times);
}
#endif

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
static const char *const assets_directory = ASSETS_DIRECTORY;
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
    snprintf(dest, FILE_NAME_MAX, "%s", base_path);
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
    int overwrite_files;
} directory_copy_data;

static struct {
    const char *root;
    const char *configs;
    const char *assets;
    const char *savegames;
    const char *scenarios;
    const char *campaigns;
    const char *screenshots;
    const char *community;
    const char *editor_custom_empires;
    const char *editor_custom_messages;
    const char *editor_custom_events;
} paths = {
    "",
    "config/",
    assets_directory,
    "savegames/",
    "scenarios/",
    "campaigns/",
    "screenshots/",
    "community/",
    "editor/empires/",
    "editor/messages/",
    "editor/events/"
};

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
            snprintf(assets_directory, FILE_NAME_MAX, "%s%s", home_dir, &ASSET_DIRS[i][1]);
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
            snprintf(parent, FILE_NAME_MAX - (parent - assets_directory), "/share/augustus-game");
#endif
        } else if (strcmp(ASSET_DIRS[i], "***EXEC_PATH***") == 0) {
#if defined(_WIN32) || defined(__vita__) || defined(__SWITCH__) || defined(__APPLE__)
            log_error("***EXEC_PATH*** is not available on your platform.", 0, 0);
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
            snprintf(assets_directory, FILE_NAME_MAX, "%s", arg0_dir);
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
            if (snprintf(assets_directory, FILE_NAME_MAX, "%s/../share/augustus-game", arg0_dir) > FILE_NAME_MAX) {
                log_error("Path too long", arg0_dir, 0);
                continue;
            }
#endif
        } else {
            snprintf(assets_directory, FILE_NAME_MAX, "%s", ASSET_DIRS[i]);
        }
        size_t offset = strlen(assets_directory);
        offset += snprintf(&assets_directory[offset], FILE_NAME_MAX - offset, "/");
        // Special case for romfs on switch
        if (strcmp(assets_directory, "romfs:/") != 0) {
            snprintf(&assets_directory[offset], FILE_NAME_MAX - offset, "%s", ASSETS_DIR_NAME);
        }
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
    snprintf(assets_directory, FILE_NAME_MAX, ".");
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
            if (*assets_directory && assets_directory[strlen(assets_directory) - 1] == '/' &&
                dir[assets_directory_length] == '/') {
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

const char *platform_file_manager_get_directory_for_location(int location, const char *user_directory)
{
    static char full_path[FILE_NAME_MAX];
    if (!user_directory) {
        user_directory = pref_user_dir();
    }
    int cursor = 0;
    char slash[2] = { 0 };
    size_t user_directory_length = strlen(user_directory);
    if (user_directory_length &&
        user_directory[user_directory_length - 1] != '/' && user_directory[user_directory_length - 1] != '\\') {
        slash[0] = '/';
    }

    switch (location) {
        default:
            full_path[0] = 0;
            break;
        case PATH_LOCATION_CONFIG:
            if (*user_directory) {
                cursor = snprintf(full_path, FILE_NAME_MAX, "%s%s%s", user_directory, slash, paths.configs);
            } else {
                full_path[0] = 0;
            }
            break;
        case PATH_LOCATION_ASSET:
            set_assets_directory();
            cursor = snprintf(full_path, FILE_NAME_MAX, "%s", paths.assets);
            break;
        case PATH_LOCATION_SAVEGAME:
            if (*user_directory) {
                cursor = snprintf(full_path, FILE_NAME_MAX, "%s%s%s", user_directory, slash, paths.savegames);
            } else {
                full_path[0] = 0;
            }
            break;
        case PATH_LOCATION_SCENARIO:
            if (*user_directory) {
                cursor = snprintf(full_path, FILE_NAME_MAX, "%s%s%s", user_directory, slash, paths.scenarios);
            } else {
                full_path[0] = 0;
            }
            break;
        case PATH_LOCATION_CAMPAIGN:
            cursor = snprintf(full_path, FILE_NAME_MAX, "%s%s%s", user_directory, slash, paths.campaigns);
            break;
        case PATH_LOCATION_SCREENSHOT:
            if (*user_directory) {
                cursor = snprintf(full_path, FILE_NAME_MAX, "%s%s%s", user_directory, slash, paths.screenshots);
            } else {
                full_path[0] = 0;
            }
            break;
        case PATH_LOCATION_COMMUNITY:
            cursor = snprintf(full_path, FILE_NAME_MAX, "%s%s%s", user_directory, slash, paths.community);
            break;
        case PATH_LOCATION_EDITOR_CUSTOM_EMPIRES:
            cursor = snprintf(full_path, FILE_NAME_MAX, "%s%s%s", user_directory, slash, paths.editor_custom_empires);
            break;
        case PATH_LOCATION_EDITOR_CUSTOM_EVENTS:
            cursor = snprintf(full_path, FILE_NAME_MAX, "%s%s%s", user_directory, slash, paths.editor_custom_events);
            break;
        case PATH_LOCATION_EDITOR_CUSTOM_MESSAGES:
            cursor = snprintf(full_path, FILE_NAME_MAX, "%s%s%s", user_directory, slash, paths.editor_custom_messages);
            break;
    }

    if (cursor >= FILE_NAME_MAX) {
        log_error("Path ID too long for location: ", 0, location);
    }
    return full_path;
}

int platform_file_manager_is_directory_writeable(const char *directory)
{
    char file_name[FILE_NAME_MAX];
    if (!directory || !*directory) {
        directory = ".";
    }
    int attempt = 0;
    do {
        snprintf(file_name, FILE_NAME_MAX, "%s/test_%d.txt", directory, random_from_stdlib());
        attempt++;
    } while (file_exists(file_name, NOT_LOCALIZED) && attempt < 5);
    FILE *fp = file_open(file_name, "w");
    if (!fp) {
        return 0;
    }
    fclose(fp);
    if (!file_remove(file_name)) {
        return 0;
    }
    return 1;
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
    const char *cased_asset_path = dir_get_file_at_location(asset, PATH_LOCATION_ASSET);
    return cased_asset_path ? platform_file_manager_open_file(cased_asset_path, mode) : 0;
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

int platform_file_manager_create_directory(const char *name, const char *location, int overwrite)
{
    char tokenized_name[FILE_NAME_MAX];
    char temporary_path[FILE_NAME_MAX] = { 0 };
    int overwrite_last = 0;
    int cursor = 0;
    if (location) {
        cursor = snprintf(temporary_path, FILE_NAME_MAX, "%s", location);
        if (cursor > FILE_NAME_MAX) {
            log_error("Path too long", name, 0);
            return 0;
        }
        size_t location_length = strlen(location);
        if (strncmp(name, location, location_length) == 0) {
            name += location_length;
        }
    }
    if (*name == '/' || *name == '\\') {
        cursor += snprintf(&temporary_path[cursor], FILE_NAME_MAX - cursor, "/");
        name++;
    }
    snprintf(tokenized_name, FILE_NAME_MAX, "%s", name);
    char *token = strtok(tokenized_name, "/\\");

    while (token) {
        overwrite_last = 0;
        cursor += snprintf(&temporary_path[cursor], FILE_NAME_MAX - cursor, "%s", token);
        if (cursor > FILE_NAME_MAX) {
            log_error("Path too long", name, 0);
            return 0;
        }
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
#elif defined (__ANDROID__)
        int result = android_create_directory(temporary_path);
        if (result != 1) {
            if (result == 0) {
                return 0;
            } else if (!overwrite) {
                overwrite_last = 1;                
            }
        }
#else
        if (mkdir(temporary_path, 0744) != 0) {
            if (errno != EEXIST) {
                return 0;
            } else if (!overwrite) {
                overwrite_last = 1;
            }
        }
#endif
        cursor += snprintf(&temporary_path[cursor], FILE_NAME_MAX - cursor, "/");
        if (cursor > FILE_NAME_MAX) {
            log_error("Path too long", name, 0);
            return 0;
        }
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
    out = platform_file_manager_open_file(dst, "rb");
    if (!out) {
        fclose(in);
        return 1;
    }
#ifndef __SWITCH__
    copy_file_times(in, out);
#endif
    file_close(in);
    file_close(out);

#ifdef __SWITCH__
    copy_file_times(src, dst);
#endif

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
    if (!directory_copy_data.overwrite_files) {
        FILE *file = platform_file_manager_open_file(directory_copy_data.current_dst_path, "rb");
        if (file) {
            file_close(file);
            return LIST_CONTINUE;
        }
    }
    int result = platform_file_manager_copy_file(directory_copy_data.current_src_path,
        directory_copy_data.current_dst_path) != 0;

    move_up_path();

    return result;
}

static int copy_directory(const char *name, long unused)
{
    if (name) {
        append_name_to_path(name);
    }
    if (!platform_file_manager_create_directory(directory_copy_data.current_dst_path, 0, 0)) {
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
}

static void copy_directory_name(const char *name, char *dst)
{
    if (strncmp(name, ASSETS_DIRECTORY, strlen(ASSETS_DIRECTORY)) == 0) {
        set_assets_directory();
        if (strlen(name) == strlen(ASSETS_DIRECTORY)) {
            snprintf(dst, FILE_NAME_MAX, "%s", assets_directory);
        } else {
            snprintf(dst, FILE_NAME_MAX, "%s%s", assets_directory, name + strlen(assets_directory));
        }
    } else {
        snprintf(dst, FILE_NAME_MAX, "%s", name);
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

static int do_nothing(const char *name, long unused)
{
    return LIST_MATCH;
}

int platform_file_manager_copy_directory(const char *src, const char *dst, int overwrite_files)
{
    if (!platform_file_manager_list_directory_contents(src, TYPE_DIR, 0, do_nothing)) {
        return 0;
    }
    copy_directory_name(src, directory_copy_data.current_src_path);
    copy_directory_name(dst, directory_copy_data.current_dst_path);
    directory_copy_data.overwrite_files = overwrite_files;
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
    if (name) {
        append_name_to_path(name);
    }

    // Remove subdirs
    int result = platform_file_manager_list_directory_contents(directory_copy_data.current_src_path,
            TYPE_FILE, 0, remove_file) != LIST_ERROR &&
        platform_file_manager_list_directory_contents(directory_copy_data.current_src_path,
            TYPE_DIR, 0, remove_directory) != LIST_ERROR;

    if (result) {
#ifdef _WIN32
        wchar_t *wdir = utf8_to_wchar(directory_copy_data.current_src_path);
        result = RemoveDirectoryW(wdir) != 0;
        free(wdir);
#elif defined(__ANDROID__)
        return android_remove_file(directory_copy_data.current_src_path);
#else
        result = remove(directory_copy_data.current_src_path) == 0;
#endif
    }

    move_up_path();

    return result;
}

int platform_file_manager_remove_directory(const char *path)
{
    copy_directory_name(path, directory_copy_data.current_src_path);
    return remove_directory(0, 0);
}
