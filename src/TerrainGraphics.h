#ifndef TERRAINGRAPHICS_H
#define TERRAINGRAPHICS_H

#include "building/building.h"

void TerrainGraphics_determineGardensFromGraphicIds();
void TerrainGraphics_updateRegionElevation(int xMin, int yMin, int xMax, int yMax);
void TerrainGraphics_updateRegionAqueduct(int xMin, int yMin, int xMax, int yMax, int includeConstruction);
void TerrainGraphics_updateAllEarthquake();
void TerrainGraphics_updateRegionRubble(int xMin, int yMin, int xMax, int yMax);

void TerrainGraphics_setTileEarthquake(int x, int y);
int TerrainGraphics_setTileAqueduct(int x, int y, int forceNoWater);
int TerrainGraphics_setTileAqueductTerrain(int x, int y);

#endif
