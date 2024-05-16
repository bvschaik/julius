#ifndef PLATFORM_USER_PATH_H
#define PLATFORM_USER_PATH_H

/**
 * Recommends a user path
 * Only works on Windows right now
 *
 * @return The recommended user path, or 0 if unsupported
 */
const char *platform_user_path_recommend(void);

/**
 * Creates subdirectories for a user path
 */
void platform_user_path_create_subdirectories(void);

/**
 * Copies files from the default user path to the new user path
 *
 * @param original_user_path The user path to copy files from
 * @param overwrite Whether to overwrite existing files
 */
void platform_user_path_copy_files(const char *original_user_path, int overwrite);

/**
 * Copies campaigns and custom empires from the default place to the new subdirectories
 */
void platform_user_path_copy_campaigns_and_custom_empires(void);

#endif // PLATFORM_USER_PATH_H

