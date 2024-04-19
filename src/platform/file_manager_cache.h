#ifndef FILE_MANAGER_CACHE_H
#define FILE_MANAGER_CACHE_H

#if defined(__vita__) || defined(__SWITCH__)
#define USE_FILE_CACHE

#include "core/file.h"

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

const dir_info *platform_file_manager_cache_get_dir_info(const char *dir);
int platform_file_manager_cache_file_has_extension(const file_info *f, const char *extension);
void platform_file_manager_cache_add_file_info(const char *filename);
void platform_file_manager_cache_delete_file_info(const char *filename);
void platform_file_manager_cache_invalidate(void);

#endif

#endif // FILE_MANAGER_CACHE_H
