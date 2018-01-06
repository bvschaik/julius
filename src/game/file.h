#ifndef GAME_FILE_H
#define GAME_FILE_H

#include <stdint.h>

int game_file_start_scenario(const uint8_t *scenario_name);

int game_file_load_scenario_data(const char *scenario_file);

int game_file_load_saved_game(const char *filename);

int game_file_write_saved_game(const char *filename);

int game_file_delete_saved_game(const char *filename);

void game_file_write_mission_saved_game();

#endif // GAME_FILE_H
