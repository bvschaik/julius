#ifndef LOADER_H
#define LOADER_H

#include "Data/Types.h"

int Loader_Graphics_initGraphics();

int Loader_Graphics_loadMainGraphics(int climate);

int Loader_Graphics_loadEnemyGraphics(int enemyId);

const Color *Loader_Graphics_loadExternalImagePixelData(int graphicId);

void Loader_GameState_init();

#endif
