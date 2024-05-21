#ifndef CORE_DIR_H
#define CORE_DIR_H

#define ASSETS_DIR_NAME "assets"
#define ASSETS_DIRECTORY "***" ASSETS_DIR_NAME "***"
#define CAMPAIGNS_DIR_NAME "campaigns"
#define CAMPAIGNS_DIRECTORY "***" CAMPAIGNS_DIR_NAME "***"

/**
 * @file
 * Directory-related functions.
 */

enum {
    NOT_LOCALIZED = 0,
    MAY_BE_LOCALIZED = 1,
    MUST_BE_LOCALIZED = 2
};

enum {
    PATH_LOCATION_ROOT = 0,
    PATH_LOCATION_CONFIG = 1,
    PATH_LOCATION_ASSET = 2,
    PATH_LOCATION_SAVEGAME = 3,
    PATH_LOCATION_SCENARIO = 4,
    PATH_LOCATION_CAMPAIGN = 5,
    PATH_LOCATION_SCREENSHOT = 6,
    PATH_LOCATION_COMMUNITY = 7,
    PATH_LOCATION_EDITOR_CUSTOM_EMPIRES = 8,
    PATH_LOCATION_EDITOR_CUSTOM_MESSAGES = 9,
    PATH_LOCATION_EDITOR_CUSTOM_EVENTS = 10,
    PATH_LOCATION_MAX = 11
};

/**
 * File information
 */
typedef struct {
    char *name; /**< Filenames in UTF-8 encoding */
    unsigned int modified_time; /**< Timestamp */
} dir_entry;

/**
 * Directory listing
 */
typedef struct {
    dir_entry *files; /**< Filenames and last modified time */
    int num_files; /**< Number of files in the list */
} dir_listing;

/**
 * Finds files with the given extension
 * @param dir The directory to search on, or null if base directory
 * @param extension Extension of the files to find
 * @return Directory listing
 */
const dir_listing *dir_find_files_with_extension(const char *dir, const char *extension);

/**
 * Finds files with the given extension at the requested location ID
 * @param location The location ID where the files should reside
 * @param extension Extension of the files to find
 * @return Directory listing
 */
const dir_listing *dir_find_files_with_extension_at_location(int location, const char *extension);

/**
 * Appends files with the given extension to the current directory listing
 * @param extension Extension of the files to find
 * @return Directory listing
 */
const dir_listing *dir_append_files_with_extension(const char *extension);

/**
 * Finds all subdirectories
 * @param dir The directory to search on, or null if base directory
 * @return Directory listing
 */
const dir_listing *dir_find_all_subdirectories(const char *dir);

/**
 * Finds all subdirectories at the requested location ID
 * @param location The location ID where the subdirectories should reside
 * @return Directory listing
 */
const dir_listing *dir_find_all_subdirectories_at_location(int location);

/**
 * Get the case sensitive and localized filename of the file
 * @param filepath File path to match to a case-sensitive file on the filesystem
 * @param localizable Whether the file may, must or must not be localized
 * @return Corrected file, or NULL if the file was not found
 */
const char *dir_get_file(const char *filepath, int localizable);

/**
 * Get the case sensitive filename from the requested location ID
 * @param filepath File path to match to a case-sensitive file on the filesystem
 * @param location The location ID where the file should reside
 * @return Corrected file, or NULL if the file was not found
 */
const char *dir_get_file_at_location(const char *filepath, int location);

/**
 * Appends the location to the filename
 * @param filename File path to append the location to
 * @param location The location ID to append to the filename
 * @return Filename with location appended
 */
const char *dir_append_location(const char *filename, int location);

#endif // CORE_DIR_H
