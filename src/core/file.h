#ifndef CORE_FILE_H
#define CORE_FILE_H

#include "core/dir.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @file
 * File-related functions.
 *
 * Methods related to extensions:
 * @li The extension starts from the first dot, double extensions are not supported
 * @li Extension parameters are expected to be 3 chars, without leading dot
 */

#define FILE_NAME_MAX 300

/**
 * Wrapper for fopen converting filename to path in current working directory
 * @param filename Filename
 * @param mode Mode to open the file (e.g. "wb").
 * @return FILE
 */
FILE *file_open(const char *filename, const char *mode);

/**
 * Wrapper to fclose
 * @return See fclose (If the stream is successfully closed, a zero value is returned.
 *         On failure, EOF is returned.)
 */
int file_close(FILE *stream);

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
void file_remove_extension(uint8_t *filename);

/**
 * Check if file exists
 * @param filename Filename to check
 * @param localizable Whether the file may be localized (see core/dir.h)
 * @return boolean true if the file exists, false otherwise
 */
int file_exists(const char *filename, int localizable);

/**
 * Remove a file
 * @param filename Filename to remove
 * @return boolean true if the file removal was successful, false otherwise
 */
int file_remove(const char *filename);

#endif // CORE_FILE_H
