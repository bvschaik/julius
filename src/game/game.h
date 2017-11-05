#ifndef GAME_H
#define GAME_H

#include "game/settings.h"

struct Game
{
    int preInit();
    int init();
    void exit();

    GameSettings settings;
};

extern Game game;


#endif
