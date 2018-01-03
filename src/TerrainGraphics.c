#include "TerrainGraphics.h"

#include "Data/State.h"
#include "Terrain.h"

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
#include "map/tiles.h"

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

static void setTileAqueduct(int gridOffset, int waterOffset, int includeConstruction)
{
	const terrain_image *image = map_image_context_get_aqueduct(gridOffset, includeConstruction);
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
		if (map_tiles_is_paved_road(gridOffset)) {
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
						map_tiles_set_road(xx, yy);
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

void TerrainGraphics_updateRegionAqueduct(int xMin, int yMin, int xMax, int yMax, int includeConstruction)
{
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
	FOREACH_REGION({
		if (map_terrain_is(gridOffset, TERRAIN_AQUEDUCT) && map_aqueduct_at(gridOffset) <= 15) {
			int waterOffset = map_image_at(gridOffset) - image_group(GROUP_BUILDING_AQUEDUCT);
			if (waterOffset >= 0 && waterOffset < 15) {
				waterOffset = 0;
			} else {
				waterOffset = 15;
			}
			setTileAqueduct(gridOffset, waterOffset, includeConstruction);
		}
	});
}

void TerrainGraphics_updateAllEarthquake()
{
	FOREACH_ALL({
		if (map_terrain_is(gridOffset, TERRAIN_ROCK) &&
			map_property_is_plaza_or_earthquake(gridOffset)) {
			TerrainGraphics_setTileEarthquake(x, y);
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
