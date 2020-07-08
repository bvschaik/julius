#include "platform/prefs.h"

#include "platform/platform.h"

#include "SDL.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static FILE *open_pref_file(const char *filename, const char *mode)
{
    #if SDL_VERSION_ATLEAST(2, 0, 1)
    if (platform_sdl_version_at_least(2, 0, 1)) {
        char *pref_dir = SDL_GetPrefPath("bvschaik", "julius");
        if (!pref_dir) {
            return NULL;
        }
        size_t dir_len = strlen(pref_dir);
        char *pref_file = malloc((strlen(filename) + dir_len + 2) * sizeof(char));
        if (!pref_file) {
            SDL_free(pref_dir);
            return NULL;
        }
        strcpy(pref_file, pref_dir);
        strcpy(&pref_file[dir_len], filename);
        SDL_free(pref_dir);

        FILE *fp = fopen(pref_file, mode);
        free(pref_file);
        return fp;
    }
    #endif
    return NULL;
}

const char *pref_data_dir(void)
{
    static char data_dir[1000];
    FILE *fp = open_pref_file("data_dir.txt", "r");
    if (fp) {
        size_t length = fread(data_dir, 1, 1000, fp);
        fclose(fp);
        if (length > 0) {
            data_dir[length] = 0;
            return data_dir;
        }
    }
    return NULL;
}

void pref_save_data_dir(const char *data_dir)
{
    FILE *fp = open_pref_file("data_dir.txt", "w");
    if (fp) {
        fwrite(data_dir, 1, strlen(data_dir), fp);
        fclose(fp);
    }
}
