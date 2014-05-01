#include "TerrainGraphics.h"

#include "Terrain.h"

#include "Data/Building.h"
#include "Data/Constants.h"
#include "Data/Graphics.h"
#include "Data/Grid.h"
#include "Data/Settings.h"

#include <stdio.h>

#define BOUND_REGION() \
	if (xMin < 0) xMin = 0;\
	if (yMin < 0) yMin = 0;\
	if (xMax >= Data_Settings_Map.width) xMax = Data_Settings_Map.width - 1;\
	if (yMax >= Data_Settings_Map.height) yMax = Data_Settings_Map.height - 1;

#define FOREACH_REGION(block) \
	int gridOffset = GridOffset(xMin, yMin);\
	for (int yy = yMin; yy <= yMax; yy++) {\
		for (int xx = xMin; xx <= xMax; xx++) {\
			block;\
			++gridOffset;\
		}\
		gridOffset += 162 - (xMax - xMin + 1);\
	}

static void TerrainGraphics_updateTileMeadow(int x, int y);
static void TerrainGraphics_updateAreaEmptyLand(int x, int y, int size, int graphicId);

static int isAllTerrainInArea(int x, int y, int size, int terrain)
{
	if (x < 0 || x + size > Data_Settings_Map.width ||
		y < 0 || y + size > Data_Settings_Map.height) {
		return 0;
	}
	for (int ty = y; ty < y + size; ty++) {
		for (int tx = x; tx < x + size; tx++) {
			int gridOffset = GridOffset(tx, ty);
			if ((Data_Grid_terrain[gridOffset] & Terrain_NotClear) != terrain) {
				return 0;
			}
			if (Data_Grid_graphicIds[gridOffset] != 0) {
				return 0;
			}
		}
	}
	return 1;
}

void TerrainGraphics_updateAllRocks()
{
	int gridOffset = Data_Settings_Map.gridStartOffset;
	for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
			int terrain = Data_Grid_terrain[gridOffset];
			if ((terrain & Terrain_Rock) && !(terrain & (Terrain_ReservoirRange | Terrain_Elevation | Terrain_AccessRamp))) {
				Data_Grid_graphicIds[gridOffset] = 0;
				Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
				Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
			}
		}
	}
	int graphicIdRock = GraphicId(ID_Graphic_TerrainRock);
	int graphicIdElevation = GraphicId(ID_Graphic_TerrainElevationRock);
	gridOffset = Data_Settings_Map.gridStartOffset;
	for (int yy = 0; yy < Data_Settings_Map.height; yy++, gridOffset += Data_Settings_Map.gridBorderSize) {
		for (int xx = 0; xx < Data_Settings_Map.width; xx++, gridOffset++) {
			int terrain = Data_Grid_terrain[gridOffset];
			if ((terrain & Terrain_Rock) && !(terrain & (Terrain_ReservoirRange | Terrain_Elevation | Terrain_AccessRamp))) {
				if (!Data_Grid_graphicIds[gridOffset]) {
					if (isAllTerrainInArea(xx, yy, 3, Terrain_Rock)) {
						int graphicId = 12 + (Data_Grid_random[gridOffset] & 1);
						if (Terrain_existsTileWithinRadiusWithType(xx, yy, 3, 4, Terrain_Elevation)) {
							graphicId += graphicIdElevation;
						} else {
							graphicId += graphicIdRock;
						}
						Terrain_addBuildingToGrids(0, xx, yy, 3, graphicId, Terrain_Rock);
					} else if (isAllTerrainInArea(xx, yy, 2, Terrain_Rock)) {
						int graphicId = 8 + (Data_Grid_random[gridOffset] & 3);
						if (Terrain_existsTileWithinRadiusWithType(xx, yy, 2, 4, Terrain_Elevation)) {
							graphicId += graphicIdElevation;
						} else {
							graphicId += graphicIdRock;
						}
						Terrain_addBuildingToGrids(0, xx, yy, 2, graphicId, Terrain_Rock);
					} else {
						int graphicId = Data_Grid_random[gridOffset] & 7;
						if (Terrain_existsTileWithinRadiusWithType(xx, yy, 1, 4, Terrain_Elevation)) {
							graphicId += graphicIdElevation;
						} else {
							graphicId += graphicIdRock;
						}
						Data_Grid_graphicIds[gridOffset] = graphicId;
					}
				}
			}
		}
	}
}

