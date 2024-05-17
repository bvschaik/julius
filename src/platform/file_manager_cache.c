#include "file_manager_cache.h"

#ifdef USE_FILE_CACHE

#include "core/string.h"
#include "platform/file_manager.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

enum {
    STAT_DOESNT_WORK = -1,
    STAT_UNTESTED = 0,
    STAT_WORKS = 1
};

static dir_info *base_dir_info;
static int stat_status;

const dir_info *platform_file_manager_cache_get_dir_info(const char *dir)
{
    dir_info *info = base_dir_info;
    if (info) {
        while (1) {
            if (strcmp(info->name, dir) == 0) {
                return info;
            }
            if (!info->next) {
                break;
            }
            info = info->next;
        }
    }
    DIR *d = opendir(dir);
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
    snprintf(info->name, FILE_NAME_MAX, "%s", dir);
    info->first_file = 0;
    info->next = 0;
    struct dirent *entry;
    file_info *file_item = 0;
    char full_name[FILE_NAME_MAX];
    size_t dir_name_offset = 0;
    if (*info->name) {
        dir_name_offset = snprintf(full_name, FILE_NAME_MAX, "%s/", info->name);
    }

    while ((entry = readdir(d))) {
        const char *name = entry->d_name;
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
        snprintf(file_item->name, FILE_NAME_MAX, "%s", name);
        snprintf(&full_name[dir_name_offset], FILE_NAME_MAX - dir_name_offset, "%s", name);

        // Copy extension
        file_item->extension = strrchr(file_item->name, '.');
        if (!file_item->extension) {
            file_item->extension = file_item->name + strlen(file_item->name);
        } else {
            file_item->extension++;
        }

        // Check type
        int type = TYPE_FILE;
        struct stat current_file_info;

        int has_stat = 0;

        if (stat_status == STAT_UNTESTED) {
            stat_status = stat(full_name, &current_file_info) == 0 ? STAT_WORKS : STAT_DOESNT_WORK;
            has_stat = stat_status == STAT_WORKS;
        } else if (stat_status == STAT_WORKS) {
            has_stat = stat(full_name, &current_file_info) == 0;
        }
        if (has_stat) {
            if (S_ISDIR(current_file_info.st_mode)) {
                type = TYPE_DIR;
            }
            file_item->modified_time = current_file_info.st_mtime;
        } else {
            // When stat does not work, we check if a file is a directory by trying to open it as a dir
            // For performance reasons, we only check for a directory if the name has no extension
            // This is effectively a hack, and definitely not full-proof, but the performance gains are well worth it
            if (!*file_item->extension) {
                DIR *file_d = opendir(full_name);
                if (file_d) {
                    type = TYPE_DIR;
                    closedir(file_d);
                }
            }
            file_item->modified_time = 0;
        }
        file_item->type = type;
    }
    closedir(d);
    return info;
}

void platform_file_manager_cache_update_file_info(const char *filename)
{
    // Augustus only modifies files in the base dir
    if (!base_dir_info) {
        return;
    }
    file_info *current_file = 0;
    for (current_file = base_dir_info->first_file; current_file; current_file = current_file->next) {
        if (strcmp(filename, current_file->name) == 0) {
            break;
        }
    }
    if (!current_file) {
        current_file = malloc(sizeof(file_info));
        snprintf(current_file->name, FILE_NAME_MAX, "%s", filename);
        current_file->type = TYPE_FILE;
        char c;
        const char *name = current_file->name;
        do {
            c = *name;
            name++;
        } while (c != '.' && c);
        current_file->extension = name;
        current_file->next = base_dir_info->first_file;
        base_dir_info->first_file = current_file;
    }
    current_file->modified_time = time(0);
}

void platform_file_manager_cache_delete_file_info(const char *filename)
{
    // Augustus only deletes files from the base dir
    if (!base_dir_info) {
        return;
    }
    file_info *prev = 0;
    for (file_info *current_file = base_dir_info->first_file; current_file; current_file = current_file->next) {
        if (strcmp(filename, current_file->name) == 0) {
            if (prev) {
                prev->next = current_file->next;
            } else {
                base_dir_info->first_file = current_file->next;
            }
            free(current_file);
            return;
        }
        prev = current_file;
    }
}

int platform_file_manager_cache_file_has_extension(const file_info *f, const char *extension)
{
    if (!(f->type & TYPE_FILE) || !extension || !*extension) {
        return 1;
    }
    return platform_file_manager_compare_filename(f->extension, extension) == 0;
}

void platform_file_manager_cache_invalidate(void)
{
    dir_info *info = base_dir_info;
    while (info) {
        file_info *file_item = info->first_file;
        while (file_item) {
            file_info *old_file_item = file_item;
            file_item = file_item->next;
            free(old_file_item);
        }
        dir_info *old_info = info;
        info = info->next;
        free(old_info);
    }
    base_dir_info = 0;
}

#endif // USE_FILE_CACHE
