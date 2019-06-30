#ifndef GAME_FILE_EDITOR_H
#define GAME_FILE_EDITOR_H

/**
 * Create data for a new scenario
 * @param size Size of the map to create
 */
void game_file_editor_create_scenario(int size);

/**
 * Load scenario from disk and init it for using in the editor
 * @param scenario_file File to load
 * @return Boolean true on success, false on failure
 */
int game_file_editor_load_scenario(const char *scenario_file);

/**
 * Write scenario to disk
 * @param scenario_file File to save tos
 * @return Boolean true on success, false on failure
 */
int game_file_editor_write_scenario(const char *scenario_file);

#endif // GAME_FILE_EDITOR_H
