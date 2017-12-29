#ifndef TERRAIN_H
#define TERRAIN_H

void Terrain_addBuildingToGrids(int buildingId, int x, int y, int size, int graphicId, int terrain);
void Terrain_removeBuildingFromGrids(int buildingId, int x, int y);

void Terrain_addRoadsForGatehouse(int x, int y, int orientation);
void Terrain_addRoadsForTriumphalArch(int x, int y, int orientation);

int Terrain_getOrientationGatehouse(int x, int y);
int Terrain_getOrientationTriumphalArch(int x, int y);

int Terrain_isClear(int x, int y, int size, int disallowedTerrain, int graphicSet);

int Terrain_canSpawnFishingBoatInWater(int x, int y, int size, int *xTile, int *yTile);

int Terrain_isAdjacentToWall(int x, int y, int size);
int Terrain_isAdjacentToWater(int x, int y, int size);
int Terrain_isAdjacentToOpenWater(int x, int y, int size);
int Terrain_getAdjacentRoadOrClearLand(int x, int y, int size, int *xTile, int *yTile);

int Terrain_getClosestReachableRoadWithinRadius(int x, int y, int size, int radius, int *xTile, int *yTile);
int Terrain_getRoadToLargestRoadNetwork(int x, int y, int size, int *xTile, int *yTile);
int Terrain_getRoadToLargestRoadNetworkHippodrome(int x, int y, int size, int *xTile, int *yTile);

int Terrain_getAdjacentRoadTilesForRoaming(int gridOffset, int *roadTiles);
int Terrain_getSurroundingRoadTilesForRoaming(int gridOffset, int *roadTiles);

int Terrain_getAdjacentRoadTilesForAqueduct(int gridOffset);

void Terrain_updateEntryExitFlags(int remove);

int Terrain_isClearToBuild(int size, int x, int y, int terrainMask);
void Terrain_updateToPlaceBuildingToOverlay(int size, int x, int y, int terrainMask, int isAbsoluteXY);

int Terrain_getWallTileWithinRadius(int x, int y, int radius, int *xTile, int *yTile);
void Terrain_rotateMap(int direction);

#endif
