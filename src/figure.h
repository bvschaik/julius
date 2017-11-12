#ifndef FIGURE_H
#define FIGURE_H

void Figure_clearList();
int Figure_create(int figureType, int x, int y, char direction);
void Figure_delete(int figureId);

void Figure_createDustCloud(int x, int y, int size);
void Figure_createFishingPoints();
void Figure_createHerds();
void Figure_createFlotsam(int xEntry, int yEntry, int hasWater);
int Figure_createMissile(int buildingId, int x, int y, int xDst, int yDst, int type);

int Figure_createSoldierFromBarracks(int buildingId, int x, int y);
int Figure_createTowerSentryFromBarracks(int buildingId, int x, int y);

void Figure_addToTileList(int figureId);
void Figure_updatePositionInTileList(int figureId);
void Figure_removeFromTileList(int figureId);

void Figure_killTowerSentriesAt(int x, int y);
void Figure_sinkAllShips();
int Figure_getCitizenOnSameTile(int figureId);
int Figure_getNonCitizenOnSameTile(int figureId);
int Figure_hasNearbyEnemy(int xStart, int yStart, int xEnd, int yEnd);

int Figure_determinePhrase(int figureId);
int Figure_playPhrase(int figureId);
void Figure_playDieSound(int figureType);
void Figure_playHitSound(int figureType);

int Figure_provideServiceCoverage(int figureId);

void FigureRoute_clearList();
void FigureRoute_clean();
void FigureRoute_add(int figureId);
void FigureRoute_remove(int figureId);

void FigureGeneration_generateFiguresForBuildings();

#endif
