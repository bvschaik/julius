#ifndef GAME_FILE_IO_H
#define GAME_FILE_IO_H

#include "graphics/color.h"
#include "scenario/data.h"

typedef struct {
    int mission;
    int custom_mission;
    int treasury;
    int population;
    int month;
    int year;
    const color_t *minimap_image;
} saved_game_info;

typedef struct {
    uint8_t description[MAX_BRIEF_DESCRIPTION];
    int image_id;
    int start_year;
    int climate;
    int map_size;
    int total_invasions;
    int player_rank;
    int is_open_play;
    int open_play_id;
    scenario_win_criteria win_criteria;
    const color_t *minimap_image;
} scenario_info;

int game_file_io_read_scenario(const char *filename);

int game_file_io_read_scenario_info(const char *filename, scenario_info *info);

int game_file_io_write_scenario(const char *filename);

int game_file_io_read_saved_game(const char *filename, int offset);

int game_file_io_write_saved_game(const char *filename);

int game_file_io_read_saved_game_info(const char *filename, saved_game_info *info);

int game_file_io_delete_saved_game(const char *filename);

#endif // GAME_FILE_IO_H
