#ifndef TERRAINGRAPHICS_H
#define TERRAINGRAPHICS_H

void TerrainGraphics_updateAllRocks();
void TerrainGraphics_updateAllGardens();
void TerrainGraphics_updateAllRoads();
void TerrainGraphics_updateRegionPlazas(int xMin, int yMin, int xMax, int yMax);
void TerrainGraphics_updateRegionWater(int xMin, int yMin, int xMax, int yMax);
void TerrainGraphics_updateRegionEmptyLand(int xMin, int yMin, int xMax, int yMax);
void TerrainGraphics_updateRegionMeadow(int xMin, int yMin, int xMax, int yMax);

void TerrainGraphics_setBuildingAreaRubble(int buildingId, int x, int y, int size);

void TerrainGraphics_setTileWater(int x, int y);
void TerrainGraphics_setTileEarthquake(int x, int y);

#endif
