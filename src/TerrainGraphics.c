#include "TerrainGraphics.h"

#include "Terrain.h"
#include "Terrain_private.h"

#include "Data/Building.h"
#include "Data/Constants.h"

#include "graphics/image.h"

static void TerrainGraphics_setTileRubble(int x, int y);
static void TerrainGraphics_updateTileMeadow(int x, int y);
static void TerrainGraphics_updateAreaEmptyLand(int x, int y, int size, int graphicId);
static void setWallGraphic(int gridOffset);
static void setRoadGraphic(int gridOffset);

static int isAllTerrainInArea(int x, int y, int size, int terrain)
{
	if (IsOutsideMap(x, y, size)) {
		return 0;
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = GridOffset(x + dx, y + dy);
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

int TerrainGraphics_isPavedRoadTile(int gridOffset)
{
	if (Data_Grid_desirability[gridOffset] > 4) {
		return 1;
	}
	if (Data_Grid_desirability[gridOffset] > 0 && Data_Grid_terrain[gridOffset] & Terrain_FountainRange) {
		return 1;
	}
	return 0;
}

static void setRoadWithAqueductGraphic(int gridOffset)
{
	int graphicIdAqueduct = image_group(ID_Graphic_Aqueduct);
	int waterOffset;
	if (Data_Grid_graphicIds[gridOffset] < graphicIdAqueduct + 15) {
		waterOffset = 0;
	} else {
		waterOffset = 15;
	}
	const struct TerrainGraphic *graphic = TerrainGraphicsContext_getAqueduct(gridOffset, 0);
	int groupOffset = graphic->groupOffset;
	if (!graphic->aqueductOffset) {
		if (Data_Grid_terrain[gridOffset + DELTA(0, -1)] & Terrain_Road) {
			groupOffset = 3;
		} else {
			groupOffset = 2;
		}
	}
	if (TerrainGraphics_isPavedRoadTile(gridOffset)) {
		Data_Grid_graphicIds[gridOffset] =
			graphicIdAqueduct + waterOffset + groupOffset - 2;
	} else {
		Data_Grid_graphicIds[gridOffset] =
			graphicIdAqueduct + waterOffset + groupOffset + 6;
	}
	Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
	Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
}

static void setRoadGraphic(int gridOffset)
{
	if (Data_Grid_bitfields[gridOffset] & Bitfield_PlazaOrEarthquake) {
		return;
	}
	if (Data_Grid_terrain[gridOffset] & Terrain_Aqueduct) {
		setRoadWithAqueductGraphic(gridOffset);
		return;
	}
	if (TerrainGraphics_isPavedRoadTile(gridOffset)) {
		const struct TerrainGraphic *graphic = TerrainGraphicsContext_getPavedRoad(gridOffset);
		Data_Grid_graphicIds[gridOffset] = image_group(ID_Graphic_Road) +
			graphic->groupOffset + graphic->itemOffset;
	} else {
		const struct TerrainGraphic *graphic = TerrainGraphicsContext_getDirtRoad(gridOffset);
		Data_Grid_graphicIds[gridOffset] = image_group(ID_Graphic_Road) +
			graphic->groupOffset + graphic->itemOffset + 49;
	}
	Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
	Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
}

static void setTileAqueduct(int gridOffset, int waterOffset, int includeOverlay)
{
	const struct TerrainGraphic *graphic = TerrainGraphicsContext_getAqueduct(gridOffset, includeOverlay);
	int groupOffset = graphic->groupOffset;
	if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
		Data_Grid_bitfields[gridOffset] &= Bitfield_NoPlaza;
		if (!graphic->aqueductOffset) {
			if (Data_Grid_terrain[gridOffset + DELTA(0, -1)] & Terrain_Road) {
				groupOffset = 3;
			} else {
				groupOffset = 2;
			}
		}
		if (TerrainGraphics_isPavedRoadTile(gridOffset)) {
			groupOffset -= 2;
		} else {
			groupOffset += 6;
		}
	}
	Data_Grid_graphicIds[gridOffset] = image_group(ID_Graphic_Aqueduct) +
		waterOffset + groupOffset + graphic->itemOffset;
	Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
	Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
	Data_Grid_aqueducts[gridOffset] = graphic->aqueductOffset;
}

void TerrainGraphics_updateAllRocks()
{
	FOREACH_ALL({
		int terrain = Data_Grid_terrain[gridOffset];
		if ((terrain & Terrain_Rock) && !(terrain & (Terrain_ReservoirRange | Terrain_Elevation | Terrain_AccessRamp))) {
			Data_Grid_graphicIds[gridOffset] = 0;
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
			Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
		}
	});
	int graphicIdRock = image_group(ID_Graphic_TerrainRock);
	int graphicIdElevation = image_group(ID_Graphic_TerrainElevationRock);
	FOREACH_ALL({
		int terrain = Data_Grid_terrain[gridOffset];
		if ((terrain & Terrain_Rock) && !(terrain & (Terrain_ReservoirRange | Terrain_Elevation | Terrain_AccessRamp))) {
			if (!Data_Grid_graphicIds[gridOffset]) {
				if (isAllTerrainInArea(x, y, 3, Terrain_Rock)) {
					int graphicId = 12 + (Data_Grid_random[gridOffset] & 1);
					if (Terrain_existsTileWithinRadiusWithType(x, y, 3, 4, Terrain_Elevation)) {
						graphicId += graphicIdElevation;
					} else {
						graphicId += graphicIdRock;
					}
					Terrain_addBuildingToGrids(0, x, y, 3, graphicId, Terrain_Rock);
				} else if (isAllTerrainInArea(x, y, 2, Terrain_Rock)) {
					int graphicId = 8 + (Data_Grid_random[gridOffset] & 3);
					if (Terrain_existsTileWithinRadiusWithType(x, y, 2, 4, Terrain_Elevation)) {
						graphicId += graphicIdElevation;
					} else {
						graphicId += graphicIdRock;
					}
					Terrain_addBuildingToGrids(0, x, y, 2, graphicId, Terrain_Rock);
				} else {
					int graphicId = Data_Grid_random[gridOffset] & 7;
					if (Terrain_existsTileWithinRadiusWithType(x, y, 1, 4, Terrain_Elevation)) {
						graphicId += graphicIdElevation;
					} else {
						graphicId += graphicIdRock;
					}
					Data_Grid_graphicIds[gridOffset] = graphicId;
				}
			}
		}
	});
}

void TerrainGraphics_updateAllGardens()
{
	FOREACH_ALL({
		int terrain = Data_Grid_terrain[gridOffset];
		if (terrain & Terrain_Garden && !(terrain & (Terrain_Elevation | Terrain_AccessRamp))) {
			Data_Grid_graphicIds[gridOffset] = 0;
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
			Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
		}
	});
	FOREACH_ALL({
		int terrain = Data_Grid_terrain[gridOffset];
		if (terrain & Terrain_Garden && !(terrain & (Terrain_Elevation | Terrain_AccessRamp))) {
			if (!Data_Grid_graphicIds[gridOffset]) {
				int graphicId = image_group(ID_Graphic_Garden);
				if (isAllTerrainInArea(x, y, 2, Terrain_Garden)) {
					switch (Data_Grid_random[gridOffset] & 3) {
						case 0: case 1: graphicId += 6; break;
						case 2: graphicId += 5; break;
						case 3: graphicId += 4; break;
					}
					Terrain_addBuildingToGrids(0, x, y, 2, graphicId, Terrain_Garden);
				} else {
					if (y & 1) {
						switch (x & 3) {
							case 0: case 2: graphicId += 2; break;
							case 1: case 3: graphicId += 3; break;
						}
					} else {
						switch (x & 3) {
							case 1: case 3: graphicId += 1; break;
						}
					}
					Data_Grid_graphicIds[gridOffset] = graphicId;
				}
			}
		}
	});
}

void TerrainGraphics_determineGardensFromGraphicIds()
{
	int baseGraphicId = image_group(ID_Graphic_Garden);
	FOREACH_ALL({
		int graphicId = Data_Grid_graphicIds[gridOffset];
		if (graphicId >= baseGraphicId && graphicId <= baseGraphicId + 6) {
			Data_Grid_terrain[gridOffset] |= Terrain_Garden;
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;
			Data_Grid_aqueducts[gridOffset] = 0;
		}
	});
}

void TerrainGraphics_updateAllRoads()
{
	FOREACH_ALL({
		if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
			if (!(Data_Grid_terrain[gridOffset] & (Terrain_Water | Terrain_Building))) {
				setRoadGraphic(gridOffset);
			}
		}
	});
}