void TerrainGraphics_updateAllGardens()
{
	// TODO
}
void TerrainGraphics_updateAllRoads()
{
	// TODO
}

void TerrainGraphics_updateRegionPlazas(int xMin, int yMin, int xMax, int yMax)
{
	// TODO
}

void TerrainGraphics_updateRegionWater(int xMin, int yMin, int xMax, int yMax)
{
	BOUND_REGION();
	FOREACH_REGION({
		int terrain = Data_Grid_terrain[gridOffset];
		if ((terrain & Terrain_Water) && !(terrain & Terrain_Building)) {
			TerrainGraphics_setTileWater(xx, yy);
		}
	});
}

void TerrainGraphics_updateRegionEmptyLand(int xMin, int yMin, int xMax, int yMax)
{
	BOUND_REGION();
	{
		FOREACH_REGION({
			if (!(Data_Grid_terrain[gridOffset] & Terrain_NotClear)) {
				Data_Grid_graphicIds[gridOffset] = 0;
				Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
				Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
			}
		});
	}
	{
		FOREACH_REGION({
			if (!(Data_Grid_terrain[gridOffset] & Terrain_NotClear) &&
				!Data_Grid_graphicIds[gridOffset] &&
				!(Data_Grid_random[gridOffset] & 0xf0)) {
				int graphicId;
				if (Data_Grid_bitfields[gridOffset] & Bitfield_AlternateTerrain) {
					graphicId = GraphicId(ID_Graphic_TerrainGrass2);
				} else {
					graphicId = GraphicId(ID_Graphic_TerrainGrass1);
				}
				TerrainGraphics_updateAreaEmptyLand(xx, yy, 1,
					(Data_Grid_random[gridOffset] & 7) + graphicId);
			}
		});
	}
	{
		FOREACH_REGION({
			if (!(Data_Grid_terrain[gridOffset] & Terrain_NotClear) &&
				!Data_Grid_graphicIds[gridOffset]) {
				int graphicId;
				if (Data_Grid_bitfields[gridOffset] & Bitfield_AlternateTerrain) {
					graphicId = GraphicId(ID_Graphic_TerrainGrass2);
				} else {
					graphicId = GraphicId(ID_Graphic_TerrainGrass1);
				}
				if (Terrain_isClear(xx, yy, 4, Terrain_All, 1)) {
					TerrainGraphics_updateAreaEmptyLand(xx, yy, 4, graphicId + 42);
				} else if (Terrain_isClear(xx, yy, 3, Terrain_All, 1)) {
					TerrainGraphics_updateAreaEmptyLand(xx, yy, 3,
						graphicId + 24 + 9 * (Data_Grid_random[gridOffset] & 1));
				} else if (Terrain_isClear(xx, yy, 2, Terrain_All, 1)) {
					TerrainGraphics_updateAreaEmptyLand(xx, yy, 2,
						graphicId + 8 + 4 * (Data_Grid_random[gridOffset] & 3));
				} else {
					TerrainGraphics_updateAreaEmptyLand(xx, yy, 1,
						graphicId + (Data_Grid_random[gridOffset] & 7));
				}
			}
		});
	}
}

void TerrainGraphics_updateRegionMeadow(int xMin, int yMin, int xMax, int yMax)
{
	int forbiddenTerrain = Terrain_Aqueduct | Terrain_Elevation | Terrain_AccessRamp;
	forbiddenTerrain |= Terrain_Rubble | Terrain_Road | Terrain_Building | Terrain_Garden;
	BOUND_REGION();
	FOREACH_REGION({
		int terrain = Data_Grid_terrain[gridOffset];
		if ((terrain & Terrain_Meadow) && !(terrain & forbiddenTerrain)) {
			TerrainGraphics_updateTileMeadow(xx, yy);
		}
	});
}

