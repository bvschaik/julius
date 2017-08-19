#ifndef FILE_H
#define FILE_H

/**
 * @file
 * File-related functions.
 *
 * Methods related to extensions:
 * @li The extension starts from the first dot, double extensions are not supported
 * @li Extension parameters are expected to be 3 chars, without leading dot
 */

#define FILE_NAME_MAX 200

/**
 * Checks whether the file has the given extension
 * @param filename Filename to check
 * @param extension Extension
 * @return boolean true if the file has the given extension, false otherwise
 */
int file_has_extension(const char *filename, const char *extension);

/**
 * Replaces the current extension by the given new extension.
 * Filename is unchanged if there was no extension.
 * @param[in,out] filename Filename to change
 * @param new_extension New extension
 */
void file_change_extension(char *filename, const char *new_extension);

/**
 * Appends the extension to the file
 * @param[in,out] filename Filename to change
 * @param extension Extension to append
 */
void file_append_extension(char *filename, const char *extension);

/**
 * Removes the extension from the file
 * @param[in,out] filename Filename to change
 */
void file_remove_extension(char *filename);

/**
 * Check if file exists
 * @param filename Filename to check
 * @return boolean true if the file exists, false otherwise
 */
int file_exists(const char *filename);

#endif // FILE_H