void TerrainGraphics_updateAllWalls()
{
	FOREACH_ALL({
		setWallGraphic(gridOffset);
	});
}

static int getAccessRampGraphicOffset(int x, int y)
{
	if (IsOutsideMap(x, y, 1)) {
		return -1;
	}
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
		case Dir_0_Top: break;
		case Dir_6_Left: graphicOffset += 1; break;
		case Dir_4_Bottom: graphicOffset += 2; break;
		case Dir_2_Right: graphicOffset += 3; break;
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
						image_group(ID_Graphic_TerrainGrass1) + (Data_Grid_random[gridOffset] & 7);
				}
			} else {
				Terrain_addBuildingToGrids(0, xx, yy, 2,
					image_group(ID_Graphic_AccessRamp) + graphicOffset, Terrain_AccessRamp);
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
							image_group(ID_Graphic_TerrainShrub) + (Data_Grid_random[gridOffset] & 7);
					} else if (terrain & Terrain_Tree) {
						Data_Grid_graphicIds[gridOffset] =
							image_group(ID_Graphic_TerrainTree) + (Data_Grid_random[gridOffset] & 7);
					} else if (terrain & Terrain_Road) {
						TerrainGraphics_setTileRoad(xx, yy);
					} else if (terrain & Terrain_Aqueduct) {
						TerrainGraphics_setTileAqueduct(xx, yy, 0);
					} else if (terrain & Terrain_Meadow) {
						Data_Grid_graphicIds[gridOffset] =
							image_group(ID_Graphic_TerrainMeadow) + (Data_Grid_random[gridOffset] & 3);
					} else {
						Data_Grid_graphicIds[gridOffset] =
							image_group(ID_Graphic_TerrainGrass1) + (Data_Grid_random[gridOffset] & 7);
					}
				}
			} else {
				Data_Grid_edge[gridOffset] = Edge_LeftmostTile;
				Data_Grid_terrain[gridOffset] |= Terrain_Elevation;
				Data_Grid_graphicIds[gridOffset] =
					image_group(ID_Graphic_TerrainElevation) + g->groupOffset + g->itemOffset;
			}
		}
	});
}

