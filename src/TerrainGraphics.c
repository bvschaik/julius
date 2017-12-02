#include "TerrainGraphics.h"

#include "Terrain.h"
#include "Terrain_private.h"

#include "Data/Building.h"

#include "core/direction.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/desirability.h"
#include "map/property.h"
#include "map/random.h"
#include "map/soldier_strength.h"

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
			int gridOffset = map_grid_offset(x + dx, y + dy);
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
	int desirability = map_desirability_get(gridOffset);
	if (desirability > 4) {
		return 1;
	}
	if (desirability > 0 && Data_Grid_terrain[gridOffset] & Terrain_FountainRange) {
		return 1;
	}
	return 0;
}

static void setRoadWithAqueductGraphic(int gridOffset)
{
	int graphicIdAqueduct = image_group(GROUP_BUILDING_AQUEDUCT);
	int waterOffset;
	if (Data_Grid_graphicIds[gridOffset] < graphicIdAqueduct + 15) {
		waterOffset = 0;
	} else {
		waterOffset = 15;
	}
	const struct TerrainGraphic *graphic = TerrainGraphicsContext_getAqueduct(gridOffset, 0);
	int groupOffset = graphic->groupOffset;
	if (!graphic->aqueductOffset) {
		if (Data_Grid_terrain[gridOffset + map_grid_delta(0, -1)] & Terrain_Road) {
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
	map_property_set_multi_tile_size(gridOffset, 1);
	map_property_mark_draw_tile(gridOffset);
}

static void setRoadGraphic(int gridOffset)
{
	if (map_property_is_plaza_or_earthquake(gridOffset)) {
		return;
	}
	if (Data_Grid_terrain[gridOffset] & Terrain_Aqueduct) {
		setRoadWithAqueductGraphic(gridOffset);
		return;
	}
	if (TerrainGraphics_isPavedRoadTile(gridOffset)) {
		const struct TerrainGraphic *graphic = TerrainGraphicsContext_getPavedRoad(gridOffset);
		Data_Grid_graphicIds[gridOffset] = image_group(GROUP_TERRAIN_ROAD) +
			graphic->groupOffset + graphic->itemOffset;
	} else {
		const struct TerrainGraphic *graphic = TerrainGraphicsContext_getDirtRoad(gridOffset);
		Data_Grid_graphicIds[gridOffset] = image_group(GROUP_TERRAIN_ROAD) +
			graphic->groupOffset + graphic->itemOffset + 49;
	}
	map_property_set_multi_tile_size(gridOffset, 1);
	map_property_mark_draw_tile(gridOffset);
}

static void setTileAqueduct(int gridOffset, int waterOffset, int includeOverlay)
{
	const struct TerrainGraphic *graphic = TerrainGraphicsContext_getAqueduct(gridOffset, includeOverlay);
	int groupOffset = graphic->groupOffset;
	if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
		map_property_clear_plaza_or_earthquake(gridOffset);
		if (!graphic->aqueductOffset) {
			if (Data_Grid_terrain[gridOffset + map_grid_delta(0, -1)] & Terrain_Road) {
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
	Data_Grid_graphicIds[gridOffset] = image_group(GROUP_BUILDING_AQUEDUCT) +
		waterOffset + groupOffset + graphic->itemOffset;
	map_property_set_multi_tile_size(gridOffset, 1);
	map_property_mark_draw_tile(gridOffset);
	Data_Grid_aqueducts[gridOffset] = graphic->aqueductOffset;
}

void TerrainGraphics_updateAllRocks()
{
	FOREACH_ALL({
		int terrain = Data_Grid_terrain[gridOffset];
		if ((terrain & Terrain_Rock) && !(terrain & (Terrain_ReservoirRange | Terrain_Elevation | Terrain_AccessRamp))) {
			Data_Grid_graphicIds[gridOffset] = 0;
			map_property_set_multi_tile_size(gridOffset, 1);
			map_property_mark_draw_tile(gridOffset);
		}
	});
	int graphicIdRock = image_group(GROUP_TERRAIN_ROCK);
	int graphicIdElevation = image_group(GROUP_TERRAIN_ELEVATION_ROCK);
	FOREACH_ALL({
		int terrain = Data_Grid_terrain[gridOffset];
		if ((terrain & Terrain_Rock) && !(terrain & (Terrain_ReservoirRange | Terrain_Elevation | Terrain_AccessRamp))) {
			if (!Data_Grid_graphicIds[gridOffset]) {
				if (isAllTerrainInArea(x, y, 3, Terrain_Rock)) {
					int graphicId = 12 + (map_random_get(gridOffset) & 1);
					if (Terrain_existsTileWithinRadiusWithType(x, y, 3, 4, Terrain_Elevation)) {
						graphicId += graphicIdElevation;
					} else {
						graphicId += graphicIdRock;
					}
					Terrain_addBuildingToGrids(0, x, y, 3, graphicId, Terrain_Rock);
				} else if (isAllTerrainInArea(x, y, 2, Terrain_Rock)) {
					int graphicId = 8 + (map_random_get(gridOffset) & 3);
					if (Terrain_existsTileWithinRadiusWithType(x, y, 2, 4, Terrain_Elevation)) {
						graphicId += graphicIdElevation;
					} else {
						graphicId += graphicIdRock;
					}
					Terrain_addBuildingToGrids(0, x, y, 2, graphicId, Terrain_Rock);
				} else {
					int graphicId = map_random_get(gridOffset) & 7;
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
			map_property_set_multi_tile_size(gridOffset, 1);
			map_property_mark_draw_tile(gridOffset);
		}
	});
	FOREACH_ALL({
		int terrain = Data_Grid_terrain[gridOffset];
		if (terrain & Terrain_Garden && !(terrain & (Terrain_Elevation | Terrain_AccessRamp))) {
			if (!Data_Grid_graphicIds[gridOffset]) {
				int graphicId = image_group(GROUP_TERRAIN_GARDEN);
				if (isAllTerrainInArea(x, y, 2, Terrain_Garden)) {
					switch (map_random_get(gridOffset) & 3) {
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
	int baseGraphicId = image_group(GROUP_TERRAIN_GARDEN);
	FOREACH_ALL({
		int graphicId = Data_Grid_graphicIds[gridOffset];
		if (graphicId >= baseGraphicId && graphicId <= baseGraphicId + 6) {
			Data_Grid_terrain[gridOffset] |= Terrain_Garden;
			map_property_clear_constructing(gridOffset);
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
	int baseOffset = map_grid_offset(x, y);
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
	switch (Data_State.map.orientation) {
		case DIR_0_TOP: break;
		case DIR_6_LEFT: graphicOffset += 1; break;
		case DIR_4_BOTTOM: graphicOffset += 2; break;
		case DIR_2_RIGHT: graphicOffset += 3; break;
	}
	if (graphicOffset >= 4) {
		graphicOffset -= 4;
	}
	return graphicOffset;
}

void TerrainGraphics_updateRegionElevation(int xMin, int yMin, int xMax, int yMax)
{
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
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
				map_property_set_multi_tile_size(gridOffset, 1);
				map_property_mark_draw_tile(gridOffset);
				if (Data_Grid_elevation[gridOffset]) {
					Data_Grid_terrain[gridOffset] |= Terrain_Elevation;
				} else {
					Data_Grid_terrain[gridOffset] &= ~Terrain_Elevation;
					Data_Grid_graphicIds[gridOffset] =
						image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(gridOffset) & 7);
				}
			} else {
				Terrain_addBuildingToGrids(0, xx, yy, 2,
					image_group(GROUP_TERRAIN_ACCESS_RAMP) + graphicOffset, Terrain_AccessRamp);
			}
		}
		if (Data_Grid_elevation[gridOffset] && !(Data_Grid_terrain[gridOffset] & Terrain_AccessRamp)) {
			const TerrainGraphic *g = TerrainGraphicsContext_getElevation(
				gridOffset, Data_Grid_elevation[gridOffset]);
			if (g->groupOffset == 44) {
				Data_Grid_terrain[gridOffset] &= ~Terrain_Elevation;
				int terrain = Data_Grid_terrain[gridOffset];
				if (!(terrain & Terrain_Building)) {
					map_property_set_multi_tile_xy(gridOffset, 0, 0, 1);
					if (terrain & Terrain_Scrub) {
						Data_Grid_graphicIds[gridOffset] =
							image_group(GROUP_TERRAIN_SHRUB) + (map_random_get(gridOffset) & 7);
					} else if (terrain & Terrain_Tree) {
						Data_Grid_graphicIds[gridOffset] =
							image_group(GROUP_TERRAIN_TREE) + (map_random_get(gridOffset) & 7);
					} else if (terrain & Terrain_Road) {
						TerrainGraphics_setTileRoad(xx, yy);
					} else if (terrain & Terrain_Aqueduct) {
						TerrainGraphics_setTileAqueduct(xx, yy, 0);
					} else if (terrain & Terrain_Meadow) {
						Data_Grid_graphicIds[gridOffset] =
							image_group(GROUP_TERRAIN_MEADOW) + (map_random_get(gridOffset) & 3);
					} else {
						Data_Grid_graphicIds[gridOffset] =
							image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(gridOffset) & 7);
					}
				}
			} else {
				map_property_set_multi_tile_xy(gridOffset, 0, 0, 1);
				Data_Grid_terrain[gridOffset] |= Terrain_Elevation;
				Data_Grid_graphicIds[gridOffset] =
					image_group(GROUP_TERRAIN_ELEVATION) + g->groupOffset + g->itemOffset;
			}
		}
	});
}

static int isTilePlaza(int gridOffset)
{
	if ((Data_Grid_terrain[gridOffset] & Terrain_Road) &&
		map_property_is_plaza_or_earthquake(gridOffset) &&
		!(Data_Grid_terrain[gridOffset] & (Terrain_Water | Terrain_Building)) &&
		!Data_Grid_graphicIds[gridOffset]) {
		return 1;
	}
	return 0;
}
static int isTwoTileSquarePlaza(int gridOffset)
{
	return
		isTilePlaza(gridOffset + map_grid_delta(1, 0)) &&
		isTilePlaza(gridOffset + map_grid_delta(0, 1)) &&
		isTilePlaza(gridOffset + map_grid_delta(1, 1));
}

void TerrainGraphics_updateRegionPlazas(int xMin, int yMin, int xMax, int yMax)
{
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	// remove plazas below buildings
	FOREACH_ALL({
		if (Data_Grid_terrain[gridOffset] & Terrain_Road &&
			map_property_is_plaza_or_earthquake(gridOffset)) {
			if (Data_Grid_terrain[gridOffset] & Terrain_Building) {
				map_property_clear_plaza_or_earthquake(gridOffset);
			}
		}
	});
	// convert plazas to single tile and remove graphic ids
	FOREACH_ALL({
		if (Data_Grid_terrain[gridOffset] & Terrain_Road &&
			map_property_is_plaza_or_earthquake(gridOffset)) {
			Data_Grid_graphicIds[gridOffset] = 0;
			map_property_set_multi_tile_size(gridOffset, 1);
			map_property_mark_draw_tile(gridOffset);
		}
	});
	FOREACH_REGION({
		if (Data_Grid_terrain[gridOffset] & Terrain_Road &&
			map_property_is_plaza_or_earthquake(gridOffset) &&
			!Data_Grid_graphicIds[gridOffset]) {
			int graphicId = image_group(GROUP_TERRAIN_PLAZA);
			if (isTwoTileSquarePlaza(gridOffset)) {
				if (map_random_get(gridOffset) & 1) {
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
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
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
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		if (Data_Grid_terrain[gridOffset] & Terrain_Aqueduct && Data_Grid_aqueducts[gridOffset] <= 15) {
			int waterOffset = Data_Grid_graphicIds[gridOffset] - image_group(GROUP_BUILDING_AQUEDUCT);
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
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		if (!(Data_Grid_terrain[gridOffset] & Terrain_NotClear)) {
			Data_Grid_graphicIds[gridOffset] = 0;
			map_property_set_multi_tile_size(gridOffset, 1);
			map_property_mark_draw_tile(gridOffset);
		}
	});
	FOREACH_REGION({
		if (!(Data_Grid_terrain[gridOffset] & Terrain_NotClear) &&
			!Data_Grid_graphicIds[gridOffset] &&
			!(map_random_get(gridOffset) & 0xf0)) {
			int graphicId;
			if (map_property_is_alternate_terrain(gridOffset)) {
				graphicId = image_group(GROUP_TERRAIN_GRASS_2);
			} else {
				graphicId = image_group(GROUP_TERRAIN_GRASS_1);
			}
			TerrainGraphics_updateAreaEmptyLand(xx, yy, 1,
				graphicId + (map_random_get(gridOffset) & 7));
		}
	});
	FOREACH_REGION({
		if (!(Data_Grid_terrain[gridOffset] & Terrain_NotClear) &&
			!Data_Grid_graphicIds[gridOffset]) {
			int graphicId;
			if (map_property_is_alternate_terrain(gridOffset)) {
				graphicId = image_group(GROUP_TERRAIN_GRASS_2);
			} else {
				graphicId = image_group(GROUP_TERRAIN_GRASS_1);
			}
			if (Terrain_isClear(xx, yy, 4, Terrain_All, 1)) {
				TerrainGraphics_updateAreaEmptyLand(xx, yy, 4, graphicId + 42);
			} else if (Terrain_isClear(xx, yy, 3, Terrain_All, 1)) {
				TerrainGraphics_updateAreaEmptyLand(xx, yy, 3,
					graphicId + 24 + 9 * (map_random_get(gridOffset) & 1));
			} else if (Terrain_isClear(xx, yy, 2, Terrain_All, 1)) {
				TerrainGraphics_updateAreaEmptyLand(xx, yy, 2,
					graphicId + 8 + 4 * (map_random_get(gridOffset) & 3));
			} else {
				TerrainGraphics_updateAreaEmptyLand(xx, yy, 1,
					graphicId + (map_random_get(gridOffset) & 7));
			}
		}
	});
}

void TerrainGraphics_updateRegionMeadow(int xMin, int yMin, int xMax, int yMax)
{
	int forbiddenTerrain = Terrain_Aqueduct | Terrain_Elevation | Terrain_AccessRamp |
			Terrain_Rubble | Terrain_Road | Terrain_Building | Terrain_Garden;
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		int terrain = Data_Grid_terrain[gridOffset];
		if ((terrain & Terrain_Meadow) && !(terrain & forbiddenTerrain)) {
			TerrainGraphics_updateTileMeadow(xx, yy);
		}
	});
}

void TerrainGraphics_updateRegionEarthquake(int xMin, int yMin, int xMax, int yMax)
{
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		if ((Data_Grid_terrain[gridOffset] & Terrain_Rock) &&
			map_property_is_plaza_or_earthquake(gridOffset)) {
			TerrainGraphics_setTileEarthquake(xx, yy);
		}
	});
}

void TerrainGraphics_updateRegionRubble(int xMin, int yMin, int xMax, int yMax)
{
	int forbiddenTerrain = Terrain_Aqueduct | Terrain_Elevation | Terrain_AccessRamp |
			Terrain_Road | Terrain_Building | Terrain_Garden;
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
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
			int gridOffset = map_grid_offset(x + dx, y + dy);
			if (map_building_at(gridOffset) != buildingId) {
				continue;
			}
			if (buildingId && Data_Buildings[map_building_at(gridOffset)].type != BUILDING_BURNING_RUIN) {
				Data_Grid_rubbleBuildingType[gridOffset] = (unsigned char) Data_Buildings[buildingId].type;
			}
			map_property_clear_constructing(gridOffset);
			map_property_set_multi_tile_size(gridOffset, 1);
			Data_Grid_aqueducts[gridOffset] = 0;
			map_building_set(gridOffset, 0);
			Data_Grid_buildingDamage[gridOffset] = 0;
			Data_Grid_spriteOffsets[gridOffset] = 0;
			map_property_set_multi_tile_xy(gridOffset, 0, 0, 1);
			if (Data_Grid_terrain[gridOffset] & Terrain_Water) {
				Data_Grid_terrain[gridOffset] &= Terrain_Water;
				TerrainGraphics_setTileWater(x + dx, y + dy);
			} else {
				Data_Grid_terrain[gridOffset] &= Terrain_2e80;
				Data_Grid_terrain[gridOffset] |= Terrain_Rubble;
				Data_Grid_graphicIds[gridOffset] =
					image_group(GROUP_TERRAIN_RUBBLE) + (map_random_get(gridOffset) & 7);
			}
		}
	}
}

static void setFarmCropTile(int buildingId, int x, int y, int dx, int dy, int cropGraphicId, int growth)
{
	int gridOffset = map_grid_offset(x + dx, y + dy);
	Data_Grid_terrain[gridOffset] &= Terrain_2e80;
	Data_Grid_terrain[gridOffset] |= Terrain_Building;
	map_building_set(gridOffset, buildingId);
	map_property_clear_constructing(gridOffset);
	map_property_set_multi_tile_xy(gridOffset, dx, dy, 1);
	Data_Grid_graphicIds[gridOffset] = cropGraphicId + (growth < 4 ? growth : 4);
}

void TerrainGraphics_setBuildingFarm(int buildingId, int x, int y, int cropGraphicId, int progress)
{
	if (IsOutsideMap(x, y, 3)) {
		return;
	}
	// farmhouse
	int leftmostX, leftmostY;
	switch (Data_State.map.orientation) {
		case DIR_0_TOP: leftmostX = 0; leftmostY = 1; break;
		case DIR_2_RIGHT: leftmostX = 0; leftmostY = 0; break;
		case DIR_4_BOTTOM: leftmostX = 1; leftmostY = 0; break;
		case DIR_6_LEFT: leftmostX = 1; leftmostY = 1; break;
		default: return;
	}
	for (int dy = 0; dy < 2; dy++) {
		for (int dx = 0; dx < 2; dx++) {
			int gridOffset = map_grid_offset(x + dx, y + dy);
			Data_Grid_terrain[gridOffset] &= Terrain_2e80;
			Data_Grid_terrain[gridOffset] |= Terrain_Building;
			map_building_set(gridOffset, buildingId);
			map_property_clear_constructing(gridOffset);
			map_property_set_multi_tile_size(gridOffset, 2);
			Data_Grid_graphicIds[gridOffset] = image_group(GROUP_BUILDING_FARM_HOUSE);
			map_property_set_multi_tile_xy(gridOffset, dx, dy,
			    dx == leftmostX && dy == leftmostY);
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
	Data_Grid_graphicIds[b->gridOffset] = image_group(GROUP_BUILDING_FARM_CROPS) + b->data.industry.progress;
}

void TerrainGraphics_setTileWater(int x, int y)
{
	Data_Grid_terrain[map_grid_offset(x, y)] |= Terrain_Water;
	int xMin = x - 1;
	int xMax = x + 1;
	int yMin = y - 1;
	int yMax = y + 1;
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		if ((Data_Grid_terrain[gridOffset] & (Terrain_Water | Terrain_Building)) == Terrain_Water) {
			const TerrainGraphic *g = TerrainGraphicsContext_getShore(gridOffset);
			int graphicId = image_group(GROUP_TERRAIN_WATER) + g->groupOffset + g->itemOffset;
			if (Terrain_existsTileWithinRadiusWithType(xx, yy, 1, 2, Terrain_Building)) {
				// fortified shore
				int base = image_group(GROUP_TERRAIN_WATER_SHORE);
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
			map_property_set_multi_tile_size(gridOffset, 1);
			map_property_mark_draw_tile(gridOffset);
		}
	});
}

void TerrainGraphics_setTileEarthquake(int x, int y)
{
	// earthquake: terrain = rock && bitfields = plaza
	int gridOffset = map_grid_offset(x, y);
	Data_Grid_terrain[gridOffset] |= Terrain_Rock;
	map_property_mark_plaza_or_earthquake(gridOffset);
	
	int xMin = x - 1;
	int yMin = y - 1;
	int xMax = x + 1;
	int yMax = y + 1;
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		if ((Data_Grid_terrain[gridOffset] & Terrain_Rock) &&
			map_property_is_plaza_or_earthquake(gridOffset)) {
			const TerrainGraphic *g = TerrainGraphicsContext_getEarthquake(gridOffset);
			if (g->isValid) {
				Data_Grid_graphicIds[gridOffset] =
					image_group(GROUP_TERRAIN_EARTHQUAKE) +
					g->groupOffset + g->itemOffset;
			} else {
				Data_Grid_graphicIds[gridOffset] = image_group(GROUP_TERRAIN_EARTHQUAKE);
			}
			map_property_set_multi_tile_size(gridOffset, 1);
			map_property_mark_draw_tile(gridOffset);
		}
	});
}

int TerrainGraphics_setTileRoad(int x, int y)
{
	int gridOffset = map_grid_offset(x, y);
	int tilesSet = 0;
	if (!(Data_Grid_terrain[gridOffset] & Terrain_Road)) {
		tilesSet = 1;
	}
	Data_Grid_terrain[gridOffset] |= Terrain_Road;
	map_property_clear_constructing(gridOffset);
	
	int xMin = x - 1;
	int yMin = y - 1;
	int xMax = x + 1;
	int yMax = y + 1;
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
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
		return map_building_at(gridOffset);
	}
	return 0;
}

static int getGatehousePosition(int gridOffset, int direction, int buildingId)
{
	int result = 0;
	if (direction == DIR_0_TOP) {
		if (Data_Grid_terrain[gridOffset + map_grid_delta(1, -1)] & Terrain_Gatehouse &&
				map_building_at(gridOffset + map_grid_delta(1, -1)) == buildingId) {
			result = 1;
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(1, 0)] & Terrain_Wall)) {
				result = 0;
			}
			if (Data_Grid_terrain[gridOffset + map_grid_delta(-1, 0)] & Terrain_Wall &&
				Data_Grid_terrain[gridOffset + map_grid_delta(-1, 1)] & Terrain_Wall) {
				result = 2;
			}
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(0, 1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(1, 1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
		} else if (Data_Grid_terrain[gridOffset + map_grid_delta(-1, -1)] & Terrain_Gatehouse &&
				map_building_at(gridOffset + map_grid_delta(-1, -1)) == buildingId) {
			result = 3;
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(-1, 0)] & Terrain_Wall)) {
				result = 0;
			}
			if (Data_Grid_terrain[gridOffset + map_grid_delta(1, 0)] & Terrain_Wall &&
				Data_Grid_terrain[gridOffset + map_grid_delta(1, 1)] & Terrain_Wall) {
				result = 4;
			}
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(0, 1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(-1, 1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
		}
	} else if (direction == DIR_6_LEFT) {
		if (Data_Grid_terrain[gridOffset + map_grid_delta(-1, 1)] & Terrain_Gatehouse &&
				map_building_at(gridOffset + map_grid_delta(-1, 1)) == buildingId) {
			result = 1;
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(0, 1)] & Terrain_Wall)) {
				result = 0;
			}
			if (Data_Grid_terrain[gridOffset + map_grid_delta(0, -1)] & Terrain_Wall &&
				Data_Grid_terrain[gridOffset + map_grid_delta(1, -1)] & Terrain_Wall) {
				result = 2;
			}
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(1, 0)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(1, 1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
		} else if (Data_Grid_terrain[gridOffset + map_grid_delta(-1, -1)] & Terrain_Gatehouse &&
				map_building_at(gridOffset + map_grid_delta(-1, -1)) == buildingId) {
			result = 3;
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(0, -1)] & Terrain_Wall)) {
				result = 0;
			}
			if (Data_Grid_terrain[gridOffset + map_grid_delta(0, 1)] & Terrain_Wall &&
				Data_Grid_terrain[gridOffset + map_grid_delta(1, 1)] & Terrain_Wall) {
				result = 4;
			}
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(1, 0)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(1, -1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
		}
	} else if (direction == DIR_4_BOTTOM) {
		if (Data_Grid_terrain[gridOffset + map_grid_delta(1, 1)] & Terrain_Gatehouse &&
				map_building_at(gridOffset + map_grid_delta(1, 1)) == buildingId) {
			result = 1;
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(1, 0)] & Terrain_Wall)) {
				result = 0;
			}
			if (Data_Grid_terrain[gridOffset + map_grid_delta(-1, 0)] & Terrain_Wall &&
				Data_Grid_terrain[gridOffset + map_grid_delta(-1, -1)] & Terrain_Wall) {
				result = 2;
			}
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(0, -1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(1, -1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
		} else if (Data_Grid_terrain[gridOffset + map_grid_delta(-1, 1)] & Terrain_Gatehouse &&
				map_building_at(gridOffset + map_grid_delta(-1, 1)) == buildingId) {
			result = 3;
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(-1, 0)] & Terrain_Wall)) {
				result = 0;
			}
			if (Data_Grid_terrain[gridOffset + map_grid_delta(1, 0)] & Terrain_Wall &&
				Data_Grid_terrain[gridOffset + map_grid_delta(1, -1)] & Terrain_Wall) {
				result = 4;
			}
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(0, -1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(-1, -1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
		}
	} else if (direction == DIR_2_RIGHT) {
		if (Data_Grid_terrain[gridOffset + map_grid_delta(1, 1)] & Terrain_Gatehouse &&
				map_building_at(gridOffset + map_grid_delta(1, 1)) == buildingId) {
			result = 1;
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(0, 1)] & Terrain_Wall)) {
				result = 0;
			}
			if (Data_Grid_terrain[gridOffset + map_grid_delta(0, -1)] & Terrain_Wall &&
				Data_Grid_terrain[gridOffset + map_grid_delta(-1, -1)] & Terrain_Wall) {
				result = 2;
			}
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(-1, 0)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(-1, 1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
		} else if (Data_Grid_terrain[gridOffset + map_grid_delta(1, -1)] & Terrain_Gatehouse &&
				map_building_at(gridOffset + map_grid_delta(1, -1)) == buildingId) {
			result = 3;
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(0, -1)] & Terrain_Wall)) {
				result = 0;
			}
			if (Data_Grid_terrain[gridOffset + map_grid_delta(0, 1)] & Terrain_Wall &&
				Data_Grid_terrain[gridOffset + map_grid_delta(-1, 1)] & Terrain_Wall) {
				result = 4;
			}
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(-1, 0)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
			if (!(Data_Grid_terrain[gridOffset + map_grid_delta(-1, -1)] & Terrain_WallOrGatehouse)) {
				result = 0;
			}
		}
	}
	return result;
}

