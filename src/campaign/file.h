#ifndef CAMPAIGN_FILE_H
#define CAMPAIGN_FILE_H

#include <stddef.h>

int campaign_file_exists(const char *file);
void *campaign_file_load(const char *file, size_t *length);
void campaign_file_set_path(const char *path);
const char *campaign_file_remove_prefix(const char *path);
int campaign_file_is_zip(void);
int campaign_file_open_zip(void);
void campaign_file_close_zip(void);

#endif // CAMPAIGN_FILE_H
