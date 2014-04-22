#ifndef LOADER_H
#define LOADER_H

int Loader_Graphics_initGraphics();

int Loader_Graphics_loadMainGraphics(int climate);

int Loader_Graphics_loadEnemyGraphics(int enemyId);

const char *Loader_Graphics_loadExternalImagePixelData(int graphicId);

int Loader_Model_loadC3ModelTxt();

void Loader_GameState_init();

#endif
