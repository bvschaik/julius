#ifndef TERRAINBRIDGE_H
#define TERRAINBRIDGE_H

int TerrainBridge_determineLengthAndDirection(int x, int y, int isShipBridge, int *length, int *direction);

void TerrainBridge_addToSpriteGrid(int x, int y, int isShipBridge);

void TerrainBridge_removeFromSpriteGrid(int gridOffset, int onlyMarkDeleted);

int TerrainBridge_countWalkersOnBridge(int gridOffset);

void TerrainBridge_updateSpriteIdsOnMapRotate(int ccw);

#endif