static void setWallGatehouseGraphicManually(int gridOffset)
{
	int buildingIdUp = getGatehouseBuildingId(gridOffset + map_grid_delta(0, -1));
	int buildingIdLeft = getGatehouseBuildingId(gridOffset + map_grid_delta(-1, 0));
	int buildingIdDown = getGatehouseBuildingId(gridOffset + map_grid_delta(0, 1));
	int buildingIdRight = getGatehouseBuildingId(gridOffset + map_grid_delta(1, 0));
	int graphicOffset = 0;
	if (Data_State.map.orientation == DIR_0_TOP) {
		if (buildingIdUp && !buildingIdLeft) {
			int pos = getGatehousePosition(gridOffset, DIR_0_TOP, buildingIdUp);
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
			int pos = getGatehousePosition(gridOffset, DIR_6_LEFT, buildingIdLeft);
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
	} else if (Data_State.map.orientation == DIR_2_RIGHT) {
		if (buildingIdUp && !buildingIdRight) {
			int pos = getGatehousePosition(gridOffset, DIR_0_TOP, buildingIdUp);
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
			int pos = getGatehousePosition(gridOffset, DIR_2_RIGHT, buildingIdRight);
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
	} else if (Data_State.map.orientation == DIR_4_BOTTOM) {
		if (buildingIdDown && !buildingIdRight) {
			int pos = getGatehousePosition(gridOffset, DIR_4_BOTTOM, buildingIdDown);
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
			int pos = getGatehousePosition(gridOffset, DIR_2_RIGHT, buildingIdRight);
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
	} else if (Data_State.map.orientation == DIR_6_LEFT) {
		if (buildingIdDown && !buildingIdLeft) {
			int pos = getGatehousePosition(gridOffset, DIR_4_BOTTOM, buildingIdDown);
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
			int pos = getGatehousePosition(gridOffset, DIR_6_LEFT, buildingIdLeft);
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
		Data_Grid_graphicIds[gridOffset] = image_group(GROUP_BUILDING_WALL) + graphicOffset;
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
	Data_Grid_graphicIds[gridOffset] = image_group(GROUP_BUILDING_WALL) +
		graphic->groupOffset + graphic->itemOffset;
	map_property_set_multi_tile_size(gridOffset, 1);
	map_property_mark_draw_tile(gridOffset);
	if (isAdjacentToGatehouse(gridOffset)) {
		graphic = TerrainGraphicsContext_getWallGatehouse(gridOffset);
		if (graphic->isValid) {
			Data_Grid_graphicIds[gridOffset] = image_group(GROUP_BUILDING_WALL) +
				graphic->groupOffset + graphic->itemOffset;
		} else {
			setWallGatehouseGraphicManually(gridOffset);
		}
	}
}

int TerrainGraphics_setTileWall(int x, int y)
{
	int gridOffset = map_grid_offset(x, y);
	int tilesSet = 0;
	if (!(Data_Grid_terrain[gridOffset] & Terrain_Wall)) {
		tilesSet = 1;
	}
	Data_Grid_terrain[gridOffset] = Terrain_Wall;
	map_property_clear_constructing(gridOffset);

	int xMin = x - 1;
	int yMin = y - 1;
	int xMax = x + 1;
	int yMax = y + 1;
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		setWallGraphic(gridOffset);
	});
	return tilesSet;
}

int TerrainGraphics_setTileAqueduct(int x, int y, int forceNoWater)
{
	int gridOffset = map_grid_offset(x, y);
	int tilesSet = 0;
	if (Data_Grid_aqueducts[gridOffset] <= 15 && !(Data_Grid_terrain[gridOffset] & Terrain_Building)) {
		tilesSet = 1;
		int waterOffset;
		if (Data_Grid_graphicIds[gridOffset] - image_group(GROUP_BUILDING_AQUEDUCT) >= 15) {
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
	int gridOffset = map_grid_offset(x,y);
	Data_Grid_terrain[gridOffset] |= Terrain_Aqueduct;
	map_property_clear_constructing(gridOffset);
	return 1;
}

static void TerrainGraphics_setTileRubble(int x, int y)
{
	int gridOffset = map_grid_offset(x, y);
	Data_Grid_graphicIds[gridOffset] =
		image_group(GROUP_TERRAIN_RUBBLE) + (map_random_get(gridOffset) & 7);
	map_property_set_multi_tile_size(gridOffset, 1);
	map_property_mark_draw_tile(gridOffset);
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
	int graphicId = image_group(GROUP_TERRAIN_MEADOW);
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		int terrain = Data_Grid_terrain[gridOffset];
		if ((terrain & Terrain_Meadow) && !(terrain & forbiddenTerrain)) {
			int random = map_random_get(gridOffset) & 3;
			if (Terrain_isAllMeadowAtDistanceRing(xx, yy, 2)) {
				Data_Grid_graphicIds[gridOffset] = graphicId + random + 8;
			} else if (Terrain_isAllMeadowAtDistanceRing(xx, yy, 1)) {
				Data_Grid_graphicIds[gridOffset] = graphicId + random + 4;
			} else {
				Data_Grid_graphicIds[gridOffset] = graphicId + random;
			}
			map_property_set_multi_tile_size(gridOffset, 1);
			map_property_mark_draw_tile(gridOffset);
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
			int gridOffset = map_grid_offset(x + dx, y + dy);
			Data_Grid_terrain[gridOffset] &= Terrain_2e80;
			map_building_set(gridOffset, 0);
			map_property_clear_constructing(gridOffset);
			map_property_set_multi_tile_size(gridOffset, 1);
			map_property_mark_draw_tile(gridOffset);
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
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
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
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
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
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		if (!(Data_Grid_terrain[gridOffset] & disallowedTerrain)) {
			if (map_soldier_strength_get(gridOffset)) {
				return 0;
			}
			int desirability = map_desirability_get(gridOffset);
			if (allowNegDes) {
				if (desirability > 1) {
					return 0;
				}
			} else if (desirability) {
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