static int isTilePlaza(int gridOffset)
{
	if ((Data_Grid_terrain[gridOffset] & Terrain_Road) &&
		(Data_Grid_bitfields[gridOffset] & Bitfield_PlazaOrEarthquake) &&
		!(Data_Grid_terrain[gridOffset] & (Terrain_Water | Terrain_Building)) &&
		!Data_Grid_graphicIds[gridOffset]) {
		return 1;
	}
	return 0;
}
static int isTwoTileSquarePlaza(int gridOffset)
{
	return
		isTilePlaza(gridOffset + DELTA(1, 0)) &&
		isTilePlaza(gridOffset + DELTA(0, 1)) &&
		isTilePlaza(gridOffset + DELTA(1, 1));
}

void TerrainGraphics_updateRegionPlazas(int xMin, int yMin, int xMax, int yMax)
{
	BOUND_REGION();
	// remove plazas below buildings
	FOREACH_ALL({
		if (Data_Grid_terrain[gridOffset] & Terrain_Road &&
			Data_Grid_bitfields[gridOffset] & Bitfield_PlazaOrEarthquake) {
			if (Data_Grid_terrain[gridOffset] & Terrain_Building) {
				Data_Grid_bitfields[gridOffset] &= Bitfield_NoPlaza;
			}
		}
	});
	// convert plazas to single tile and remove graphic ids
	FOREACH_ALL({
		if (Data_Grid_terrain[gridOffset] & Terrain_Road &&
			Data_Grid_bitfields[gridOffset] & Bitfield_PlazaOrEarthquake) {
			Data_Grid_graphicIds[gridOffset] = 0;
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
			Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
		}
	});
	FOREACH_REGION({
		if (Data_Grid_terrain[gridOffset] & Terrain_Road &&
			Data_Grid_bitfields[gridOffset] & Bitfield_PlazaOrEarthquake &&
			!Data_Grid_graphicIds[gridOffset]) {
			int graphicId = image_group(ID_Graphic_Plaza);
			if (isTwoTileSquarePlaza(gridOffset)) {
				if (Data_Grid_random[gridOffset] & 1) {
					graphicId += 7;
				} else {
					graphicId += 6;
				}
				Terrain_addBuildingToGrids(0, xx, yy, 2, graphicId, Terrain_Road);
			} else {
				// single tile plaza
				switch ((xx & 1) + (yy & 1)) {
					case 2: graphicId += 1; break;
					case 1: graphicId += 2; break;
				}
				Data_Grid_graphicIds[gridOffset] = graphicId;
			}
		}
	});
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

void TerrainGraphics_updateRegionAqueduct(int xMin, int yMin, int xMax, int yMax, int includeOverlay)
{
	xMin--;
	xMax++;
	yMin--;
	yMax++;
	BOUND_REGION();
	FOREACH_REGION({
		if (Data_Grid_terrain[gridOffset] & Terrain_Aqueduct && Data_Grid_aqueducts[gridOffset] <= 15) {
			int waterOffset = Data_Grid_graphicIds[gridOffset] - image_group(ID_Graphic_Aqueduct);
			if (waterOffset >= 0 && waterOffset < 15) {
				waterOffset = 0;
			} else {
				waterOffset = 15;
			}
			setTileAqueduct(gridOffset, waterOffset, includeOverlay);
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
				graphicId = image_group(ID_Graphic_TerrainGrass2);
			} else {
				graphicId = image_group(ID_Graphic_TerrainGrass1);
			}
			TerrainGraphics_updateAreaEmptyLand(xx, yy, 1,
				graphicId + (Data_Grid_random[gridOffset] & 7));
		}
	});
	FOREACH_REGION({
		if (!(Data_Grid_terrain[gridOffset] & Terrain_NotClear) &&
			!Data_Grid_graphicIds[gridOffset]) {
			int graphicId;
			if (Data_Grid_bitfields[gridOffset] & Bitfield_AlternateTerrain) {
				graphicId = image_group(ID_Graphic_TerrainGrass2);
			} else {
				graphicId = image_group(ID_Graphic_TerrainGrass1);
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
	int forbiddenTerrain = Terrain_Aqueduct | Terrain_Elevation | Terrain_AccessRamp |
			Terrain_Rubble | Terrain_Road | Terrain_Building | Terrain_Garden;
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
			(Data_Grid_bitfields[gridOffset] & Bitfield_PlazaOrEarthquake)) {
			TerrainGraphics_setTileEarthquake(xx, yy);
		}
	});
}

void TerrainGraphics_updateRegionRubble(int xMin, int yMin, int xMax, int yMax)
{
	int forbiddenTerrain = Terrain_Aqueduct | Terrain_Elevation | Terrain_AccessRamp |
			Terrain_Road | Terrain_Building | Terrain_Garden;
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
			if (buildingId && Data_Buildings[Data_Grid_buildingIds[gridOffset]].type != BUILDING_BURNING_RUIN) {
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
					image_group(ID_Graphic_TerrainRubble) + (Data_Grid_random[gridOffset] & 7);
			}
		}
	}
}

