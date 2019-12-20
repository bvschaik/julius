#include "android/android.h"
#include "core/file.h"
#include "core/log.h"
#include "file.h"
#include "vita/vita.h"

#include "SDL.h"
#include <dirent.h>
#include <string.h>

#ifdef _MSC_VER
#include <direct.h>
#define chdir _chdir
#elif !defined(__vita__) || !defined(__ANDROID__)
#include <unistd.h>
#endif

static int has_file_access_permissions = 1;
static char base_path[FILE_NAME_MAX];
static int base_path_length = 0;

#ifdef _WIN32
const char *filename_to_utf8(const wchar_t *str)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    char* result = (char*)malloc(sizeof(char) * size_needed);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, result, size_needed, NULL, NULL);
    return result;
}
#endif

void platform_check_file_access_permissions(void)
{
    // Only used by Android so far
#ifdef __ANDROID__
    if (!has_file_access_permissions) {
        android_request_rw_permissions();
        // Check again before leaving
        has_file_access_permissions = android_check_rw_permissions();
        if (!has_file_access_permissions) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                     "Permissions not set",
                                     "Julius needs permissions to read and write to the SD Card in order to load the original C3 assets.\n\n"
                                     "Please add permissions to read and write in Settings -> Applications.",
                                     NULL);
            SDL_Log("Exiting: no permissions to read or write files.");
            exit(1);
        }
        android_toast_message("Permission to read and write files granted.");
    }
#endif
}

void platform_set_file_access_permissions(int permissions)
{
    has_file_access_permissions = permissions;
}

const char* platform_get_base_path(void)
{
    return base_path;
}

int platform_set_base_path(const char* path)
{
    if (!path) {
        log_error("set_base_path: path was not set. Julius will probably crash.", 0, 0);
        return 0;
    }
#ifdef __vita__
    path = VITA_PATH_PREFIX;
#endif

#if defined(__vita__) || defined(__ANDROID__)
    // Windows will not get here, so opendir can be safely used
    DIR* dir = opendir(path);
    if (dir) {
        closedir(dir);
    } else {
        return 0;
    }

    if (strlen(path) >= FILE_NAME_MAX) {
        SDL_Log("set_base_path: the path length was too long and was truncated. Julius will probably not work.");
    }
    base_path_length = snprintf(base_path, FILE_NAME_MAX - 1, "%s", path);
    return 1;
#else
    return chdir(path) == 0;
#endif
}

int platform_generate_full_file_path(char *full_path, const char *filepath)
{
#if defined (__vita__) || defined (__ANDROID__)
    strcpy(full_path, base_path);
#endif
    strncpy(full_path + base_path_length, filepath, FILE_NAME_MAX);
    return base_path_length;
}
