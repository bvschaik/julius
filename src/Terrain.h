#ifndef TERRAIN_H
#define TERRAIN_H

void Terrain_addBuildingToGrids(int buildingId, int x, int y, int width, int height, int graphicId, int terrain);
void Terrain_removeBuildingFromGrids(int buildingId, int x, int y);

int Terrain_hasRoadAccess(int x, int y, int size, int *roadX, int *roadY);
int Terrain_hasRoadAccessHippodrome(int x, int y, int *roadX, int *roadY);
int Terrain_hasRoadAccessGranary(int x, int y, int *roadX, int *roadY);

int Terrain_getClosestRoadWithinRadius(int x, int y, int size, int radius, int *xTile, int *yTile);

int Terrain_isAdjacentToWall(int x, int y, int size);
int Terrain_isAdjacentToWater(int x, int y, int size);
int Terrain_getAdjacentRoadOrClearLand(int x, int y, int size, int *xTile, int *yTile);

void Terrain_setWithRadius(int x, int y, int size, int radius, unsigned short typeToAdd);
void Terrain_clearWithRadius(int x, int y, int size, int radius, unsigned short typeToKeep);

int Terrain_existsTileWithinAreaWithType(int x, int y, int size, unsigned short type);

int Terrain_existsTileWithinRadiusWithType(int x, int y, int size, int radius, unsigned short type);
int Terrain_existsClearTileWithinRadius(int x, int y, int size, int radius, int exceptGridOffset);
int Terrain_allTilesWithinRadiusHaveType(int x, int y, int size, int radius, unsigned short type);

void Terrain_markNativeLand(int x, int y, int size, int radius);
int Terrain_hasBuildingOnNativeLand();

void Terrain_initDistanceRing();
int Terrain_isAllRockAndTreesAtDistanceRing(int x, int y, int distance);
int Terrain_isAllMeadowAtDistanceRing(int x, int y, int distance);
void Terrain_addDesirability(int x, int y, int size, int desBase, int desStep, int desStepSize, int desRange);

#endif
