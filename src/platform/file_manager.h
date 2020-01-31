#ifndef PLATFORM_FILE_MANAGER_H
#define PLATFORM_FILE_MANAGER_H

#include <stdio.h>

/**
 * Sets the base path for Julius
 * @param path The path to be set as the base
 * @return true if the base path was correctly set, false otherwise
 */
int platform_file_manager_set_base_path(const char *path);

/**
 * Gets the contents of a directory by the specified extention
 * @param file_list An array of the found file names
 * @param file_count The number of files found
 * @param extension The extension to filter
 * @param max_files The maximum number of possible entries in file_list
 * @return true if successful, false otherwise
 */
int platform_file_manager_get_directory_contents_by_extension(char **file_list, int *file_count, const char *extension, int max_files);

/**
 * Corrects a file name to be case corrected
 * @param filepath The path to be corrected
 * @return A string with the corrected file name, or NULL if the requested file does not exist
 */
const char *platform_file_manager_case_correct_file(const char *filepath);

/**
 * Opens a file
 * @param filename The file to open
 * @param mode The mode to open the file - refer to fopen()
 * @return A pointer to a FILE structure on success, NULL otherwise
 */
FILE *platform_file_manager_open_file(const char *filename, const char *mode);

/**
 * Checks if a file exists
 * @param filename The file to check
 * @return true if the file exists, false otherwise
 */
int platform_file_manager_file_exists(const char *filename);

/**
 * Removes a file
 * @param filename The file to remove
 * @return true if removal was successful, false otherwise
 */
int platform_file_manager_remove_file(const char *filename);

#endif // PLATFORM_FILE_MANAGER_H
