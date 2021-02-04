#ifndef PLATFORM_FILE_MANAGER_H
#define PLATFORM_FILE_MANAGER_H

#include <stdio.h>

enum {
    TYPE_NONE = 0,
    TYPE_DIR = 1,
    TYPE_FILE = 2,
    TYPE_ANY = 3
};

enum {
    LIST_ERROR = 0,
    LIST_NO_MATCH = 1,
    LIST_CONTINUE = 1,
    LIST_MATCH = 2
};

/**
 * Sets the base path for Julius
 * @param path The path to be set as the base
 * @return true if the base path was correctly set, false otherwise
 */
int platform_file_manager_set_base_path(const char *path);

/**
 * Gets the contents of a directory by the specified extension
 * @param dir The directory to search on, or null if base directory
 * @param type The file type to filter (dir, file or both)
 * @param extension The extension to filter
 * @param callback The function to call when a matched file is found
 * @return LIST_ERROR if error, LIST_MATCH if there was a match in the callback, LIST_NO_MATCH if no match was set
 */
int platform_file_manager_list_directory_contents(
    const char *dir, int type, const char *extension, int (*callback)(const char *));

/**
 * Indicates whether the file name casing should be checked
 * @return Whether file name casing should be checked
 */
int platform_file_manager_should_case_correct_file(void);

/**
 * Opens a file
 * @param filename The file to open
 * @param mode The mode to open the file - refer to fopen()
 * @return A pointer to a FILE structure on success, NULL otherwise
 */
FILE *platform_file_manager_open_file(const char *filename, const char *mode);

/**
 * Opens an asset file
 * @param filename The asset file to open
 * @param mode The mode to open the asset file - refer to fopen()
 * @return A pointer to a FILE structure on success, NULL otherwise
 */
FILE *platform_file_manager_open_asset(const char *asset, const char *mode);

/**
 * Removes a file
 * @param filename The file to remove
 * @return true if removal was successful, false otherwise
 */
int platform_file_manager_remove_file(const char *filename);

#endif // PLATFORM_FILE_MANAGER_H
