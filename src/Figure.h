#ifndef FIGURE_H
#define FIGURE_H

#include "building/building.h"
#include "figure/figure.h"

void Figure_createDustCloud(int x, int y, int size);
void Figure_createFishingPoints();
void Figure_createHerds();
void Figure_createFlotsam();
void Figure_createMissile(int buildingId, int x, int y, int xDst, int yDst, int type);

void Figure_killTowerSentriesAt(int x, int y);
void Figure_sinkAllShips();
int Figure_getCitizenOnSameTile(int figureId);
int Figure_getNonCitizenOnSameTile(int figureId);
int Figure_hasNearbyEnemy(int xStart, int yStart, int xEnd, int yEnd);

int Figure_provideServiceCoverage(figure *f);

#endif
