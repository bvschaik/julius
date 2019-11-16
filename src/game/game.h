#ifndef GAME_GAME_H
#define GAME_GAME_H

#include "input/cursor.h"

enum {
    GAME_INIT_ERROR = 0,
    GAME_INIT_OK = 1,
    GAME_INIT_NO_PATCH = 2
};

int game_pre_init(void);

int game_init(cursor_scale cur_scale);

int game_init_editor(void);

void game_run(void);

void game_draw(void);

void game_exit_editor(void);

void game_exit(void);

#endif // GAME_GAME_H
