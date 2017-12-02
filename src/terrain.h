#ifndef TERRAIN_H
#define TERRAIN_H

void Terrain_addBuildingToGrids(int buildingId, int x, int y, int size, int graphicId, int terrain);
void Terrain_removeBuildingFromGrids(int buildingId, int x, int y);

void Terrain_addWatersideBuildingToGrids(int buildingId, int x, int y, int size, int graphicId);
int Terrain_determineOrientationWatersideSize2(int x, int y, int adjustXY,
        int *orientationAbsolute, int *orientationRelative);
int Terrain_determineOrientationWatersideSize3(int x, int y, int adjustXY,
        int *orientationAbsolute, int *orientationRelative);

void Terrain_addRoadsForGatehouse(int x, int y, int orientation);
void Terrain_addRoadsForTriumphalArch(int x, int y, int orientation);

int Terrain_hasRoadAccess(int x, int y, int size, int *roadX, int *roadY);
int Terrain_hasRoadAccessHippodrome(int x, int y, int *roadX, int *roadY);
int Terrain_hasRoadAccessGranary(int x, int y, int *roadX, int *roadY);

int Terrain_getOrientationGatehouse(int x, int y);
int Terrain_getOrientationTriumphalArch(int x, int y);

int Terrain_getClosestRoadWithinRadius(int x, int y, int size, int radius, int *xTile, int *yTile);

int Terrain_isClear(int x, int y, int size, int disallowedTerrain, int graphicSet);

int Terrain_canSpawnFishingBoatInWater(int x, int y, int size, int *xTile, int *yTile);

int Terrain_isAdjacentToWall(int x, int y, int size);
int Terrain_isAdjacentToWater(int x, int y, int size);
int Terrain_isAdjacentToOpenWater(int x, int y, int size);
int Terrain_getAdjacentRoadOrClearLand(int x, int y, int size, int *xTile, int *yTile);

void Terrain_setWithRadius(int x, int y, int size, int radius, unsigned short typeToAdd);
void Terrain_clearWithRadius(int x, int y, int size, int radius, unsigned short typeToKeep);

int Terrain_existsTileWithinAreaWithType(int x, int y, int size, unsigned short type);

int Terrain_existsTileWithinRadiusWithType(int x, int y, int size, int radius, unsigned short type);
int Terrain_existsClearTileWithinRadius(int x, int y, int size, int radius, int exceptGridOffset, int *xTile, int *yTile);
int Terrain_allTilesWithinRadiusHaveType(int x, int y, int size, int radius, unsigned short type);
int Terrain_allHousesWithinWellRadiusHaveFountain(int buildingId, int radius);

int Terrain_getClosestRoadWithinRadius(int x, int y, int size, int radius, int *xTile, int *yTile);
int Terrain_getClosestReachableRoadWithinRadius(int x, int y, int size, int radius, int *xTile, int *yTile);
int Terrain_getRoadToLargestRoadNetwork(int x, int y, int size, int *xTile, int *yTile);
int Terrain_getRoadToLargestRoadNetworkHippodrome(int x, int y, int size, int *xTile, int *yTile);

int Terrain_getAdjacentRoadTilesForRoaming(int gridOffset, int *roadTiles);
int Terrain_getSurroundingRoadTilesForRoaming(int gridOffset, int *roadTiles);

int Terrain_getAdjacentRoadTilesForAqueduct(int gridOffset);

void Terrain_markNativeLand(int x, int y, int size, int radius);
int Terrain_hasBuildingOnNativeLand(int x, int y, int size, int radius);

void Terrain_markBuildingsWithinWellRadius(int buildingId, int radius);
int Terrain_isReservoir(int gridOffset);

int Terrain_isAllRockAndTreesAtDistanceRing(int x, int y, int distance);
int Terrain_isAllMeadowAtDistanceRing(int x, int y, int distance);
void Terrain_addDesirability(int x, int y, int size, int desBase, int desStep, int desStepSize, int desRange);

int Terrain_countTerrainTypeDirectlyAdjacentTo(int gridOffset, int terrainMask);
int Terrain_countTerrainTypeDiagonallyAdjacentTo(int gridOffset, int terrainMask);
int Terrain_hasTerrainTypeSameYAdjacentTo(int gridOffset, int terrainMask);
int Terrain_hasTerrainTypeSameXAdjacentTo(int gridOffset, int terrainMask);

void Terrain_updateEntryExitFlags(int remove);

int Terrain_isClearToBuild(int size, int x, int y, int terrainMask);
void Terrain_updateToPlaceBuildingToOverlay(int size, int x, int y, int terrainMask, int isAbsoluteXY);

int Terrain_getWallTileWithinRadius(int x, int y, int radius, int *xTile, int *yTile);
void Terrain_rotateMap(int direction);

int Terrain_Water_findOpenWaterForShipwreck(int figureId, int* xTile, int* yTile);
int Terrain_Water_getFreeDockDestination(int figureId, int *xTile, int *yTile);
int Terrain_Water_getQueueDockDestination(int* xTile, int* yTile);
int Terrain_Water_getWharfTileForNewFishingBoat(int figureId, int *xTile, int *yTile);
int Terrain_Water_findAlternativeTileForFishingBoat(int figureId, int *xTile, int *yTile);
int Terrain_Water_getNearestFishTile(int figureId, int *xTile, int *yTile);

#endif
