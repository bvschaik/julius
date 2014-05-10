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
	{int gridOffset = GridOffset(xMin, yMin);\
	for (int yy = yMin; yy <= yMax; yy++) {\
		for (int xx = xMin; xx <= xMax; xx++) {\
			block;\
			++gridOffset;\
		}\
		gridOffset += 162 - (xMax - xMin + 1);\
	}}

static void TerrainGraphics_setTileRubble(int x, int y);
static void TerrainGraphics_updateTileMeadow(int x, int y);
static void TerrainGraphics_updateAreaEmptyLand(int x, int y, int size, int graphicId);

static int isAllTerrainInArea(int x, int y, int size, int terrain)
{
	if (IsOutsideMap(x, y, size)) {
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

static int getAccessRampGraphicOffset(int x, int y)
{
	if (IsOutsideMap(x, y, 1)) {
		return -1;
	}
	// TODO
	static const int offsets[4][6] = {
		{162, 163, 0, 1, 324, 325},
		{0, 162, 1, 163, -1, 161},
		{0, 1, 162, 163, -162, -161},
		{1, 163, 0, 162, 2, 164},
	};
	int baseOffset = GridOffset(x, y);
	int graphicOffset = -1;
	for (int dir = 0; dir < 4; dir++) {
		int rightTiles = 0;
		int height = -1;
		for (int i = 0; i < 6; i++) {
			int gridOffset = baseOffset + offsets[dir][i];
			if (i < 2) { // 2nd row
				if (Data_Grid_terrain[gridOffset] & Terrain_Elevation) {
					rightTiles++;
				}
				height = Data_Grid_elevation[gridOffset];
			} else if (i < 4) { // 1st row
				if ((Data_Grid_terrain[gridOffset] & Terrain_AccessRamp) &&
					Data_Grid_elevation[gridOffset] < height) {
					rightTiles++;
				}
			} else { // higher row beyond access ramp
				if (Data_Grid_terrain[gridOffset] & Terrain_Elevation) {
					if (Data_Grid_elevation[gridOffset] != height) {
						rightTiles++;
					}
				} else if (Data_Grid_elevation[gridOffset] >= height) {
					rightTiles++;
				}
			}
		}
		if (rightTiles == 6) {
			graphicOffset = dir;
			break;
		}
	}
	if (graphicOffset < 0) {
		return -1;
	}
	switch (Data_Settings_Map.orientation) {
		case Direction_Top: break;
		case Direction_Left: graphicOffset += 1; break;
		case Direction_Bottom: graphicOffset += 2; break;
		case Direction_Right: graphicOffset += 3; break;
	}
	if (graphicOffset >= 4) {
		graphicOffset -= 4;
	}
	return graphicOffset;
}

void TerrainGraphics_updateRegionElevation(int xMin, int yMin, int xMax, int yMax)
{
	BOUND_REGION();
	FOREACH_REGION({
		if (Data_Grid_terrain[gridOffset] & Terrain_AccessRamp) {
			Data_Grid_graphicIds[gridOffset] = 0;
		}
	});
	FOREACH_REGION({
		if ((Data_Grid_terrain[gridOffset] & Terrain_AccessRamp) &&
			!Data_Grid_graphicIds[gridOffset]) {
			int graphicOffset = getAccessRampGraphicOffset(xx, yy);
			if (graphicOffset < 0) {
				// invalid map: remove access ramp
				Data_Grid_terrain[gridOffset] &= ~Terrain_AccessRamp;
				Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
				Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
				if (Data_Grid_elevation[gridOffset]) {
					Data_Grid_terrain[gridOffset] |= Terrain_Elevation;
				} else {
					Data_Grid_terrain[gridOffset] &= ~Terrain_Elevation;
					Data_Grid_graphicIds[gridOffset] =
						GraphicId(ID_Graphic_TerrainGrass1) + (Data_Grid_random[gridOffset] & 7);
				}
			} else {
				Terrain_addBuildingToGrids(0, xx, yy, 2,
					graphicOffset + GraphicId(ID_Graphic_AccessRamp), Terrain_AccessRamp);
			}
		}
		if (Data_Grid_elevation[gridOffset] && !(Data_Grid_terrain[gridOffset] & Terrain_AccessRamp)) {
			const TerrainGraphic *g = TerrainGraphicsContext_getElevation(
				gridOffset, Data_Grid_elevation[gridOffset]);
			if (g->groupOffset == 44) {
				Data_Grid_terrain[gridOffset] &= ~Terrain_Elevation;
				int terrain = Data_Grid_terrain[gridOffset];
				if (!(terrain & Terrain_Building)) {
					Data_Grid_edge[gridOffset] = Edge_LeftmostTile;
					if (terrain & Terrain_Scrub) {
						Data_Grid_graphicIds[gridOffset] =
							GraphicId(ID_Graphic_TerrainShrub) + (Data_Grid_random[gridOffset] & 7);
					} else if (terrain & Terrain_Tree) {
						Data_Grid_graphicIds[gridOffset] =
							GraphicId(ID_Graphic_TerrainTree) + (Data_Grid_random[gridOffset] & 7);
					} else if (terrain & Terrain_Road) {
						TerrainGraphics_setTileRoad(xx, yy);
					} else if (terrain & Terrain_Aqueduct) {
						TerrainGraphics_setTileAqueduct(xx, yy, 0);
					} else if (terrain & Terrain_Meadow) {
						Data_Grid_graphicIds[gridOffset] =
							GraphicId(ID_Graphic_TerrainMeadow) + (Data_Grid_random[gridOffset] & 3);
					} else {
						Data_Grid_graphicIds[gridOffset] =
							GraphicId(ID_Graphic_TerrainGrass1) + (Data_Grid_random[gridOffset] & 7);
					}
				}
			} else {
				Data_Grid_edge[gridOffset] = Edge_LeftmostTile;
				Data_Grid_terrain[gridOffset] |= Terrain_Elevation;
				Data_Grid_graphicIds[gridOffset] =
					GraphicId(ID_Graphic_TerrainElevation) +
					g->groupOffset + g->itemOffset;
			}
		}
	});
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
	FOREACH_REGION({
		if (!(Data_Grid_terrain[gridOffset] & Terrain_NotClear)) {
			Data_Grid_graphicIds[gridOffset] = 0;
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
			Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
		}
	});
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

void TerrainGraphics_updateRegionEarthquake(int xMin, int yMin, int xMax, int yMax)
{
	BOUND_REGION();
	FOREACH_REGION({
		if ((Data_Grid_terrain[gridOffset] & Terrain_Rock) &&
			(Data_Grid_bitfields[gridOffset] & Bitfield_Plaza)) {
			TerrainGraphics_setTileEarthquake(xx, yy);
		}
	});
}

void TerrainGraphics_updateRegionRubble(int xMin, int yMin, int xMax, int yMax)
{
	// TODO
	int forbiddenTerrain = Terrain_Aqueduct | Terrain_Elevation | Terrain_AccessRamp;
	forbiddenTerrain |= Terrain_Road | Terrain_Building | Terrain_Garden;
	BOUND_REGION();
	FOREACH_REGION({
		int terrain = Data_Grid_terrain[gridOffset];
		if ((terrain & Terrain_Rubble) && !(terrain & forbiddenTerrain)) {
			TerrainGraphics_setTileRubble(xx, yy);
		}
	});
}

void TerrainGraphics_setBuildingAreaRubble(int buildingId, int x, int y, int size)
{
	if (IsOutsideMap(x, y, size)) {
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

void TerrainGraphics_setBuildingFarm(int buildingId, int x, int y, int cropGraphicId, int progress)
{
	if (IsOutsideMap(x, y, 3)) {
		return;
	}
	// farmhouse
	int leftmostX, leftmostY;
	switch (Data_Settings_Map.orientation) {
		case Direction_Top: leftmostX = 0; leftmostY = 1; break;
		case Direction_Right: leftmostX = 0; leftmostY = 0; break;
		case Direction_Bottom: leftmostX = 1; leftmostY = 0; break;
		case Direction_Left: leftmostX = 1; leftmostY = 1; break;
	}
	for (int dy = 0; dy < 2; dy++) {
		for (int dx = 0; dx < 2; dx++) {
			int gridOffset = GridOffset(x + dx, y + dy);
			Data_Grid_terrain[gridOffset] &= Terrain_2e80;
			Data_Grid_terrain[gridOffset] |= Terrain_Building;
			Data_Grid_buildingIds[gridOffset] = buildingId;
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;
			Data_Grid_bitfields[gridOffset] |= Bitfield_Size2;
			Data_Grid_graphicIds[gridOffset] = GraphicId(ID_Graphic_FarmHouse);
			Data_Grid_edge[gridOffset] = EdgeXY(dx, dy);
			if (dx == leftmostX && dy == leftmostY) {
				Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
			}
		}
	}
	// crop tile 1
	int growth = progress / 10;
	int gridOffset = GridOffset(x, y+2);
	Data_Grid_terrain[gridOffset] &= Terrain_2e80;
	Data_Grid_terrain[gridOffset] |= Terrain_Building;
	Data_Grid_buildingIds[gridOffset] = buildingId;
	Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;
	Data_Grid_edge[gridOffset] = EdgeXY(0, 2) | Edge_LeftmostTile;
	Data_Grid_graphicIds[gridOffset] = cropGraphicId + (growth < 4 ? growth : 4);

	// crop tile 2
	growth -= 4;
	if (growth < 0) {
		growth = 0;
	}
	gridOffset = GridOffset(x+1, y+2);
	Data_Grid_terrain[gridOffset] &= Terrain_2e80;
	Data_Grid_terrain[gridOffset] |= Terrain_Building;
	Data_Grid_buildingIds[gridOffset] = buildingId;
	Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;
	Data_Grid_edge[gridOffset] = EdgeXY(1, 2) | Edge_LeftmostTile;
	Data_Grid_graphicIds[gridOffset] = cropGraphicId + (growth < 4 ? growth : 4);
	
	// crop tile 3
	growth -= 4;
	if (growth < 0) {
		growth = 0;
	}
	gridOffset = GridOffset(x+2, y+2);
	Data_Grid_terrain[gridOffset] &= Terrain_2e80;
	Data_Grid_terrain[gridOffset] |= Terrain_Building;
	Data_Grid_buildingIds[gridOffset] = buildingId;
	Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;
	Data_Grid_edge[gridOffset] = EdgeXY(2, 2) | Edge_LeftmostTile;
	Data_Grid_graphicIds[gridOffset] = cropGraphicId + (growth < 4 ? growth : 4);

	// crop tile 4
	growth -= 4;
	if (growth < 0) {
		growth = 0;
	}
	gridOffset = GridOffset(x+2, y+1);
	Data_Grid_terrain[gridOffset] &= Terrain_2e80;
	Data_Grid_terrain[gridOffset] |= Terrain_Building;
	Data_Grid_buildingIds[gridOffset] = buildingId;
	Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;
	Data_Grid_edge[gridOffset] = EdgeXY(2, 1) | Edge_LeftmostTile;
	Data_Grid_graphicIds[gridOffset] = cropGraphicId + (growth < 4 ? growth : 4);

	// crop tile 5
	growth -= 4;
	if (growth < 0) {
		growth = 0;
	}
	gridOffset = GridOffset(x+2, y);
	Data_Grid_terrain[gridOffset] &= Terrain_2e80;
	Data_Grid_terrain[gridOffset] |= Terrain_Building;
	Data_Grid_buildingIds[gridOffset] = buildingId;
	Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;
	Data_Grid_edge[gridOffset] = EdgeXY(2, 0) | Edge_LeftmostTile;
	Data_Grid_graphicIds[gridOffset] = cropGraphicId + (growth < 4 ? growth : 4);
}

void TerrainGraphics_setTileWater(int x, int y)
{
	Data_Grid_terrain[GridOffset(x, y)] |= Terrain_Water;
	int xMin = x - 1;
	int xMax = xMin + 3;
	int yMin = y - 1;
	int yMax = yMin + 3;
	BOUND_REGION();
	FOREACH_REGION({
		if ((Data_Grid_terrain[gridOffset] & (Terrain_Water | Terrain_Building)) == Terrain_Water) {
			const TerrainGraphic *g = TerrainGraphicsContext_getShore(gridOffset);
			int graphicId = GraphicId(ID_Graphic_TerrainWater) + g->groupOffset + g->itemOffset;
			if (Terrain_existsTileWithinRadiusWithType(xx, yy, 1, 2, Terrain_Building)) {
				// fortified shore
				graphicId = GraphicId(ID_Graphic_TerrainWaterShore);
				switch (g->groupOffset) {
					case 8: graphicId += 10; break;
					case 12: graphicId += 11; break;
					case 16: graphicId += 9; break;
					case 20: graphicId += 8; break;
					case 24: graphicId += 18; break;
					case 28: graphicId += 16; break;
					case 32: graphicId += 19; break;
					case 36: graphicId += 17; break;
					case 50: graphicId += 12; break;
					case 51: graphicId += 14; break;
					case 52: graphicId += 13; break;
					case 53: graphicId += 15; break;
					default:
						graphicId = GraphicId(ID_Graphic_TerrainWater) + g->groupOffset + g->itemOffset;
						break;
				}
			}
			Data_Grid_graphicIds[gridOffset] = graphicId;
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
			Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
		}
	});
}

void TerrainGraphics_setTileEarthquake(int x, int y)
{
	// earthquake: terrain = rock && bitfields = plaza
	int gridOffset = GridOffset(x, y);
	Data_Grid_terrain[gridOffset] |= Terrain_Rock;
	Data_Grid_bitfields[gridOffset] |= Bitfield_Plaza;

	int xMin = x - 1;
	int yMin = y - 1;
	int xMax = xMin + 3;
	int yMax = yMin + 3;
	BOUND_REGION();
	FOREACH_REGION({
		if ((Data_Grid_terrain[gridOffset] & Terrain_Rock) &&
			(Data_Grid_bitfields[gridOffset] & Bitfield_Plaza)) {
			const TerrainGraphic *g = TerrainGraphicsContext_getEarthquake(gridOffset);
			if (g->isValid) {
				Data_Grid_graphicIds[gridOffset] =
					GraphicId(ID_Graphic_TerrainEarthquake) +
					g->groupOffset + g->itemOffset;
			} else {
				Data_Grid_graphicIds[gridOffset] = GraphicId(ID_Graphic_TerrainEarthquake);
			}
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
			Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
		}
	});
}

void TerrainGraphics_setTileRoad(int x, int y)
{
	// TODO
}

void TerrainGraphics_setTileAqueduct(int x, int y, int flag)
{
	// TODO
}

static void TerrainGraphics_setTileRubble(int x, int y)
{
	int gridOffset = GridOffset(x, y);
	Data_Grid_graphicIds[gridOffset] =
		GraphicId(ID_Graphic_TerrainRubble) + (Data_Grid_random[gridOffset] & 7);
	Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
	Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
	Data_Grid_aqueducts[gridOffset] = 0;
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
	if (IsOutsideMap(x, y, size)) {
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
