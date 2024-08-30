#include "platform/prefs.h"

#include "core/log.h"
#include "core/file.h"
#include "platform/platform.h"

#include "SDL.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    const char *filename;
    int retrieved;
    char location[FILE_NAME_MAX];
} directory;

static struct {
    directory data;
    directory user;
    int location_printed;
} prefs = {
    { .filename = "data_dir.txt" },
    { .filename = "user_dir.txt" }
};

static FILE *open_pref_file(const char *filename, const char *mode)
{
    char *pref_dir = platform_get_pref_path();
    if (!prefs.location_printed) {
        log_info("Pref dir location:", pref_dir ? pref_dir : ".", 0);
        prefs.location_printed = 1;
    }
    size_t file_len = strlen(filename) + strlen(pref_dir) + 1;
    char *pref_file = malloc(file_len * sizeof(char));
    if (!pref_file) {
        SDL_free(pref_dir);
        return NULL;
    }
    snprintf(pref_file, file_len, "%s%s", pref_dir ? pref_dir : "", filename);
    SDL_free(pref_dir);
    FILE *fp = fopen(pref_file, mode);
    free(pref_file);
    return fp;
}

static const char *retrieve_directory(directory *dir)
{
    if (dir->retrieved) {
        return dir->location;
    }
    FILE *fp = open_pref_file(dir->filename, "r");
    if (fp) {
        size_t length = fread(dir->location, 1, FILE_NAME_MAX - 1, fp);
        fclose(fp);
        if (length > 0) {
            dir->location[length] = 0;
        }
        dir->retrieved = 1;
    }
    return dir->location;
}

static void save_directory(directory *dir, const char *location)
{
    snprintf(dir->location, FILE_NAME_MAX, "%s", location);
    FILE *fp = open_pref_file(dir->filename, "w");
    if (fp) {
        fwrite(location, 1, strlen(location), fp);
        fclose(fp);
    }
    dir->retrieved = 1;
}

const char *pref_data_dir(void)
{
    return retrieve_directory(&prefs.data);
}

void pref_save_data_dir(const char *data_dir)
{
    save_directory(&prefs.data, data_dir);
}

const char *pref_user_dir(void)
{
    return retrieve_directory(&prefs.user);
}

void pref_save_user_dir(const char *user_dir)
{
    save_directory(&prefs.user, user_dir);
}
