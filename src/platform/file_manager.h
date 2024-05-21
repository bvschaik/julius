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
 * Gets a directory location for the specified type
 * @param location The location to get
 * @param user_directory The user directory to use, or 0 to use the one from the preferences
 * @return The path for the provided location, or an empty string if the location is the base path.
 *         If there is an actual path, it is guaranteeed to end with "/".
 *         The return value is a pointer to a static buffer, so it should be copied if needed.
 */
const char *platform_file_manager_get_directory_for_location(int location, const char *user_directory);

/**
 * Checks if a directory is writeable
 * @param directory The directory to check
 * @return 1 if the directory is writeable, 0 otherwise
 */
int platform_file_manager_is_directory_writeable(const char *directory);

/**
 * Gets the contents of a directory by the specified extension
 * @param dir The directory to search on, or null if base directory
 * @param type The file type to filter (dir, file or both)
 * @param extension The extension to filter
 * @param callback The function to call when a matched file is found
 * @return LIST_ERROR if error, LIST_MATCH if there was a match in the callback, LIST_NO_MATCH if no match was set
 */
int platform_file_manager_list_directory_contents(
    const char *dir, int type, const char *extension, int (*callback)(const char *, long));

/**
 * Indicates whether the file name casing should be checked
 * @return Whether file name casing should be checked
 */
int platform_file_manager_should_case_correct_file(void);

/**
 * Checks whether a filename contains a string
 * @param filename Filename to check
 * @param expression Expression that needs to be contained
 * @return 1 if the filename contains the expression, 0 otherwise
 */
int platform_file_manager_filename_contains(const char *filename, const char *expression);

/**
 * Compares two filenames in a case-insensitive manner
 * @param a Filename A
 * @param b Filename B
 * @return Negative if A < B, positive if A > B, zero if A == B
 */
int platform_file_manager_compare_filename(const char *a, const char *b);

/**
 * Checks if the filename starts with the given prefix in a case-insensitive manner
 * @param filename Filename to check
 * @param prefix Prefix to check for
 * @param prefix_len Length of the prefix
 * @return Negative if filename < prefix, 0 if filename starts with prefix, positive if filename > prefix
 */
int platform_file_manager_compare_filename_prefix(const char *filename, const char *prefix, int prefix_len);

/**
 * Opens a file
 * @param filename The file to open
 * @param mode The mode to open the file - refer to fopen()
 * @return A pointer to a FILE structure on success, NULL otherwise
 */
FILE *platform_file_manager_open_file(const char *filename, const char *mode);

/**
 * Opens an asset file
 * @param asset The asset file to open
 * @param mode The mode to open the asset file - refer to fopen()
 * @return A pointer to a FILE structure on success, NULL otherwise
 */
FILE *platform_file_manager_open_asset(const char *asset, const char *mode);


/**
 * Closes a file
 * @param stream A pointer to the FILE structure to close
 * @return 1 if the file was closed, 0 otherwise
 */
int platform_file_manager_close_file(FILE *stream);


/**
 * Removes a file
 * @param filename The file to remove
 * @return 1 if removal was successful, 0 otherwise
 */
int platform_file_manager_remove_file(const char *filename);

/**
 * Creates a directory
 * @param name The full path to the new directory
 * @param location The base location to create the directory. The game won't attempt to create the directories in location.
 * If location and name are similar, the game will create the directory in the location. Can be 0.
 * @param overwrite Whether to return error if overwriting the directory
 * @return 1 if creation was successful, 0 otherwise
 */
int platform_file_manager_create_directory(const char *name, const char *location, int overwrite);

/**
 * Copies a file
 * @param src The source file
 * @param dst The destination file
 * @return 1 if copying was successful, 0 otherwise
 */
int platform_file_manager_copy_file(const char *src, const char *dst);

/**
 * Copies a directory recursively
 * @param src The source directory
 * @param dst The destination directory
 * @param overwrite_files Whether to overwrite existing files
 * @return 1 if copying was successful, 0 otherwise
 */
int platform_file_manager_copy_directory(const char *src, const char *dst, int overwrite_files);

/**
 * Removes a directory recursively - use with care!!!
 * @param path The directory to remove
 * @return 1 if removing was successful, 0 otherwise
 */
int platform_file_manager_remove_directory(const char *path);

#endif // PLATFORM_FILE_MANAGER_H
