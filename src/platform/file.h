#ifndef PLATFORM_FILE_H
#define PLATFORM_FILE_H

#ifdef _WIN32
#include <windows.h>

#define fs_dir_type _WDIR
#define fs_dir_entry struct _wdirent
#define fs_dir_open _wopendir
#define fs_dir_close _wclosedir
#define fs_dir_read _wreaddir
#define dir_entry_name(d) filename_to_utf8(d->d_name)
#define dir_entry_close_name(n) free((void*)n)
#define FS_BASE_DIR L"."

const char* filename_to_utf8(const wchar_t *str);

#else // not _WIN32
#define fs_dir_type DIR
#define fs_dir_entry struct dirent
#define fs_dir_open opendir
#define fs_dir_close closedir
#define fs_dir_read readdir
#define dir_entry_name(d) ((d)->d_name)
#define dir_entry_close_name(n)
#define FS_BASE_DIR platform_get_base_path()
#endif

#ifdef __vita__
#define CURRENT_DIR VITA_PATH_PREFIX
#elif defined(_WIN32)
#define CURRENT_DIR L"."
#else
#define CURRENT_DIR "."
#endif

void platform_check_file_access_permissions(void);
const char* platform_get_base_path(void);
int platform_set_base_path(const char *path);
void platform_set_file_access_permissions(int permissions);
int platform_generate_full_file_path(char *full_path, const char *filepath);

#endif // PLATFORM_FILE_H
