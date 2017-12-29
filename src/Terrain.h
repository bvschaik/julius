#ifndef TERRAIN_H
#define TERRAIN_H

void Terrain_addRoadsForGatehouse(int x, int y, int orientation);
void Terrain_addRoadsForTriumphalArch(int x, int y, int orientation);

int Terrain_getOrientationGatehouse(int x, int y);
int Terrain_getOrientationTriumphalArch(int x, int y);

int Terrain_isClear(int x, int y, int size, int disallowedTerrain, int graphicSet);

void Terrain_updateEntryExitFlags(int remove);

int Terrain_isClearToBuild(int size, int x, int y, int terrainMask);
void Terrain_updateToPlaceBuildingToOverlay(int size, int x, int y, int terrainMask, int isAbsoluteXY);

void Terrain_rotateMap(int direction);

#endif
