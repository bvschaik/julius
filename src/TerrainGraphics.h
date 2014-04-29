#ifndef TERRAINGRAPHICS_H
#define TERRAINGRAPHICS_H

void TerrainGraphics_updateAllRocks();
void TerrainGraphics_updateRegionWater(int xMin, int yMin, int xMax, int yMax);
void TerrainGraphics_updateRegionMeadow(int xMin, int yMin, int xMax, int yMax);

void TerrainGraphics_updateTileWater(int x, int y, int isSet);

#endif
