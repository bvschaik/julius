#ifndef TERRAIN_H
#define TERRAIN_H

int Terrain_hasRoadAccess(int x, int y, int size, int *roadX, int *roadY);

int Terrain_getClosestRoadWithinRadius(int x, int y, int size, int radius, int *xTile, int *yTile);

int Terrain_isAdjacentToWall(int x, int y, int size);
int Terrain_isAdjacentToWater(int x, int y, int size);
int Terrain_getAdjacentRoadOrClearLand(int x, int y, int size, int *xTile, int *yTile);

void Terrain_setWithRadius(int x, int y, int size, int radius, unsigned short typeToAdd);
void Terrain_clearWithRadius(int x, int y, int size, int radius, unsigned short typeToKeep);

int Terrain_existsTileWithinRadiusWithType(int x, int y, int size, int radius, unsigned short type);
int Terrain_existsClearTileWithinRadius(int x, int y, int size, int radius, int exceptGridOffset);
int Terrain_allTilesWithinRadiusHaveType(int x, int y, int size, int radius, unsigned short type);

void Terrain_markNativeLand(int x, int y, int size, int radius);
int Terrain_hasBuildingOnNativeLand();

#endif
