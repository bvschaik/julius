#ifndef GAME_FILE_IO_H
#define GAME_FILE_IO_H

int game_file_io_read_scenario(const char *filename);

int game_file_io_write_scenario(const char *filename);

int game_file_io_read_saved_game(const char *filename, int offset);

int game_file_io_write_saved_game(const char *filename);

int game_file_io_delete_saved_game(const char *filename);

#endif // GAME_FILE_IO_H