static void setFarmCropTile(int buildingId, int x, int y, int dx, int dy, int cropGraphicId, int growth)
{
	int gridOffset = GridOffset(x + dx, y + dy);
	Data_Grid_terrain[gridOffset] &= Terrain_2e80;
	Data_Grid_terrain[gridOffset] |= Terrain_Building;
	Data_Grid_buildingIds[gridOffset] = buildingId;
	Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;
	Data_Grid_edge[gridOffset] = EdgeXY(dx, dy) | Edge_LeftmostTile;
	Data_Grid_graphicIds[gridOffset] = cropGraphicId + (growth < 4 ? growth : 4);
}

void TerrainGraphics_setBuildingFarm(int buildingId, int x, int y, int cropGraphicId, int progress)
{
	if (IsOutsideMap(x, y, 3)) {
		return;
	}
	// farmhouse
	int leftmostX, leftmostY;
	switch (Data_Settings_Map.orientation) {
		case Dir_0_Top: leftmostX = 0; leftmostY = 1; break;
		case Dir_2_Right: leftmostX = 0; leftmostY = 0; break;
		case Dir_4_Bottom: leftmostX = 1; leftmostY = 0; break;
		case Dir_6_Left: leftmostX = 1; leftmostY = 1; break;
		default: return;
	}
	for (int dy = 0; dy < 2; dy++) {
		for (int dx = 0; dx < 2; dx++) {
			int gridOffset = GridOffset(x + dx, y + dy);
			Data_Grid_terrain[gridOffset] &= Terrain_2e80;
			Data_Grid_terrain[gridOffset] |= Terrain_Building;
			Data_Grid_buildingIds[gridOffset] = buildingId;
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;
			Data_Grid_bitfields[gridOffset] |= Bitfield_Size2;
			Data_Grid_graphicIds[gridOffset] = image_group(ID_Graphic_FarmHouse);
			Data_Grid_edge[gridOffset] = EdgeXY(dx, dy);
			if (dx == leftmostX && dy == leftmostY) {
				Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
			}
		}
	}
	// crop tile 1
	int growth = progress / 10;
	setFarmCropTile(buildingId, x, y, 0, 2, cropGraphicId, growth);
	
	// crop tile 2
	growth -= 4;
	if (growth < 0) {
		growth = 0;
	}
	setFarmCropTile(buildingId, x, y, 1, 2, cropGraphicId, growth);
	
	// crop tile 3
	growth -= 4;
	if (growth < 0) {
		growth = 0;
	}
	setFarmCropTile(buildingId, x, y, 2, 2, cropGraphicId, growth);
	
	// crop tile 4
	growth -= 4;
	if (growth < 0) {
		growth = 0;
	}
	setFarmCropTile(buildingId, x, y, 2, 1, cropGraphicId, growth);
	
	// crop tile 5
	growth -= 4;
	if (growth < 0) {
		growth = 0;
	}
	setFarmCropTile(buildingId, x, y, 2, 0, cropGraphicId, growth);
}

void TerrainGraphics_updateNativeCropProgress(int buildingId)
{
	struct Data_Building *b = &Data_Buildings[buildingId];
	b->data.industry.progress++;
	if (b->data.industry.progress >= 5) {
		b->data.industry.progress = 0;
	}
	Data_Grid_graphicIds[b->gridOffset] = image_group(ID_Graphic_FarmCrops) + b->data.industry.progress;
}

