#include "TerrainGraphics.h"

#include "Data/State.h"
#include "Terrain.h"

#include "building/building.h"
#include "core/direction.h"
#include "graphics/image.h"
#include "map/aqueduct.h"
#include "map/building.h"
#include "map/building_tiles.h"
#include "map/desirability.h"
#include "map/elevation.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/image_context.h"
#include "map/property.h"
#include "map/random.h"
#include "map/soldier_strength.h"
#include "map/sprite.h"
#include "map/terrain.h"

#define FOREACH_ALL(block) \
    {int gridOffset = Data_State.map.gridStartOffset;\
    for (int y = 0; y < Data_State.map.height; y++, gridOffset += Data_State.map.gridBorderSize) {\
        for (int x = 0; x < Data_State.map.width; x++, gridOffset++) {\
            block;\
        }\
    }}

#define FOREACH_REGION(block) \
    {int gridOffset = map_grid_offset(xMin, yMin);\
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
static void setWallGraphic(int gridOffset);
static void setRoadGraphic(int gridOffset);

static int isAllTerrainInArea(int x, int y, int size, int terrain)
{
	if (!map_grid_is_inside(x, y, size)) {
		return 0;
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = map_grid_offset(x + dx, y + dy);
			if ((map_terrain_get(gridOffset) & TERRAIN_NOT_CLEAR) != terrain) {
				return 0;
			}
			if (map_image_at(gridOffset) != 0) {
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
	if (desirability > 0 && map_terrain_is(gridOffset, TERRAIN_FOUNTAIN_RANGE)) {
		return 1;
	}
	return 0;
}

static void setRoadWithAqueductGraphic(int gridOffset)
{
	int graphicIdAqueduct = image_group(GROUP_BUILDING_AQUEDUCT);
	int waterOffset;
	if (map_image_at(gridOffset) < graphicIdAqueduct + 15) {
		waterOffset = 0;
	} else {
		waterOffset = 15;
	}
	const terrain_image *image = map_image_context_get_aqueduct(gridOffset, 0);
	int groupOffset = image->group_offset;
	if (!image->aqueduct_offset) {
		if (map_terrain_is(gridOffset + map_grid_delta(0, -1), TERRAIN_ROAD)) {
			groupOffset = 3;
		} else {
			groupOffset = 2;
		}
	}
	if (TerrainGraphics_isPavedRoadTile(gridOffset)) {
		map_image_set(gridOffset, graphicIdAqueduct + waterOffset + groupOffset - 2);
	} else {
		map_image_set(gridOffset, graphicIdAqueduct + waterOffset + groupOffset + 6);
	}
	map_property_set_multi_tile_size(gridOffset, 1);
	map_property_mark_draw_tile(gridOffset);
}

static void setRoadGraphic(int gridOffset)
{
	if (map_property_is_plaza_or_earthquake(gridOffset)) {
		return;
	}
	if (map_terrain_is(gridOffset, TERRAIN_AQUEDUCT)) {
		setRoadWithAqueductGraphic(gridOffset);
		return;
	}
	if (TerrainGraphics_isPavedRoadTile(gridOffset)) {
		const terrain_image *image = map_image_context_get_paved_road(gridOffset);
		map_image_set(gridOffset, image_group(GROUP_TERRAIN_ROAD) +
			          image->group_offset + image->item_offset);
	} else {
		const terrain_image *image = map_image_context_get_dirt_road(gridOffset);
		map_image_set(gridOffset, image_group(GROUP_TERRAIN_ROAD) +
			          image->group_offset + image->item_offset + 49);
	}
	map_property_set_multi_tile_size(gridOffset, 1);
	map_property_mark_draw_tile(gridOffset);
}

static void setTileAqueduct(int gridOffset, int waterOffset, int includeOverlay)
{
	const terrain_image *image = map_image_context_get_aqueduct(gridOffset, includeOverlay);
	int groupOffset = image->group_offset;
	if (map_terrain_is(gridOffset, TERRAIN_ROAD)) {
		map_property_clear_plaza_or_earthquake(gridOffset);
		if (!image->aqueduct_offset) {
			if (map_terrain_is(gridOffset + map_grid_delta(0, -1), TERRAIN_ROAD)) {
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
	map_image_set(gridOffset, image_group(GROUP_BUILDING_AQUEDUCT) +
		waterOffset + groupOffset + image->item_offset);
	map_property_set_multi_tile_size(gridOffset, 1);
	map_property_mark_draw_tile(gridOffset);
	map_aqueduct_set(gridOffset, image->aqueduct_offset);
}

void TerrainGraphics_updateAllRocks()
{
	FOREACH_ALL({
		if (map_terrain_is(gridOffset, TERRAIN_ROCK) && !map_terrain_is(gridOffset, TERRAIN_RESERVOIR_RANGE | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP)) {
			map_image_set(gridOffset, 0);
			map_property_set_multi_tile_size(gridOffset, 1);
			map_property_mark_draw_tile(gridOffset);
		}
	});
	int graphicIdRock = image_group(GROUP_TERRAIN_ROCK);
	int graphicIdElevation = image_group(GROUP_TERRAIN_ELEVATION_ROCK);
	FOREACH_ALL({
		if (map_terrain_is(gridOffset, TERRAIN_ROCK) && !map_terrain_is(gridOffset, TERRAIN_RESERVOIR_RANGE | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP)) {
			if (!map_image_at(gridOffset)) {
				if (isAllTerrainInArea(x, y, 3, TERRAIN_ROCK)) {
					int graphicId = 12 + (map_random_get(gridOffset) & 1);
					if (map_terrain_exists_tile_in_radius_with_type(x, y, 3, 4, TERRAIN_ELEVATION)) {
						graphicId += graphicIdElevation;
					} else {
						graphicId += graphicIdRock;
					}
					map_building_tiles_add(0, x, y, 3, graphicId, TERRAIN_ROCK);
				} else if (isAllTerrainInArea(x, y, 2, TERRAIN_ROCK)) {
					int graphicId = 8 + (map_random_get(gridOffset) & 3);
					if (map_terrain_exists_tile_in_radius_with_type(x, y, 2, 4, TERRAIN_ELEVATION)) {
						graphicId += graphicIdElevation;
					} else {
						graphicId += graphicIdRock;
					}
					map_building_tiles_add(0, x, y, 2, graphicId, TERRAIN_ROCK);
				} else {
					int graphicId = map_random_get(gridOffset) & 7;
					if (map_terrain_exists_tile_in_radius_with_type(x, y, 1, 4, TERRAIN_ELEVATION)) {
						graphicId += graphicIdElevation;
					} else {
						graphicId += graphicIdRock;
					}
					map_image_set(gridOffset, graphicId);
				}
			}
		}
	});
}

void TerrainGraphics_updateAllGardens()
{
	FOREACH_ALL({
		if (map_terrain_is(gridOffset, TERRAIN_GARDEN) && !map_terrain_is(gridOffset, TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP)) {
			map_image_set(gridOffset, 0);
			map_property_set_multi_tile_size(gridOffset, 1);
			map_property_mark_draw_tile(gridOffset);
		}
	});
	FOREACH_ALL({
		if (map_terrain_is(gridOffset, TERRAIN_GARDEN) && !map_terrain_is(gridOffset, TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP)) {
			if (!map_image_at(gridOffset)) {
				int graphicId = image_group(GROUP_TERRAIN_GARDEN);
				if (isAllTerrainInArea(x, y, 2, TERRAIN_GARDEN)) {
					switch (map_random_get(gridOffset) & 3) {
						case 0: case 1: graphicId += 6; break;
						case 2: graphicId += 5; break;
						case 3: graphicId += 4; break;
					}
					map_building_tiles_add(0, x, y, 2, graphicId, TERRAIN_GARDEN);
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
					map_image_set(gridOffset, graphicId);
				}
			}
		}
	});
}

void TerrainGraphics_determineGardensFromGraphicIds()
{
	int baseGraphicId = image_group(GROUP_TERRAIN_GARDEN);
	FOREACH_ALL({
		int image_id = map_image_at(gridOffset);
		if (image_id >= baseGraphicId && image_id <= baseGraphicId + 6) {
			map_terrain_add(gridOffset, TERRAIN_GARDEN);
			map_property_clear_constructing(gridOffset);
			map_aqueduct_set(gridOffset, 0);
		}
	});
}

void TerrainGraphics_updateAllRoads()
{
	FOREACH_ALL({
		if (map_terrain_is(gridOffset, TERRAIN_ROAD)) {
			if (!map_terrain_is(gridOffset, TERRAIN_WATER | TERRAIN_BUILDING)) {
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
	if (!map_grid_is_inside(x, y, 1)) {
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
				if (map_terrain_is(gridOffset, TERRAIN_ELEVATION)) {
					rightTiles++;
				}
				height = map_elevation_at(gridOffset);
			} else if (i < 4) { // 1st row
				if (map_terrain_is(gridOffset, TERRAIN_ACCESS_RAMP) &&
					map_elevation_at(gridOffset) < height) {
					rightTiles++;
				}
			} else { // higher row beyond access ramp
				if (map_terrain_is(gridOffset, TERRAIN_ELEVATION)) {
					if (map_elevation_at(gridOffset) != height) {
						rightTiles++;
					}
				} else if (map_elevation_at(gridOffset) >= height) {
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
		if (map_terrain_is(gridOffset, TERRAIN_ACCESS_RAMP)) {
			map_image_set(gridOffset, 0);
		}
	});
	FOREACH_REGION({
		if (map_terrain_is(gridOffset, TERRAIN_ACCESS_RAMP) && !map_image_at(gridOffset)) {
			int graphicOffset = getAccessRampGraphicOffset(xx, yy);
			if (graphicOffset < 0) {
				// invalid map: remove access ramp
				map_terrain_remove(gridOffset, TERRAIN_ACCESS_RAMP);
				map_property_set_multi_tile_size(gridOffset, 1);
				map_property_mark_draw_tile(gridOffset);
				if (map_elevation_at(gridOffset)) {
					map_terrain_add(gridOffset, TERRAIN_ELEVATION);
				} else {
					map_terrain_remove(gridOffset, TERRAIN_ELEVATION);
					map_image_set(gridOffset,
						image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(gridOffset) & 7));
				}
			} else {
				map_building_tiles_add(0, xx, yy, 2,
					image_group(GROUP_TERRAIN_ACCESS_RAMP) + graphicOffset, TERRAIN_ACCESS_RAMP);
			}
		}
		if (map_elevation_at(gridOffset) && !map_terrain_is(gridOffset, TERRAIN_ACCESS_RAMP)) {
			const terrain_image *image = map_image_context_get_elevation(gridOffset, map_elevation_at(gridOffset));
			if (image->group_offset == 44) {
				map_terrain_remove(gridOffset, TERRAIN_ELEVATION);
				int terrain = map_terrain_get(gridOffset);
				if (!(terrain & TERRAIN_BUILDING)) {
					map_property_set_multi_tile_xy(gridOffset, 0, 0, 1);
					if (terrain & TERRAIN_SCRUB) {
						map_image_set(gridOffset, image_group(GROUP_TERRAIN_SHRUB) + (map_random_get(gridOffset) & 7));
					} else if (terrain & TERRAIN_TREE) {
						map_image_set(gridOffset, image_group(GROUP_TERRAIN_TREE) + (map_random_get(gridOffset) & 7));
					} else if (terrain & TERRAIN_ROAD) {
						TerrainGraphics_setTileRoad(xx, yy);
					} else if (terrain & TERRAIN_AQUEDUCT) {
						TerrainGraphics_setTileAqueduct(xx, yy, 0);
					} else if (terrain & TERRAIN_MEADOW) {
						map_image_set(gridOffset, image_group(GROUP_TERRAIN_MEADOW) + (map_random_get(gridOffset) & 3));
					} else {
						map_image_set(gridOffset, image_group(GROUP_TERRAIN_GRASS_1) + (map_random_get(gridOffset) & 7));
					}
				}
			} else {
				map_property_set_multi_tile_xy(gridOffset, 0, 0, 1);
				map_terrain_add(gridOffset, TERRAIN_ELEVATION);
				map_image_set(gridOffset, image_group(GROUP_TERRAIN_ELEVATION) + image->group_offset + image->item_offset);
			}
		}
	});
}

static int isTilePlaza(int gridOffset)
{
	if (map_terrain_is(gridOffset, TERRAIN_ROAD) &&
		map_property_is_plaza_or_earthquake(gridOffset) &&
		!map_terrain_is(gridOffset, TERRAIN_WATER | TERRAIN_BUILDING) &&
		!map_image_at(gridOffset)) {
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
		if (map_terrain_is(gridOffset, TERRAIN_ROAD) &&
			map_property_is_plaza_or_earthquake(gridOffset)) {
			if (map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
				map_property_clear_plaza_or_earthquake(gridOffset);
			}
		}
	});
	// convert plazas to single tile and remove graphic ids
	FOREACH_ALL({
		if (map_terrain_is(gridOffset, TERRAIN_ROAD) &&
			map_property_is_plaza_or_earthquake(gridOffset)) {
			map_image_set(gridOffset, 0);
			map_property_set_multi_tile_size(gridOffset, 1);
			map_property_mark_draw_tile(gridOffset);
		}
	});
	FOREACH_REGION({
		if (map_terrain_is(gridOffset, TERRAIN_ROAD) &&
			map_property_is_plaza_or_earthquake(gridOffset) &&
			!map_image_at(gridOffset)) {
			int graphicId = image_group(GROUP_TERRAIN_PLAZA);
			if (isTwoTileSquarePlaza(gridOffset)) {
				if (map_random_get(gridOffset) & 1) {
					graphicId += 7;
				} else {
					graphicId += 6;
				}
				map_building_tiles_add(0, xx, yy, 2, graphicId, TERRAIN_ROAD);
			} else {
				// single tile plaza
				switch ((xx & 1) + (yy & 1)) {
					case 2: graphicId += 1; break;
					case 1: graphicId += 2; break;
				}
				map_image_set(gridOffset, graphicId);
			}
		}
	});
}

void TerrainGraphics_updateRegionWater(int xMin, int yMin, int xMax, int yMax)
{
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		if (map_terrain_is(gridOffset, TERRAIN_WATER) && !map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
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
		if (map_terrain_is(gridOffset, TERRAIN_AQUEDUCT) && map_aqueduct_at(gridOffset) <= 15) {
			int waterOffset = map_image_at(gridOffset) - image_group(GROUP_BUILDING_AQUEDUCT);
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
		if (!map_terrain_is(gridOffset, TERRAIN_NOT_CLEAR)) {
			map_image_set(gridOffset, 0);
			map_property_set_multi_tile_size(gridOffset, 1);
			map_property_mark_draw_tile(gridOffset);
		}
	});
	FOREACH_REGION({
		if (!map_terrain_is(gridOffset, TERRAIN_NOT_CLEAR) &&
			!map_image_at(gridOffset) &&
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
		if (!map_terrain_is(gridOffset, TERRAIN_NOT_CLEAR) &&
			!map_image_at(gridOffset)) {
			int graphicId;
			if (map_property_is_alternate_terrain(gridOffset)) {
				graphicId = image_group(GROUP_TERRAIN_GRASS_2);
			} else {
				graphicId = image_group(GROUP_TERRAIN_GRASS_1);
			}
			if (Terrain_isClear(xx, yy, 4, TERRAIN_ALL, 1)) {
				TerrainGraphics_updateAreaEmptyLand(xx, yy, 4, graphicId + 42);
			} else if (Terrain_isClear(xx, yy, 3, TERRAIN_ALL, 1)) {
				TerrainGraphics_updateAreaEmptyLand(xx, yy, 3,
					graphicId + 24 + 9 * (map_random_get(gridOffset) & 1));
			} else if (Terrain_isClear(xx, yy, 2, TERRAIN_ALL, 1)) {
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
	int forbiddenTerrain = TERRAIN_AQUEDUCT | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP |
            TERRAIN_RUBBLE | TERRAIN_ROAD | TERRAIN_BUILDING | TERRAIN_GARDEN;
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		if (map_terrain_is(gridOffset, TERRAIN_MEADOW) && !map_terrain_is(gridOffset, forbiddenTerrain)) {
			TerrainGraphics_updateTileMeadow(xx, yy);
		}
	});
}

void TerrainGraphics_updateRegionEarthquake(int xMin, int yMin, int xMax, int yMax)
{
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		if (map_terrain_is(gridOffset, TERRAIN_ROCK) &&
			map_property_is_plaza_or_earthquake(gridOffset)) {
			TerrainGraphics_setTileEarthquake(xx, yy);
		}
	});
}

void TerrainGraphics_updateRegionRubble(int xMin, int yMin, int xMax, int yMax)
{
	int forbiddenTerrain = TERRAIN_AQUEDUCT | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP |
            TERRAIN_ROAD | TERRAIN_BUILDING | TERRAIN_GARDEN;
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		if (map_terrain_is(gridOffset, TERRAIN_RUBBLE) && !map_terrain_is(gridOffset, forbiddenTerrain)) {
			TerrainGraphics_setTileRubble(xx, yy);
		}
	});
}

void TerrainGraphics_setTileWater(int x, int y)
{
	map_terrain_add(map_grid_offset(x, y), TERRAIN_WATER);
	int xMin = x - 1;
	int xMax = x + 1;
	int yMin = y - 1;
	int yMax = y + 1;
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		if ((map_terrain_get(gridOffset) & (TERRAIN_WATER | TERRAIN_BUILDING)) == TERRAIN_WATER) {
			const terrain_image *image = map_image_context_get_shore(gridOffset);
			int graphicId = image_group(GROUP_TERRAIN_WATER) + image->group_offset + image->item_offset;
			if (map_terrain_exists_tile_in_radius_with_type(xx, yy, 1, 2, TERRAIN_BUILDING)) {
				// fortified shore
				int base = image_group(GROUP_TERRAIN_WATER_SHORE);
				switch (image->group_offset) {
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
			map_image_set(gridOffset, graphicId);
			map_property_set_multi_tile_size(gridOffset, 1);
			map_property_mark_draw_tile(gridOffset);
		}
	});
}

void TerrainGraphics_setTileEarthquake(int x, int y)
{
	// earthquake: terrain = rock && bitfields = plaza
	int gridOffset = map_grid_offset(x, y);
	map_terrain_add(gridOffset, TERRAIN_ROCK);
	map_property_mark_plaza_or_earthquake(gridOffset);
	
	int xMin = x - 1;
	int yMin = y - 1;
	int xMax = x + 1;
	int yMax = y + 1;
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		if (map_terrain_is(gridOffset, TERRAIN_ROCK) &&
			map_property_is_plaza_or_earthquake(gridOffset)) {
			const terrain_image *image = map_image_context_get_earthquake(gridOffset);
			if (image->is_valid) {
				map_image_set(gridOffset,
					image_group(GROUP_TERRAIN_EARTHQUAKE) +
					image->group_offset + image->item_offset);
			} else {
				map_image_set(gridOffset, image_group(GROUP_TERRAIN_EARTHQUAKE));
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
	if (!map_terrain_is(gridOffset, TERRAIN_ROAD)) {
		tilesSet = 1;
	}
	map_terrain_add(gridOffset, TERRAIN_ROAD);
	map_property_clear_constructing(gridOffset);
	
	int xMin = x - 1;
	int yMin = y - 1;
	int xMax = x + 1;
	int yMax = y + 1;
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		int terrain = map_terrain_get(gridOffset);
		if (terrain & TERRAIN_ROAD && !(terrain & (TERRAIN_WATER | TERRAIN_BUILDING))) {
			if (terrain & TERRAIN_AQUEDUCT) {
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
	if (map_terrain_is(gridOffset, TERRAIN_GATEHOUSE)) {
		return map_building_at(gridOffset);
	}
	return 0;
}

static int getGatehousePosition(int gridOffset, int direction, int buildingId)
{
	int result = 0;
	if (direction == DIR_0_TOP) {
		if (map_terrain_is(gridOffset + map_grid_delta(1, -1), TERRAIN_GATEHOUSE) &&
				map_building_at(gridOffset + map_grid_delta(1, -1)) == buildingId) {
			result = 1;
			if (!map_terrain_is(gridOffset + map_grid_delta(1, 0), TERRAIN_WALL)) {
				result = 0;
			}
			if (map_terrain_is(gridOffset + map_grid_delta(-1, 0), TERRAIN_WALL) &&
				map_terrain_is(gridOffset + map_grid_delta(-1, 1), TERRAIN_WALL)) {
				result = 2;
			}
			if (!map_terrain_is(gridOffset + map_grid_delta(0, 1), TERRAIN_WALL_OR_GATEHOUSE)) {
				result = 0;
			}
			if (!map_terrain_is(gridOffset + map_grid_delta(1, 1), TERRAIN_WALL_OR_GATEHOUSE)) {
				result = 0;
			}
		} else if (map_terrain_is(gridOffset + map_grid_delta(-1, -1), TERRAIN_GATEHOUSE) &&
				map_building_at(gridOffset + map_grid_delta(-1, -1)) == buildingId) {
			result = 3;
			if (!map_terrain_is(gridOffset + map_grid_delta(-1, 0), TERRAIN_WALL)) {
				result = 0;
			}
			if (map_terrain_is(gridOffset + map_grid_delta(1, 0), TERRAIN_WALL) &&
				map_terrain_is(gridOffset + map_grid_delta(1, 1), TERRAIN_WALL)) {
				result = 4;
			}
			if (!map_terrain_is(gridOffset + map_grid_delta(0, 1), TERRAIN_WALL_OR_GATEHOUSE)) {
				result = 0;
			}
			if (!map_terrain_is(gridOffset + map_grid_delta(-1, 1), TERRAIN_WALL_OR_GATEHOUSE)) {
				result = 0;
			}
		}
	} else if (direction == DIR_6_LEFT) {
		if (map_terrain_is(gridOffset + map_grid_delta(-1, 1), TERRAIN_GATEHOUSE) &&
				map_building_at(gridOffset + map_grid_delta(-1, 1)) == buildingId) {
			result = 1;
			if (!map_terrain_is(gridOffset + map_grid_delta(0, 1), TERRAIN_WALL)) {
				result = 0;
			}
			if (map_terrain_is(gridOffset + map_grid_delta(0, -1), TERRAIN_WALL) &&
				map_terrain_is(gridOffset + map_grid_delta(1, -1), TERRAIN_WALL)) {
				result = 2;
			}
			if (!map_terrain_is(gridOffset + map_grid_delta(1, 0), TERRAIN_WALL_OR_GATEHOUSE)) {
				result = 0;
			}
			if (!map_terrain_is(gridOffset + map_grid_delta(1, 1), TERRAIN_WALL_OR_GATEHOUSE)) {
				result = 0;
			}
		} else if (map_terrain_is(gridOffset + map_grid_delta(-1, -1), TERRAIN_GATEHOUSE) &&
				map_building_at(gridOffset + map_grid_delta(-1, -1)) == buildingId) {
			result = 3;
			if (!map_terrain_is(gridOffset + map_grid_delta(0, -1), TERRAIN_WALL)) {
				result = 0;
			}
			if (map_terrain_is(gridOffset + map_grid_delta(0, 1), TERRAIN_WALL) &&
				map_terrain_is(gridOffset + map_grid_delta(1, 1), TERRAIN_WALL)) {
				result = 4;
			}
			if (!map_terrain_is(gridOffset + map_grid_delta(1, 0), TERRAIN_WALL_OR_GATEHOUSE)) {
				result = 0;
			}
			if (!map_terrain_is(gridOffset + map_grid_delta(1, -1), TERRAIN_WALL_OR_GATEHOUSE)) {
				result = 0;
			}
		}
	} else if (direction == DIR_4_BOTTOM) {
		if (map_terrain_is(gridOffset + map_grid_delta(1, 1), TERRAIN_GATEHOUSE) &&
				map_building_at(gridOffset + map_grid_delta(1, 1)) == buildingId) {
			result = 1;
			if (!map_terrain_is(gridOffset + map_grid_delta(1, 0), TERRAIN_WALL)) {
				result = 0;
			}
			if (map_terrain_is(gridOffset + map_grid_delta(-1, 0), TERRAIN_WALL) &&
				map_terrain_is(gridOffset + map_grid_delta(-1, -1), TERRAIN_WALL)) {
				result = 2;
			}
			if (!map_terrain_is(gridOffset + map_grid_delta(0, -1), TERRAIN_WALL_OR_GATEHOUSE)) {
				result = 0;
			}
			if (!map_terrain_is(gridOffset + map_grid_delta(1, -1), TERRAIN_WALL_OR_GATEHOUSE)) {
				result = 0;
			}
		} else if (map_terrain_is(gridOffset + map_grid_delta(-1, 1), TERRAIN_GATEHOUSE) &&
				map_building_at(gridOffset + map_grid_delta(-1, 1)) == buildingId) {
			result = 3;
			if (!map_terrain_is(gridOffset + map_grid_delta(-1, 0), TERRAIN_WALL)) {
				result = 0;
			}
			if (map_terrain_is(gridOffset + map_grid_delta(1, 0), TERRAIN_WALL) &&
				map_terrain_is(gridOffset + map_grid_delta(1, -1), TERRAIN_WALL)) {
				result = 4;
			}
			if (!map_terrain_is(gridOffset + map_grid_delta(0, -1), TERRAIN_WALL_OR_GATEHOUSE)) {
				result = 0;
			}
			if (!map_terrain_is(gridOffset + map_grid_delta(-1, -1), TERRAIN_WALL_OR_GATEHOUSE)) {
				result = 0;
			}
		}
	} else if (direction == DIR_2_RIGHT) {
		if (map_terrain_is(gridOffset + map_grid_delta(1, 1), TERRAIN_GATEHOUSE) &&
				map_building_at(gridOffset + map_grid_delta(1, 1)) == buildingId) {
			result = 1;
			if (!map_terrain_is(gridOffset + map_grid_delta(0, 1), TERRAIN_WALL)) {
				result = 0;
			}
			if (map_terrain_is(gridOffset + map_grid_delta(0, -1), TERRAIN_WALL) &&
				map_terrain_is(gridOffset + map_grid_delta(-1, -1), TERRAIN_WALL)) {
				result = 2;
			}
			if (!map_terrain_is(gridOffset + map_grid_delta(-1, 0), TERRAIN_WALL_OR_GATEHOUSE)) {
				result = 0;
			}
			if (!map_terrain_is(gridOffset + map_grid_delta(-1, 1), TERRAIN_WALL_OR_GATEHOUSE)) {
				result = 0;
			}
		} else if (map_terrain_is(gridOffset + map_grid_delta(1, -1), TERRAIN_GATEHOUSE) &&
				map_building_at(gridOffset + map_grid_delta(1, -1)) == buildingId) {
			result = 3;
			if (!map_terrain_is(gridOffset + map_grid_delta(0, -1), TERRAIN_WALL)) {
				result = 0;
			}
			if (map_terrain_is(gridOffset + map_grid_delta(0, 1), TERRAIN_WALL) &&
				map_terrain_is(gridOffset + map_grid_delta(-1, 1), TERRAIN_WALL)) {
				result = 4;
			}
			if (!map_terrain_is(gridOffset + map_grid_delta(-1, 0), TERRAIN_WALL_OR_GATEHOUSE)) {
				result = 0;
			}
			if (!map_terrain_is(gridOffset + map_grid_delta(-1, -1), TERRAIN_WALL_OR_GATEHOUSE)) {
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
		map_image_set(gridOffset, image_group(GROUP_BUILDING_WALL) + graphicOffset);
	}
}

static int isAdjacentToGatehouse(int gridOffset)
{
	return map_terrain_count_directly_adjacent_with_type(gridOffset, TERRAIN_GATEHOUSE) > 0;
}

static void setWallGraphic(int gridOffset)
{
	if (!map_terrain_is(gridOffset, TERRAIN_WALL) ||
		map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
		return;
	}
	const terrain_image *image = map_image_context_get_wall(gridOffset);
	map_image_set(gridOffset, image_group(GROUP_BUILDING_WALL) +
		          image->group_offset + image->item_offset);
	map_property_set_multi_tile_size(gridOffset, 1);
	map_property_mark_draw_tile(gridOffset);
	if (isAdjacentToGatehouse(gridOffset)) {
		image = map_image_context_get_wall_gatehouse(gridOffset);
		if (image->is_valid) {
			map_image_set(gridOffset, image_group(GROUP_BUILDING_WALL) +
				          image->group_offset + image->item_offset);
		} else {
			setWallGatehouseGraphicManually(gridOffset);
		}
	}
}

int TerrainGraphics_setTileWall(int x, int y)
{
	int gridOffset = map_grid_offset(x, y);
	int tilesSet = 0;
	if (!map_terrain_is(gridOffset, TERRAIN_WALL)) {
		tilesSet = 1;
	}
	map_terrain_set(gridOffset, TERRAIN_WALL);
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
	if (map_aqueduct_at(gridOffset) <= 15 && !map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
		tilesSet = 1;
		int waterOffset;
		if (map_image_at(gridOffset) - image_group(GROUP_BUILDING_AQUEDUCT) >= 15) {
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
	map_terrain_add(gridOffset, TERRAIN_AQUEDUCT);
	map_property_clear_constructing(gridOffset);
	return 1;
}

static void TerrainGraphics_setTileRubble(int x, int y)
{
	int gridOffset = map_grid_offset(x, y);
	map_image_set(gridOffset, image_group(GROUP_TERRAIN_RUBBLE) + (map_random_get(gridOffset) & 7));
	map_property_set_multi_tile_size(gridOffset, 1);
	map_property_mark_draw_tile(gridOffset);
	map_aqueduct_set(gridOffset, 0);
}

static void TerrainGraphics_updateTileMeadow(int x, int y)
{
	int forbiddenTerrain = TERRAIN_AQUEDUCT | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP |
            TERRAIN_RUBBLE | TERRAIN_ROAD | TERRAIN_BUILDING | TERRAIN_GARDEN;

	int xMin = x - 1;
	int yMin = y - 1;
	int xMax = x + 1;
	int yMax = y + 1;
	int graphicId = image_group(GROUP_TERRAIN_MEADOW);
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		if (map_terrain_is(gridOffset, TERRAIN_MEADOW) && !map_terrain_is(gridOffset, forbiddenTerrain)) {
			int random = map_random_get(gridOffset) & 3;
			if (map_terrain_has_only_meadow_in_ring(xx, yy, 2)) {
				map_image_set(gridOffset, graphicId + random + 8);
			} else if (map_terrain_has_only_meadow_in_ring(xx, yy, 1)) {
				map_image_set(gridOffset, graphicId + random + 4);
			} else {
				map_image_set(gridOffset, graphicId + random);
			}
			map_property_set_multi_tile_size(gridOffset, 1);
			map_property_mark_draw_tile(gridOffset);
			map_aqueduct_set(gridOffset, 0);
		}
	});
}

static void TerrainGraphics_updateAreaEmptyLand(int x, int y, int size, int graphicId)
{
	if (!map_grid_is_inside(x, y, size)) {
		return;
	}
	int index = 0;
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = map_grid_offset(x + dx, y + dy);
			map_terrain_remove(gridOffset, TERRAIN_CLEARABLE);
			map_building_set(gridOffset, 0);
			map_property_clear_constructing(gridOffset);
			map_property_set_multi_tile_size(gridOffset, 1);
			map_property_mark_draw_tile(gridOffset);
			map_image_set(gridOffset, graphicId + index);
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
		if (map_terrain_is(gridOffset, TERRAIN_ROAD)) {
			if (!map_terrain_is(gridOffset, TERRAIN_WATER | TERRAIN_BUILDING)) {
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
	unsigned short disallowedTerrain = ~(TERRAIN_ACCESS_RAMP | TERRAIN_MEADOW);
	int tileFound = 0;
	int tileX = 0, tileY = 0;
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		if (!map_terrain_is(gridOffset, disallowedTerrain)) {
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
