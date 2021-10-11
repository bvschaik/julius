#ifndef GAME_FILE_IO_H
#define GAME_FILE_IO_H

#include "graphics/color.h"

typedef struct {
    int mission;
    int custom_mission;
    int treasury;
    int population;
    int month;
    int year;
    const color_t *minimap_image;
    int minimap_image_width;
    int minimap_image_height;
} saved_game_info;

int game_file_io_read_scenario(const char *filename);

int game_file_io_write_scenario(const char *filename);

int game_file_io_read_saved_game(const char *filename, int offset);

int game_file_io_write_saved_game(const char *filename);

int game_file_io_read_saved_game_info(const char *filename, saved_game_info *info);

int game_file_io_delete_saved_game(const char *filename);

#endif // GAME_FILE_IO_H
