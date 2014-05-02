#ifndef TERRAINGRAPHICS_H
#define TERRAINGRAPHICS_H

void TerrainGraphics_updateAllRocks();
void TerrainGraphics_updateAllGardens();
void TerrainGraphics_updateAllRoads();
void TerrainGraphics_updateRegionElevation(int xMin, int yMin, int xMax, int yMax);
void TerrainGraphics_updateRegionPlazas(int xMin, int yMin, int xMax, int yMax);
void TerrainGraphics_updateRegionWater(int xMin, int yMin, int xMax, int yMax);
void TerrainGraphics_updateRegionEmptyLand(int xMin, int yMin, int xMax, int yMax);
void TerrainGraphics_updateRegionMeadow(int xMin, int yMin, int xMax, int yMax);
void TerrainGraphics_updateRegionEarthquake(int xMin, int yMin, int xMax, int yMax);

void TerrainGraphics_setBuildingAreaRubble(int buildingId, int x, int y, int size);

void TerrainGraphics_setTileWater(int x, int y);
void TerrainGraphics_setTileEarthquake(int x, int y);
void TerrainGraphics_setTileRoad(int x, int y);
void TerrainGraphics_setTileAqueduct(int x, int y, int flag);

enum {
	TerrainGraphicsContext_Water,
	TerrainGraphicsContext_Wall,
	TerrainGraphicsContext_WallGatehouse,
	TerrainGraphicsContext_Elevation,
	TerrainGraphicsContext_Earthquake,
	TerrainGraphicsContext_DirtRoad,
	TerrainGraphicsContext_PavedRoad,
	TerrainGraphicsContext_Aqueduct,
	TerrainGraphicsContext_NumItems
};

typedef struct TerrainGraphic {
	int isValid;
	int groupOffset;
	int itemOffset;
	int field12;
} TerrainGraphic;

void TerrainGraphicsContext_init();
const TerrainGraphic *TerrainGraphicsContext_getElevation(int gridOffset, int elevation);
const TerrainGraphic *TerrainGraphicsContext_getEarthquake(int gridOffset);

#endif
