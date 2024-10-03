#ifndef GAME_FILE_IO_H
#define GAME_FILE_IO_H

#include "core/buffer.h"
#include "scenario/data.h"

typedef enum {
    SAVEGAME_FROM_CUSTOM_SCENARIO = 0,
    SAVEGAME_FROM_ORIGINAL_CAMPAIGN = 1,
    SAVEGAME_FROM_CUSTOM_CAMPAIGN = 2
} saved_game_origin;

typedef struct {
    struct {
        int mission;
        char scenario_name[MAX_SCENARIO_NAME];
        char campaign_name[FILE_NAME_MAX];
        saved_game_origin type;
    } origin;
    int treasury;
    int population;
    int month;
    int year;
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
} saved_game_info;

int game_file_io_read_scenario(const char *filename);

int game_file_io_read_scenario_from_buffer(buffer *buf);

int game_file_io_read_scenario_info(const char *filename, saved_game_info *info);

int game_file_io_read_scenario_info_from_buffer(buffer *buf, saved_game_info *info);

int game_file_io_write_scenario(const char *filename);

int game_file_io_read_saved_game(const char *filename, int offset);

int game_file_io_read_save_game_from_buffer(buffer *buf);

int game_file_io_read_saved_game_info(const char *filename, int offset, saved_game_info *info);

int game_file_io_read_saved_game_info_from_buffer(buffer *buf, saved_game_info *info);

int game_file_io_write_saved_game(const char *filename);

int game_file_io_delete_saved_game(const char *filename);

#endif // GAME_FILE_IO_H
