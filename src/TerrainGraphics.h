#ifndef TERRAINGRAPHICS_H
#define TERRAINGRAPHICS_H

#include "building/building.h"

void TerrainGraphics_updateAllRocks();
void TerrainGraphics_updateAllGardens();
void TerrainGraphics_determineGardensFromGraphicIds();
void TerrainGraphics_updateAllRoads();
void TerrainGraphics_updateAllWalls();
void TerrainGraphics_updateRegionElevation(int xMin, int yMin, int xMax, int yMax);
void TerrainGraphics_updateRegionPlazas(int xMin, int yMin, int xMax, int yMax);
void TerrainGraphics_updateRegionWater(int xMin, int yMin, int xMax, int yMax);
void TerrainGraphics_updateRegionAqueduct(int xMin, int yMin, int xMax, int yMax, int includeOverlay);
void TerrainGraphics_updateRegionEmptyLand(int xMin, int yMin, int xMax, int yMax);
void TerrainGraphics_updateRegionMeadow(int xMin, int yMin, int xMax, int yMax);
void TerrainGraphics_updateRegionEarthquake(int xMin, int yMin, int xMax, int yMax);
void TerrainGraphics_updateRegionRubble(int xMin, int yMin, int xMax, int yMax);

void TerrainGraphics_updateAreaWalls(int x, int y, int size);
void TerrainGraphics_updateAreaRoads(int x, int y, int size);

void TerrainGraphics_setTileWater(int x, int y);
void TerrainGraphics_setTileEarthquake(int x, int y);
int TerrainGraphics_setTileRoad(int x, int y);
int TerrainGraphics_setTileWall(int x, int y);
int TerrainGraphics_setTileAqueduct(int x, int y, int forceNoWater);
int TerrainGraphics_setTileAqueductTerrain(int x, int y);

int TerrainGraphics_isPavedRoadTile(int gridOffset);

#endif
