#ifndef CORE_IO_H
#define CORE_IO_H

#include "core/dir.h"

/**
 * @file
 * I/O functions.
 */

/**
 * Reads the entire file into the buffer
 * @param filepath File to read
 * @param buffer Buffer to read into
 * @param max_size Max size to read
 * @param localizable Whether the file may be localized (see core/dir.h)
 * @return Number of bytes read
 */
int io_read_file_into_buffer(const char *filepath, void *buffer, int max_size, int localizable);

/**
 * Reads part of the file into the buffer
 * @param filepath File to read
 * @param buffer Buffer to read into
 * @param size Number of bytes to read
 * @param offset_in_file Offset into the file to start reading
 * @param localizable Whether the file may be localized (see core/dir.h)
 */
int io_read_file_part_into_buffer(const char *filepath, void *buffer, int size, int offset_in_file, int localizable);

/**
 * Writes the entire buffer to the file
 * @param filepath File to write
 * @param buffer Buffer to write
 * @param size Number of bytes to write
 * @return Number of bytes written
 */
int io_write_buffer_to_file(const char *filepath, const void *buffer, int size);

#endif // CORE_IO_H
