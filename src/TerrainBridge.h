#ifndef TERRAINBRIDGE_H
#define TERRAINBRIDGE_H

int TerrainBridge_determineLengthAndDirection(int x, int y, int isShipBridge, int *length, int *direction);

int TerrainBridge_addToSpriteGrid(int x, int y, int isShipBridge); // returns length

void TerrainBridge_removeFromSpriteGrid(int gridOffset, int onlyMarkDeleted);

int TerrainBridge_countFiguresOnBridge(int gridOffset);

void TerrainBridge_updateSpriteIdsOnMapRotate(int ccw);

int TerrainBridge_getLength();
void TerrainBridge_resetLength();

#endif