void TerrainGraphics_setTileWater(int x, int y)
{
	Data_Grid_terrain[GridOffset(x, y)] |= Terrain_Water;
	int xMin = x - 1;
	int xMax = x + 1;
	int yMin = y - 1;
	int yMax = y + 1;
	BOUND_REGION();
	FOREACH_REGION({
		if ((Data_Grid_terrain[gridOffset] & (Terrain_Water | Terrain_Building)) == Terrain_Water) {
			const TerrainGraphic *g = TerrainGraphicsContext_getShore(gridOffset);
			int graphicId = image_group(ID_Graphic_TerrainWater) + g->groupOffset + g->itemOffset;
			if (Terrain_existsTileWithinRadiusWithType(xx, yy, 1, 2, Terrain_Building)) {
				// fortified shore
				int base = image_group(ID_Graphic_TerrainWaterShore);
				switch (g->groupOffset) {
					case 8: graphicId = base + 10; break;
					case 12: graphicId = base + 11; break;
					case 16: graphicId = base + 9; break;
					case 20: graphicId = base + 8; break;
					case 24: graphicId = base + 18; break;
					case 28: graphicId = base + 16; break;
					case 32: graphicId = base + 19; break;
					case 36: graphicId = base + 17; break;
					case 50: graphicId = base + 12; break;
					case 51: graphicId = base + 14; break;
					case 52: graphicId = base + 13; break;
					case 53: graphicId = base + 15; break;
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
	Data_Grid_bitfields[gridOffset] |= Bitfield_PlazaOrEarthquake;
	
	int xMin = x - 1;
	int yMin = y - 1;
	int xMax = x + 1;
	int yMax = y + 1;
	BOUND_REGION();
	FOREACH_REGION({
		if ((Data_Grid_terrain[gridOffset] & Terrain_Rock) &&
			(Data_Grid_bitfields[gridOffset] & Bitfield_PlazaOrEarthquake)) {
			const TerrainGraphic *g = TerrainGraphicsContext_getEarthquake(gridOffset);
			if (g->isValid) {
				Data_Grid_graphicIds[gridOffset] =
					image_group(ID_Graphic_TerrainEarthquake) +
					g->groupOffset + g->itemOffset;
			} else {
				Data_Grid_graphicIds[gridOffset] = image_group(ID_Graphic_TerrainEarthquake);
			}
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
			Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
		}
	});
}

int TerrainGraphics_setTileRoad(int x, int y)
{
	int gridOffset = GridOffset(x, y);
	int tilesSet = 0;
	if (!(Data_Grid_terrain[gridOffset] & Terrain_Road)) {
		tilesSet = 1;
	}
	Data_Grid_terrain[gridOffset] |= Terrain_Road;
	Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;
	
	int xMin = x - 1;
	int yMin = y - 1;
	int xMax = x + 1;
	int yMax = y + 1;
	BOUND_REGION();
	FOREACH_REGION({
		int terrain = Data_Grid_terrain[gridOffset];
		if (terrain & Terrain_Road && !(terrain & (Terrain_Water | Terrain_Building))) {
			if (terrain & Terrain_Aqueduct) {
				setRoadWithAqueductGraphic(gridOffset);
			} else {
				setRoadGraphic(gridOffset);
			}
		}
	});
	return tilesSet;
}

static int getGatehouseBuildingId(int gridOffset)
{
	if (Data_Grid_terrain[gridOffset] & Terrain_Gatehouse) {
		return Data_Grid_buildingIds[gridOffset];
	}
	return 0;
}

static int getGatehousePosition(int gridOffset, int direction, int buildingId)
{
	int result = 0;
	if (direction == Dir_0_Top) {
		if (Data_Grid_terrain[gridOffset + DELTA(1, -1)] & Terrain_Gatehouse &&
				Data_Grid_buildingIds[gridOffset + DELTA(1, -1)] == buildingId) {
			result = 1;
			if (!(Data_Grid_terrain[gridOffset + DELTA(1, 0)] & Terrain_Wall)) {
				result = 0;
			}
			if (Data_Grid_terrain[gridOffset + DELTA(-1, 0)] & Terrain_Wall &&
				Data_Grid_terrain[gridOffset + DELTA(-1, 1)] & Terrain_Wall) {
				result = 2;
			}
			if (!(Data_Grid_terrain[gridOffset + DELTA(0, 1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
			if (!(Data_Grid_terrain[gridOffset + DELTA(1, 1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
		} else if (Data_Grid_terrain[gridOffset + DELTA(-1, -1)] & Terrain_Gatehouse &&
				Data_Grid_buildingIds[gridOffset + DELTA(-1, -1)] == buildingId) {
			result = 3;
			if (!(Data_Grid_terrain[gridOffset + DELTA(-1, 0)] & Terrain_Wall)) {
				result = 0;
			}
			if (Data_Grid_terrain[gridOffset + DELTA(1, 0)] & Terrain_Wall &&
				Data_Grid_terrain[gridOffset + DELTA(1, 1)] & Terrain_Wall) {
				result = 4;
			}
			if (!(Data_Grid_terrain[gridOffset + DELTA(0, 1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
			if (!(Data_Grid_terrain[gridOffset + DELTA(-1, 1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
		}
	} else if (direction == Dir_6_Left) {
		if (Data_Grid_terrain[gridOffset + DELTA(-1, 1)] & Terrain_Gatehouse &&
				Data_Grid_buildingIds[gridOffset + DELTA(-1, 1)] == buildingId) {
			result = 1;
			if (!(Data_Grid_terrain[gridOffset + DELTA(0, 1)] & Terrain_Wall)) {
				result = 0;
			}
			if (Data_Grid_terrain[gridOffset + DELTA(0, -1)] & Terrain_Wall &&
				Data_Grid_terrain[gridOffset + DELTA(1, -1)] & Terrain_Wall) {
				result = 2;
			}
			if (!(Data_Grid_terrain[gridOffset + DELTA(1, 0)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
			if (!(Data_Grid_terrain[gridOffset + DELTA(1, 1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
		} else if (Data_Grid_terrain[gridOffset + DELTA(-1, -1)] & Terrain_Gatehouse &&
				Data_Grid_buildingIds[gridOffset + DELTA(-1, -1)] == buildingId) {
			result = 3;
			if (!(Data_Grid_terrain[gridOffset + DELTA(0, -1)] & Terrain_Wall)) {
				result = 0;
			}
			if (Data_Grid_terrain[gridOffset + DELTA(0, 1)] & Terrain_Wall &&
				Data_Grid_terrain[gridOffset + DELTA(1, 1)] & Terrain_Wall) {
				result = 4;
			}
			if (!(Data_Grid_terrain[gridOffset + DELTA(1, 0)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
			if (!(Data_Grid_terrain[gridOffset + DELTA(1, -1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
		}
	} else if (direction == Dir_4_Bottom) {
		if (Data_Grid_terrain[gridOffset + DELTA(1, 1)] & Terrain_Gatehouse &&
				Data_Grid_buildingIds[gridOffset + DELTA(1, 1)] == buildingId) {
			result = 1;
			if (!(Data_Grid_terrain[gridOffset + DELTA(1, 0)] & Terrain_Wall)) {
				result = 0;
			}
			if (Data_Grid_terrain[gridOffset + DELTA(-1, 0)] & Terrain_Wall &&
				Data_Grid_terrain[gridOffset + DELTA(-1, -1)] & Terrain_Wall) {
				result = 2;
			}
			if (!(Data_Grid_terrain[gridOffset + DELTA(0, -1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
			if (!(Data_Grid_terrain[gridOffset + DELTA(1, -1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
		} else if (Data_Grid_terrain[gridOffset + DELTA(-1, 1)] & Terrain_Gatehouse &&
				Data_Grid_buildingIds[gridOffset + DELTA(-1, 1)] == buildingId) {
			result = 3;
			if (!(Data_Grid_terrain[gridOffset + DELTA(-1, 0)] & Terrain_Wall)) {
				result = 0;
			}
			if (Data_Grid_terrain[gridOffset + DELTA(1, 0)] & Terrain_Wall &&
				Data_Grid_terrain[gridOffset + DELTA(1, -1)] & Terrain_Wall) {
				result = 4;
			}
			if (!(Data_Grid_terrain[gridOffset + DELTA(0, -1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
			if (!(Data_Grid_terrain[gridOffset + DELTA(-1, -1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
		}
	} else if (direction == Dir_2_Right) {
		if (Data_Grid_terrain[gridOffset + DELTA(1, 1)] & Terrain_Gatehouse &&
				Data_Grid_buildingIds[gridOffset + DELTA(1, 1)] == buildingId) {
			result = 1;
			if (!(Data_Grid_terrain[gridOffset + DELTA(0, 1)] & Terrain_Wall)) {
				result = 0;
			}
			if (Data_Grid_terrain[gridOffset + DELTA(0, -1)] & Terrain_Wall &&
				Data_Grid_terrain[gridOffset + DELTA(-1, -1)] & Terrain_Wall) {
				result = 2;
			}
			if (!(Data_Grid_terrain[gridOffset + DELTA(-1, 0)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
			if (!(Data_Grid_terrain[gridOffset + DELTA(-1, 1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
		} else if (Data_Grid_terrain[gridOffset + DELTA(1, -1)] & Terrain_Gatehouse &&
				Data_Grid_buildingIds[gridOffset + DELTA(1, -1)] == buildingId) {
			result = 3;
			if (!(Data_Grid_terrain[gridOffset + DELTA(0, -1)] & Terrain_Wall)) {
				result = 0;
			}
			if (Data_Grid_terrain[gridOffset + DELTA(0, 1)] & Terrain_Wall &&
				Data_Grid_terrain[gridOffset + DELTA(-1, 1)] & Terrain_Wall) {
				result = 4;
			}
			if (!(Data_Grid_terrain[gridOffset + DELTA(-1, 0)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
			if (!(Data_Grid_terrain[gridOffset + DELTA(-1, -1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
		}
	}
	return result;
}

static void setWallGatehouseGraphicManually(int gridOffset)
{
	int buildingIdUp = getGatehouseBuildingId(gridOffset + DELTA(0, -1));
	int buildingIdLeft = getGatehouseBuildingId(gridOffset + DELTA(-1, 0));
	int buildingIdDown = getGatehouseBuildingId(gridOffset + DELTA(0, 1));
	int buildingIdRight = getGatehouseBuildingId(gridOffset + DELTA(1, 0));
	int graphicOffset = 0;
	if (Data_Settings_Map.orientation == Dir_0_Top) {
		if (buildingIdUp && !buildingIdLeft) {
			int pos = getGatehousePosition(gridOffset, Dir_0_Top, buildingIdUp);
			if (pos > 0) {
				if (pos <= 2) {
					graphicOffset = 29;
				} else if (pos == 3) {
					graphicOffset = 31;
				} else {
					graphicOffset = 33;
				}
			}
		} else if (buildingIdLeft && !buildingIdUp) {
			int pos = getGatehousePosition(gridOffset, Dir_6_Left, buildingIdLeft);
			if (pos > 0) {
				if (pos <= 2) {
					graphicOffset = 30;
				} else if (pos == 3) {
					graphicOffset = 32;
				} else {
					graphicOffset = 33;
				}
			}
		}
	} else if (Data_Settings_Map.orientation == Dir_2_Right) {
		if (buildingIdUp && !buildingIdRight) {
			int pos = getGatehousePosition(gridOffset, Dir_0_Top, buildingIdUp);
			if (pos > 0) {
				if (pos == 1) {
					graphicOffset = 32;
				} else if (pos == 2) {
					graphicOffset = 33;
				} else {
					graphicOffset = 30;
				}
			}
		} else if (buildingIdRight && !buildingIdUp) {
			int pos = getGatehousePosition(gridOffset, Dir_2_Right, buildingIdRight);
			if (pos > 0) {
				if (pos <= 2) {
					graphicOffset = 29;
				} else if (pos == 3) {
					graphicOffset = 31;
				} else {
					graphicOffset = 33;
				}
			}
		}
	} else if (Data_Settings_Map.orientation == Dir_4_Bottom) {
		if (buildingIdDown && !buildingIdRight) {
			int pos = getGatehousePosition(gridOffset, Dir_4_Bottom, buildingIdDown);
			if (pos > 0) {
				if (pos == 1) {
					graphicOffset = 31;
				} else if (pos == 2) {
					graphicOffset = 33;
				} else {
					graphicOffset = 29;
				}
			}
		} else if (buildingIdRight && !buildingIdDown) {
			int pos = getGatehousePosition(gridOffset, Dir_2_Right, buildingIdRight);
			if (pos > 0) {
				if (pos == 1) {
					graphicOffset = 32;
				} else if (pos == 2) {
					graphicOffset = 33;
				} else {
					graphicOffset = 30;
				}
			}
		}
	} else if (Data_Settings_Map.orientation == Dir_6_Left) {
		if (buildingIdDown && !buildingIdLeft) {
			int pos = getGatehousePosition(gridOffset, Dir_4_Bottom, buildingIdDown);
			if (pos > 0) {
				if (pos <= 2) {
					graphicOffset = 30;
				} else if (pos == 3) {
					graphicOffset = 32;
				} else {
					graphicOffset = 33;
				}
			}
		} else if (buildingIdLeft && !buildingIdDown) {
			int pos = getGatehousePosition(gridOffset, Dir_6_Left, buildingIdLeft);
			if (pos > 0) {
				if (pos == 1) {
					graphicOffset = 31;
				} else if (pos == 2) {
					graphicOffset = 33;
				} else {
					graphicOffset = 29;
				}
			}
		}
	}
	if (graphicOffset) {
		Data_Grid_graphicIds[gridOffset] = image_group(ID_Graphic_Wall) + graphicOffset;
	}
}

static int isAdjacentToGatehouse(int gridOffset)
{
	return Terrain_countTerrainTypeDirectlyAdjacentTo(gridOffset, Terrain_Gatehouse) > 0;
}

static void setWallGraphic(int gridOffset)
{
	if (!(Data_Grid_terrain[gridOffset] & Terrain_Wall) ||
		Data_Grid_terrain[gridOffset] & Terrain_Building) {
		return;
	}
	const TerrainGraphic *graphic = TerrainGraphicsContext_getWall(gridOffset);
	Data_Grid_graphicIds[gridOffset] = image_group(ID_Graphic_Wall) +
		graphic->groupOffset + graphic->itemOffset;
	Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
	Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
	if (isAdjacentToGatehouse(gridOffset)) {
		graphic = TerrainGraphicsContext_getWallGatehouse(gridOffset);
		if (graphic->isValid) {
			Data_Grid_graphicIds[gridOffset] = image_group(ID_Graphic_Wall) +
				graphic->groupOffset + graphic->itemOffset;
		} else {
			setWallGatehouseGraphicManually(gridOffset);
		}
	}
}

int TerrainGraphics_setTileWall(int x, int y)
{
	int gridOffset = GridOffset(x, y);
	int tilesSet = 0;
	if (!(Data_Grid_terrain[gridOffset] & Terrain_Wall)) {
		tilesSet = 1;
	}
	Data_Grid_terrain[gridOffset] = Terrain_Wall;
	Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;

	int xMin = x - 1;
	int yMin = y - 1;
	int xMax = x + 1;
	int yMax = y + 1;
	BOUND_REGION();
	FOREACH_REGION({
		setWallGraphic(gridOffset);
	});
	return tilesSet;
}

int TerrainGraphics_setTileAqueduct(int x, int y, int forceNoWater)
{
	int gridOffset = GridOffset(x, y);
	int tilesSet = 0;
	if (Data_Grid_aqueducts[gridOffset] <= 15 && !(Data_Grid_terrain[gridOffset] & Terrain_Building)) {
		tilesSet = 1;
		int waterOffset;
		if (Data_Grid_graphicIds[gridOffset] - image_group(ID_Graphic_Aqueduct) >= 15) {
			waterOffset = 15;
		} else {
			waterOffset = 0;
		}
		if (forceNoWater) {
			waterOffset = 15;
		}
		setTileAqueduct(gridOffset, waterOffset, 0);
	}
	return tilesSet;
}

int TerrainGraphics_setTileAqueductTerrain(int x, int y)
{
	int gridOffset = GridOffset(x,y);
	Data_Grid_terrain[gridOffset] |= Terrain_Aqueduct;
	Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;
	return 1;
}

static void TerrainGraphics_setTileRubble(int x, int y)
{
	int gridOffset = GridOffset(x, y);
	Data_Grid_graphicIds[gridOffset] =
		image_group(ID_Graphic_TerrainRubble) + (Data_Grid_random[gridOffset] & 7);
	Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
	Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
	Data_Grid_aqueducts[gridOffset] = 0;
}

static void TerrainGraphics_updateTileMeadow(int x, int y)
{
	int forbiddenTerrain = Terrain_Aqueduct | Terrain_Elevation | Terrain_AccessRamp |
			Terrain_Rubble | Terrain_Road | Terrain_Building | Terrain_Garden;

	int xMin = x - 1;
	int yMin = y - 1;
	int xMax = x + 1;
	int yMax = y + 1;
	int graphicId = image_group(ID_Graphic_TerrainMeadow);
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

void TerrainGraphics_updateAreaWalls(int x, int y, int size)
{
	int xMin = x - 1;
	int yMin = y - 1;
	int xMax = xMin + size - 1;
	int yMax = yMin + size - 1;
	BOUND_REGION();
	FOREACH_REGION({
		setWallGraphic(gridOffset);
	});
}

void TerrainGraphics_updateAreaRoads(int x, int y, int size)
{
	int xMin = x - 1;
	int yMin = y - 1;
	int xMax = xMin + size - 1;
	int yMax = yMin + size - 1;
	BOUND_REGION();
	FOREACH_REGION({
		if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
			if (!(Data_Grid_terrain[gridOffset] & (Terrain_Water | Terrain_Building))) {
				setRoadGraphic(gridOffset);
			}
		}
	});
}

int TerrainGraphics_getFreeTileForHerd(int x, int y, int allowNegDes, int *xTile, int *yTile)
{
	int xMin = x - 4;
	int xMax = x + 4;
	int yMin = y - 4;
	int yMax = y + 4;
	unsigned short disallowedTerrain = ~(Terrain_AccessRamp | Terrain_Meadow);
	int tileFound = 0;
	int tileX = 0, tileY = 0;
	BOUND_REGION();
	FOREACH_REGION({
		if (!(Data_Grid_terrain[gridOffset] & disallowedTerrain)) {
			if (Data_Grid_romanSoldierConcentration[gridOffset]) {
				return 0;
			}
			if (allowNegDes) {
				if (Data_Grid_desirability[gridOffset] > 1) {
					return 0;
				}
			} else if (Data_Grid_desirability[gridOffset]) {
				return 0;
			}
			tileFound = 1;
			tileX = xx;
			tileY = yy;
		}
	});
	*xTile = tileX;
	*yTile = tileY;
	return tileFound;
}