void TerrainGraphics_setBuildingAreaRubble(int buildingId, int x, int y, int size)
{
	if (x < 0 || x + size > Data_Settings_Map.width ||
		y < 0 || y + size > Data_Settings_Map.height) {
		return;
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = GridOffset(x + dx, y + dy);
			if (Data_Grid_buildingIds[gridOffset] != buildingId) {
				continue;
			}
			if (buildingId && Data_Buildings[Data_Grid_buildingIds[gridOffset]].type != Building_BurningRuin) {
				Data_Grid_rubbleBuildingType[gridOffset] = (unsigned char) Data_Buildings[buildingId].type;
			}
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
			Data_Grid_aqueducts[gridOffset] = 0;
			Data_Grid_buildingIds[gridOffset] = 0;
			Data_Grid_buildingDamage[gridOffset] = 0;
			Data_Grid_spriteOffsets[gridOffset] = 0;
			Data_Grid_edge[gridOffset] = Edge_LeftmostTile;
			if (Data_Grid_terrain[gridOffset] & Terrain_Water) {
				Data_Grid_terrain[gridOffset] &= Terrain_Water;
				TerrainGraphics_setTileWater(x + dx, y + dy);
			} else {
				Data_Grid_terrain[gridOffset] &= Terrain_2e80;
				Data_Grid_terrain[gridOffset] |= Terrain_Rubble;
				Data_Grid_graphicIds[gridOffset] =
					GraphicId(ID_Graphic_TerrainRubble) + (Data_Grid_random[gridOffset] & 7);
			}
		}
	}
}

void TerrainGraphics_setTileWater(int x, int y)
{
	// TODO
}

void TerrainGraphics_setTileEarthquake(int x, int y)
{
	// TODO
}

static void TerrainGraphics_updateTileMeadow(int x, int y)
{
	int forbiddenTerrain = Terrain_Aqueduct | Terrain_Elevation | Terrain_AccessRamp;
	forbiddenTerrain |= Terrain_Rubble | Terrain_Road | Terrain_Building | Terrain_Garden;

	int xMin = x - 1;
	int yMin = y - 1;
	int xMax = xMin + 3;
	int yMax = yMin + 3;
	int graphicId = GraphicId(ID_Graphic_TerrainMeadow);
	BOUND_REGION();
	FOREACH_REGION({
		int terrain = Data_Grid_terrain[gridOffset];
		if ((terrain & Terrain_Meadow) && !(terrain & forbiddenTerrain)) {
			int random = Data_Grid_random[gridOffset] & 3;
			if (Terrain_isAllMeadowAtDistanceRing(xx, yy, 2)) {
				Data_Grid_graphicIds[gridOffset] = graphicId + random + 8;
			} else if (Terrain_isAllMeadowAtDistanceRing(xx, yy, 1)) {
				Data_Grid_graphicIds[gridOffset] = graphicId + random + 4;
			} else {
				Data_Grid_graphicIds[gridOffset] = graphicId + random;
			}
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
			Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
			Data_Grid_aqueducts[gridOffset] = 0;
		}
	});
}

static void TerrainGraphics_updateAreaEmptyLand(int x, int y, int size, int graphicId)
{
	if (x < 0 || x + size > Data_Settings_Map.width ||
		y < 0 || y + size > Data_Settings_Map.height) {
		return;
	}
	int index = 0;
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = GridOffset(x + dx, y + dy);
			Data_Grid_terrain[gridOffset] &= Terrain_2e80;
			Data_Grid_buildingIds[gridOffset] = 0;
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
			Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
			Data_Grid_graphicIds[gridOffset] = graphicId + index;
			index++;
		}
	}
}
