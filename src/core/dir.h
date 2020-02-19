#ifndef CORE_DIR_H
#define CORE_DIR_H

/**
 * @file
 * Directory-related functions.
 */

#define DIR_MAX_FILES 200
#define DIR_MAX_SAVE_FILES (DIR_MAX_FILES * 10)

/**
 * Directory listing
 */
typedef struct {
    char *files[DIR_MAX_SAVE_FILES]; /**< Filenames in UTF-8 encoding */
    int num_files; /**< Number of files in the list */
} dir_listing;

/**
 * Finds files with the given extension
 * @param extension Extension of the files to find
 * @return Directory listing
 */
const dir_listing *dir_find_files_with_extension(const char *extension);

/**
 * Get the case sensitive filename of the file
 * @param filepath File path to match to a case-sensitive file on the filesystem
 * @return Case-corrected file, or NULL if the file was not found
 */
const char *dir_get_case_corrected_file(const char *filepath);

#endif // CORE_DIR_H
