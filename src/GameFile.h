#ifndef GAMEFILE_H
#define GAMEFILE_H

int GameFile_loadSavedGame(const char *filename);

int GameFile_deleteSavedGame(const char *filename);

int GameFile_loadScenario(const char *filename);

#endif
