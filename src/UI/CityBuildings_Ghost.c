#include "CityBuildings_private.h"

#include "../BuildingPlacement.h"
#include "../Terrain.h"
#include "../TerrainGraphics.h"

#include "building/building.h"
#include "building/count.h"
#include "building/properties.h"
#include "city/finance.h"
#include "core/direction.h"
#include "core/time.h"
#include "figure/formation.h"
#include "map/bridge.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/road_aqueduct.h"
#include "map/terrain.h"

static void drawBuildingGhostDraggableReservoir();
static void drawBuildingGhostAqueduct();
static void drawBuildingGhostFountain();
static void drawBuildingGhostBathhouse();
static void drawBuildingGhostBridge();
static void drawBuildingGhostFort();
static void drawBuildingGhostHippodrome();
static void drawBuildingGhostShipyardWharf();
static void drawBuildingGhostDock();
static void drawBuildingGhostRoad();
static void drawBuildingGhostDefault();
static void drawFlatTile(int xOffset, int yOffset, color_t mask);

static const int xViewOffsets[25] = {
	0,
	-30, 30, 0,
	-60, 60, -30, 30, 0,
	-90, 90, -60, 60, -30, 30, 0,
	-120, 120, -90, 90, -60, 60, -30, 30, 0
};

static const int yViewOffsets[25] = {
	0,
	15, 15, 30,
	30, 30, 45, 45, 60,
	45, 45, 60, 60, 75, 75, 90,
	60, 60, 75, 75, 90, 90, 105, 105, 120
};

static const int tileGridOffsets[4][25] = {
	{0,
	162, 1, 163,
	324, 2, 325, 164, 326,
	486, 3, 487, 165, 488, 327, 489,
	648, 4, 649, 166, 650, 328, 651, 490, 652},
	{0,
	-1, 162, 161,
	-2, 324, 160, 323, 322,
	-3, 486, 159, 485, 321, 484, 483,
	-4, 648, 158, 647, 320, 646, 482, 645, 644},
	{0,
	-162, -1, -163,
	-324, -2, -325, -164, -326,
	-486, -3, -487, -165, -488, -327, -489,
	-648, -4, -649, -166, -650, -328, -651, -490, -652},
	{0,
	1, -162, -161,
	2, -324, -160, -323, -322,
	3, -486, -159, -485, -321, -484, -483,
	4, -648, -158, -647, -320, -646, -482, -645, -644},
};

static const int fortGroundGridOffsets[4] = {-159, -158, 4, 3};
static const int fortGroundXViewOffsets[4] = {120, 90, -120, -90};
static const int fortGroundYViewOffsets[4] = {30, -75, -60, 45};

static const int hippodromeXViewOffsets[4] = {150, 150, -150, -150};
static const int hippodromeYViewOffsets[4] = {75, -75, -75, 75};

void UI_CityBuildings_drawSelectedBuildingGhost()
{
	if (!Data_State.map.current.gridOffset || Data_CityView.isScrolling) {
		return;
	}
	if (Data_State.selectedBuilding.drawAsConstructing || Data_State.selectedBuilding.type <= 0) {
		return;
	}
	switch (Data_State.selectedBuilding.type) {
		case BUILDING_DRAGGABLE_RESERVOIR:
			drawBuildingGhostDraggableReservoir();
			break;
		case BUILDING_AQUEDUCT:
			drawBuildingGhostAqueduct();
			break;
		case BUILDING_FOUNTAIN:
			drawBuildingGhostFountain();
			break;
		case BUILDING_BATHHOUSE:
			drawBuildingGhostBathhouse();
			break;
		case BUILDING_LOW_BRIDGE:
		case BUILDING_SHIP_BRIDGE:
			drawBuildingGhostBridge();
			break;
		case BUILDING_FORT_LEGIONARIES:
		case BUILDING_FORT_JAVELIN:
		case BUILDING_FORT_MOUNTED:
			drawBuildingGhostFort();
			break;
		case BUILDING_HIPPODROME:
			drawBuildingGhostHippodrome();
			break;
		case BUILDING_SHIPYARD:
		case BUILDING_WHARF:
			drawBuildingGhostShipyardWharf();
			break;
		case BUILDING_DOCK:
			drawBuildingGhostDock();
			break;
		case BUILDING_ROAD:
			drawBuildingGhostRoad();
			break;
		default:
			drawBuildingGhostDefault();
			break;
	}
}

static void drawBuildingGhostDefault()
{
	int fullyObstructed = 0;
	int placementObstructed = 0;
	if (Data_State.selectedBuilding.roadRequired > 0) {
		if (time_get_millis() > Data_State.selectedBuilding.roadLastUpdate + 1500) {
			Data_State.selectedBuilding.roadLastUpdate = time_get_millis();
			Data_State.selectedBuilding.roadRequired = Data_State.selectedBuilding.roadRequired == 1 ? 2 : 1;
		}
	}
	building_type type = Data_State.selectedBuilding.type;
    const building_properties *props = building_properties_for_type(type);
	int gridOffset = Data_State.map.current.gridOffset;
	int buildingSize = props->size;
	int graphicId = image_group(props->image_group) + props->image_offset;
	if (type == BUILDING_WAREHOUSE) {
		buildingSize = 3;
	}
	int xStart = 0, yStart = 0;
	// determine x and y offset
	switch (Data_State.map.orientation) {
		case DIR_0_TOP:
			xStart = Data_State.map.current.x;
			yStart = Data_State.map.current.y;
			break;
		case DIR_2_RIGHT:
			xStart = Data_State.map.current.x - buildingSize + 1;
			yStart = Data_State.map.current.y;
			break;
		case DIR_4_BOTTOM:
			xStart = Data_State.map.current.x - buildingSize + 1;
			yStart = Data_State.map.current.y - buildingSize + 1;
			break;
		case DIR_6_LEFT:
			xStart = Data_State.map.current.x;
			yStart = Data_State.map.current.y - buildingSize + 1;
			break;
	}
	// check if we can place building
	if (Data_State.selectedBuilding.meadowRequired) {
		if (!Terrain_existsTileWithinRadiusWithType(xStart, yStart, 3, 1, TERRAIN_MEADOW)) {
			fullyObstructed = 1;
			placementObstructed = 1;
		}
	} else if (Data_State.selectedBuilding.rockRequired) {
		if (!Terrain_existsTileWithinRadiusWithType(xStart, yStart, 2, 1, TERRAIN_ROCK)) {
			fullyObstructed = 1;
			placementObstructed = 1;
		}
	} else if (Data_State.selectedBuilding.treesRequired) {
		if (!Terrain_existsTileWithinRadiusWithType(xStart, yStart, 2, 1, TERRAIN_TREE | TERRAIN_SCRUB)) {
			fullyObstructed = 1;
			placementObstructed = 1;
		}
	} else if (Data_State.selectedBuilding.waterRequired) {
		if (!Terrain_existsTileWithinRadiusWithType(xStart, yStart, 2, 3, TERRAIN_WATER)) {
			fullyObstructed = 1;
			placementObstructed = 1;
		}
	} else if (Data_State.selectedBuilding.wallRequired) {
		if (!Terrain_existsTileWithinRadiusWithType(xStart, yStart, 2, 0, TERRAIN_WALL)) {
			fullyObstructed = 1;
			placementObstructed = 1;
		}
	} else if (Data_State.selectedBuilding.type == BUILDING_GATEHOUSE) {
		int orientation = Terrain_getOrientationGatehouse(
			Data_State.map.current.x, Data_State.map.current.y);
		int graphicOffset;
		if (orientation == 2) {
			graphicOffset = 1;
		} else if (orientation == 1) {
			graphicOffset = 0;
		} else {
			graphicOffset = Data_State.selectedBuilding.roadRequired == 2 ? 1 : 0;
		}
		if (Data_State.map.orientation == DIR_6_LEFT ||
			Data_State.map.orientation == DIR_2_RIGHT) {
			graphicOffset = 1 - graphicOffset;
		}
		graphicId += graphicOffset;
	} else if (Data_State.selectedBuilding.type == BUILDING_TRIUMPHAL_ARCH) {
		int orientation = Terrain_getOrientationTriumphalArch(
			Data_State.map.current.x, Data_State.map.current.y);
		int graphicOffset;
		if (orientation == 2) {
			graphicOffset = 2;
		} else if (orientation == 1) {
			graphicOffset = 0;
		} else {
			graphicOffset = Data_State.selectedBuilding.roadRequired == 2 ? 2 : 0;
		}
		if (Data_State.map.orientation == DIR_6_LEFT ||
			Data_State.map.orientation == DIR_2_RIGHT) {
			graphicOffset = 2 - graphicOffset;
		}
		graphicId += graphicOffset;
	}
	if (Data_State.selectedBuilding.type == BUILDING_SENATE_UPGRADED && Data_CityInfo.buildingSenatePlaced) {
		fullyObstructed = 1;
		placementObstructed = 1;
	}
	if (Data_State.selectedBuilding.type == BUILDING_BARRACKS && building_count_total(BUILDING_BARRACKS)) {
		fullyObstructed = 1;
		placementObstructed = 1;
	}

	int numTiles = buildingSize * buildingSize;
	int orientationIndex = Data_State.map.orientation / 2;
	for (int i = 0; i < numTiles; i++) {
		int tileOffset = gridOffset + tileGridOffsets[orientationIndex][i];
		int terrain = map_terrain_get(tileOffset) & TERRAIN_NOT_CLEAR;
		if (type == BUILDING_GATEHOUSE || type == BUILDING_TRIUMPHAL_ARCH || type == BUILDING_PLAZA) {
			terrain &= ~TERRAIN_ROAD;
		}
		if (type == BUILDING_TOWER) {
			terrain &= ~TERRAIN_WALL;
		}
		if (terrain || map_has_figure_at(tileOffset)) {
			placementObstructed = 1;
		}
	}
	if (type == BUILDING_PLAZA && !map_terrain_is(gridOffset, TERRAIN_ROAD)) {
		placementObstructed = 1;
	}
	if (city_finance_out_of_money()) {
		fullyObstructed = 1;
		placementObstructed = 1;
	}
	int xOffsetBase = Data_CityView.selectedTile.xOffsetInPixels;
	int yOffsetBase = Data_CityView.selectedTile.yOffsetInPixels;
	if (placementObstructed) {
		for (int i = 0; i < numTiles; i++) {
			int tileOffset = gridOffset + tileGridOffsets[orientationIndex][i];
			int tileObstructed = map_terrain_get(tileOffset) & TERRAIN_NOT_CLEAR;
			if (type == BUILDING_GATEHOUSE) {
				tileObstructed &= ~TERRAIN_ROAD;
			}
			if (map_has_figure_at(tileOffset)) {
				tileObstructed = 1;
			}
			int xOffset = xOffsetBase + xViewOffsets[i];
			int yOffset = yOffsetBase + yViewOffsets[i];
			if (fullyObstructed || tileObstructed) {
				drawFlatTile(xOffset, yOffset, COLOR_MASK_RED);
			} else {
				drawFlatTile(xOffset, yOffset, COLOR_MASK_GREEN);
			}
		}
	} else {
		// can place, draw ghost
		if (BuildingIsFarm(type)) {
			Graphics_drawIsometricFootprint(graphicId,
				xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId,
				xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
			// fields
			for (int i = 4; i < 9; i++) {
				Graphics_drawIsometricFootprint(graphicId + 1,
					xOffsetBase + xViewOffsets[i],
					yOffsetBase + yViewOffsets[i], COLOR_MASK_GREEN);
			}
		} else if (type == BUILDING_WAREHOUSE) {
			Graphics_drawIsometricFootprint(graphicId,
				xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId,
				xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
			Graphics_drawImageMasked(image_group(GROUP_BUILDING_WAREHOUSE) + 17,
				xOffsetBase - 4, yOffsetBase - 42, COLOR_MASK_GREEN);
			int graphicIdSpace = image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_EMPTY);
			for (int i = 1; i < 9; i++) {
				int xOffset = xOffsetBase + xViewOffsets[i];
				int yOffset = yOffsetBase + yViewOffsets[i];
				Graphics_drawIsometricFootprint(graphicIdSpace,
					xOffset, yOffset, COLOR_MASK_GREEN);
				Graphics_drawIsometricTop(graphicIdSpace,
					xOffset, yOffset, COLOR_MASK_GREEN);
			}
		} else if (type == BUILDING_GRANARY) {
			Graphics_drawIsometricFootprint(graphicId,
				xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
			Graphics_drawImageMasked(graphicId + 1,
				xOffsetBase + image_get(graphicId + 1)->sprite_offset_x - 32,
				yOffsetBase + image_get(graphicId + 1)->sprite_offset_y - 64,
				COLOR_MASK_GREEN);
		} else if (type == BUILDING_HOUSE_VACANT_LOT) {
			int graphicIdLot = image_group(GROUP_BUILDING_HOUSE_VACANT_LOT);
			Graphics_drawIsometricFootprint(graphicIdLot,
				xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicIdLot,
				xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
		} else if (type == BUILDING_TRIUMPHAL_ARCH) {
			Graphics_drawIsometricFootprint(graphicId,
				xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId,
				xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
			if (graphicId == image_group(GROUP_BUILDING_TRIUMPHAL_ARCH)) {
				Graphics_drawImageMasked(graphicId + 1,
					xOffsetBase + image_get(graphicId + 1)->sprite_offset_x + 4,
					yOffsetBase + image_get(graphicId + 1)->sprite_offset_y - 51,
					COLOR_MASK_GREEN);
			} else {
				Graphics_drawImageMasked(graphicId + 1,
					xOffsetBase + image_get(graphicId + 1)->sprite_offset_x - 33,
					yOffsetBase + image_get(graphicId + 1)->sprite_offset_y - 56,
					COLOR_MASK_GREEN);
			}
		} else if (type != BUILDING_CLEAR_LAND) {
			Graphics_drawIsometricFootprint(graphicId,
				xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId,
				xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
		}
	}
}

static void drawBuildingGhostDraggableReservoir()
{
	int placementObstructed = 0;
	if (Data_State.selectedBuilding.placementInProgress) {
		if (!Data_State.selectedBuilding.cost) {
			placementObstructed = 1;
		}
	} else {
		if (map_building_is_reservoir(Data_State.map.current.x - 1, Data_State.map.current.y - 1)) {
			placementObstructed = 0;
		} else if (!Terrain_isClear(
				Data_State.map.current.x - 1, Data_State.map.current.y - 1,
				3, TERRAIN_ALL, 0)) {
			placementObstructed = 1;
		}
	}
	if (city_finance_out_of_money()) {
		placementObstructed = 1;
	}
	if (Data_State.selectedBuilding.placementInProgress) {
		int xOffsetBase = Data_State.selectedBuilding.reservoirOffsetX;
		int yOffsetBase = Data_State.selectedBuilding.reservoirOffsetY - 30;
		if (placementObstructed) {
			for (int i = 0; i < 9; i++) {
				int xOffset = xOffsetBase + xViewOffsets[i];
				int yOffset = yOffsetBase + yViewOffsets[i];
				drawFlatTile(xOffset, yOffset, COLOR_MASK_RED);
			}
		} else {
			int graphicId = image_group(GROUP_BUILDING_RESERVOIR);
			Graphics_drawIsometricFootprint(graphicId, xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId, xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
		}
	}
	int xOffsetBase = Data_CityView.selectedTile.xOffsetInPixels;
	int yOffsetBase = Data_CityView.selectedTile.yOffsetInPixels - 30;
	if (placementObstructed) {
		for (int i = 0; i < 9; i++) {
			int xOffset = xOffsetBase + xViewOffsets[i];
			int yOffset = yOffsetBase + yViewOffsets[i];
			drawFlatTile(xOffset, yOffset, COLOR_MASK_RED);
		}
	} else {
		int graphicId = image_group(GROUP_BUILDING_RESERVOIR);
		Graphics_drawIsometricFootprint(graphicId, xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
		Graphics_drawIsometricTop(graphicId, xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
		if (Terrain_existsTileWithinAreaWithType(
			Data_State.map.current.x - 2, Data_State.map.current.y - 2,
			5, TERRAIN_WATER)) {
			const image *img = image_get(graphicId);
			Graphics_drawImageMasked(graphicId + 1,
				xOffsetBase - 58 + img->sprite_offset_x - 2,
				yOffsetBase + img->sprite_offset_y - (img->height - 90),
				COLOR_MASK_GREEN);
		}
	}
}

static void drawBuildingGhostAqueduct()
{
	int placementObstructed = 0;
	if (Data_State.selectedBuilding.placementInProgress) {
		if (!Data_State.selectedBuilding.cost) {
			placementObstructed = 1;
		}
	} else {
		int gridOffset = map_grid_offset(Data_State.map.current.x, Data_State.map.current.y);
		if (map_terrain_is(gridOffset, TERRAIN_ROAD)) {
			placementObstructed = Terrain_getAdjacentRoadTilesForAqueduct(gridOffset) == 2 ? 0 : 1;
		} else if (map_terrain_is(gridOffset, TERRAIN_NOT_CLEAR)) {
			placementObstructed = 1;
		}
	}
	if (city_finance_out_of_money()) {
		placementObstructed = 1;
	}
	int xOffset = Data_CityView.selectedTile.xOffsetInPixels;
	int yOffset = Data_CityView.selectedTile.yOffsetInPixels;
	if (placementObstructed) {
		drawFlatTile(xOffset, yOffset, COLOR_MASK_RED);
	} else {
		int gridOffset = Data_State.map.current.gridOffset;
		int graphicId = image_group(GROUP_BUILDING_AQUEDUCT);
		const struct TerrainGraphic *graphic = TerrainGraphicsContext_getAqueduct(gridOffset, 0);
		if (map_terrain_is(gridOffset, TERRAIN_ROAD)) {
			int groupOffset = graphic->groupOffset;
			if (!graphic->aqueductOffset) {
				if (map_terrain_is(gridOffset - 162, TERRAIN_ROAD)) {
					groupOffset = 3;
				} else {
					groupOffset = 2;
				}
			}
			if (TerrainGraphics_isPavedRoadTile(gridOffset)) {
				graphicId += groupOffset + 13;
			} else {
				graphicId += groupOffset + 21;
			}
		} else {
			graphicId += graphic->groupOffset + 15;
		}
		Graphics_drawIsometricFootprint(graphicId, xOffset, yOffset, COLOR_MASK_GREEN);
		Graphics_drawIsometricTop(graphicId, xOffset, yOffset, COLOR_MASK_GREEN);
	}
}

static void drawBuildingGhostFountain()
{
	int gridOffset = Data_State.map.current.gridOffset;
	int xOffset = Data_CityView.selectedTile.xOffsetInPixels;
	int yOffset = Data_CityView.selectedTile.yOffsetInPixels;

	int graphicId = image_group(building_properties_for_type(BUILDING_FOUNTAIN)->image_group);
	if (city_finance_out_of_money()) {
		drawFlatTile(xOffset, yOffset, COLOR_MASK_RED);
	} else {
		Graphics_drawIsometricFootprint(graphicId, xOffset, yOffset, COLOR_MASK_GREEN);
		Graphics_drawIsometricTop(graphicId, xOffset, yOffset, COLOR_MASK_GREEN);
		if (map_terrain_is(gridOffset, TERRAIN_RESERVOIR_RANGE)) {
			Graphics_drawImageMasked(graphicId + 1,
				xOffset + image_get(graphicId)->sprite_offset_x,
				yOffset + image_get(graphicId)->sprite_offset_y, COLOR_MASK_GREEN);
		}
	}
}

static void drawBuildingGhostBathhouse()
{
	int fullyObstructed = 0;
	int placementObstructed = 0;
	int gridOffset = Data_State.map.current.gridOffset;
	int numTiles = 4;
	int orientationIndex = Data_State.map.orientation / 2;
	for (int i = 0; i < numTiles; i++) {
		int tileOffset = gridOffset + tileGridOffsets[orientationIndex][i];
		if (map_terrain_is(tileOffset, TERRAIN_NOT_CLEAR)) {
			placementObstructed = 1;
		}
		if (map_has_figure_at(tileOffset)) {
			placementObstructed = 1;
		}
	}
	if (city_finance_out_of_money()) {
		fullyObstructed = 1;
		placementObstructed = 1;
	}

	int xOffsetBase = Data_CityView.selectedTile.xOffsetInPixels;
	int yOffsetBase = Data_CityView.selectedTile.yOffsetInPixels;
	if (placementObstructed) {
		for (int i = 0; i < numTiles; i++) {
			int tileOffset = gridOffset + tileGridOffsets[orientationIndex][i];
			int tileObstructed = map_terrain_is(tileOffset, TERRAIN_NOT_CLEAR);
			if (map_has_figure_at(tileOffset)) {
				tileObstructed = 1;
			}
			int xOffset = xOffsetBase + xViewOffsets[i];
			int yOffset = yOffsetBase + yViewOffsets[i];
			if (fullyObstructed || tileObstructed) {
				drawFlatTile(xOffset, yOffset, COLOR_MASK_RED);
			} else {
				drawFlatTile(xOffset, yOffset, COLOR_MASK_GREEN);
			}
		}
	} else {
		int graphicId = image_group(building_properties_for_type(BUILDING_BATHHOUSE)->image_group);
		Graphics_drawIsometricFootprint(graphicId, xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
		Graphics_drawIsometricTop(graphicId, xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
		int hasWater = 0;
		for (int i = 0; i < numTiles; i++) { // FIXED: was not accurate on rotated maps
			int tileOffset = gridOffset + tileGridOffsets[orientationIndex][i];
			if (map_terrain_is(tileOffset, TERRAIN_RESERVOIR_RANGE)) {
				hasWater = 1;
			}
		}
		if (hasWater) {
			Graphics_drawImageMasked(graphicId - 1,
				xOffsetBase + image_get(graphicId)->sprite_offset_x,
				yOffsetBase + image_get(graphicId)->sprite_offset_y,
				COLOR_MASK_GREEN);
		}
	}
}

static void drawBuildingGhostBridge()
{
	int length, direction;
	int endGridOffset = map_bridge_calculate_length_direction(
		Data_State.map.current.x, Data_State.map.current.y,
		Data_State.selectedBuilding.type == BUILDING_LOW_BRIDGE ? 0 : 1,
		&length, &direction);

	int dir = direction - Data_State.map.orientation;
	if (dir < 0) {
		dir += 8;
	}
	int obstructed = 0;
	if (Data_State.selectedBuilding.type == BUILDING_SHIP_BRIDGE && length < 5) {
		obstructed = 1;
	} else if (!endGridOffset) {
		obstructed = 1;
	}
	if (city_finance_out_of_money()) {
		obstructed = 1;
	}
	if (obstructed) {
		int xOffset = Data_CityView.selectedTile.xOffsetInPixels;
		int yOffset = Data_CityView.selectedTile.yOffsetInPixels;
		drawFlatTile(xOffset, yOffset, length > 0 ? COLOR_MASK_GREEN : COLOR_MASK_RED);
		if (length > 1) {
			switch (dir) {
				case DIR_0_TOP:
					xOffset += 29 * (length - 1);
					yOffset -= 15 * (length - 1);
					break;
				case DIR_2_RIGHT:
					xOffset += 29 * (length - 1);
					yOffset += 15 * (length - 1);
					break;
				case DIR_4_BOTTOM:
					xOffset -= 29 * (length - 1);
					yOffset += 15 * (length - 1);
					break;
				case DIR_6_LEFT:
					xOffset -= 29 * (length - 1);
					yOffset -= 15 * (length - 1);
					break;
				default: return;
			}
			drawFlatTile(xOffset, yOffset, COLOR_MASK_RED);
		}
		return;
	}

	// bridge can be built
	int xOffsetBase = Data_CityView.selectedTile.xOffsetInPixels;
	int yOffsetBase = Data_CityView.selectedTile.yOffsetInPixels;
	struct {
		int graphicId;
		int xOffset;
		int yOffset;
	} tiles[40];
	int numTiles = 0;
	int graphicBase = image_group(GROUP_BUILDING_BRIDGE);
	int graphicId, xOffset, yOffset;
	int xAdd, yAdd;
	switch (dir) {
		case DIR_0_TOP: xAdd = 29; yAdd = -15; break;
		case DIR_2_RIGHT: xAdd = 29; yAdd = 15; break;
		case DIR_4_BOTTOM: xAdd = -29; yAdd = 15; break;
		case DIR_6_LEFT: xAdd = -29; yAdd = -15; break;
		default: return;
	}
	if (Data_State.selectedBuilding.type == BUILDING_LOW_BRIDGE) {
		switch (dir) {
			case DIR_0_TOP:
				graphicId = graphicBase + 5;
				xOffset = xOffsetBase;
				yOffset = yOffsetBase - 20;
				break;
			case DIR_2_RIGHT:
				graphicId = graphicBase;
				xOffset = xOffsetBase - 1;
				yOffset = yOffsetBase - 8;
				break;
			case DIR_4_BOTTOM:
				graphicId = graphicBase + 3;
				xOffset = xOffsetBase;
				yOffset = yOffsetBase - 8;
				break;
			case DIR_6_LEFT:
				graphicId = graphicBase + 2;
				xOffset = xOffsetBase + 7;
				yOffset = yOffsetBase - 20;
				break;
		}
		Graphics_drawImageMasked(graphicId, xOffset, yOffset, COLOR_MASK_GREEN);
		tiles[numTiles].xOffset = xOffset;
		tiles[numTiles].yOffset = yOffset;
		tiles[numTiles++].graphicId = graphicId;
		for (int i = 1; i < length; i++) {
			xOffsetBase += xAdd;
			yOffsetBase += yAdd;
			if (i == length - 1) {
				switch (dir) {
					case DIR_0_TOP:
						graphicId = graphicBase + 3;
						xOffset = xOffsetBase;
						yOffset = yOffsetBase - 8;
						break;
					case DIR_2_RIGHT:
						graphicId = graphicBase + 2;
						xOffset = xOffsetBase + 7;
						yOffset = yOffsetBase - 20;
						break;
					case DIR_4_BOTTOM:
						graphicId = graphicBase + 5;
						xOffset = xOffsetBase;
						yOffset = yOffsetBase - 20;
						break;
					case DIR_6_LEFT:
						graphicId = graphicBase;
						xOffset = xOffsetBase - 1;
						yOffset = yOffsetBase - 8;
						break;
				}
			} else {
				if (dir == 0 || dir == 4) {
					graphicId = graphicBase + 4;
					xOffset = xOffsetBase;
				} else {
					graphicId = graphicBase + 1;
					xOffset = xOffsetBase + 5;
				}
				yOffset = yOffsetBase - 21;
			}
			Graphics_drawImageMasked(graphicId, xOffset, yOffset, COLOR_MASK_GREEN);
			tiles[numTiles].xOffset = xOffset;
			tiles[numTiles].yOffset = yOffset;
			tiles[numTiles++].graphicId = graphicId;
		}
	} else { // ship bridge
		int pillarDistance;
		switch (length) {
			case  9: pillarDistance = 4; break;
			case 10: pillarDistance = 4; break;
			case 11: pillarDistance = 5; break;
			case 12: pillarDistance = 5; break;
			case 13: pillarDistance = 6; break;
			case 14: pillarDistance = 6; break;
			case 15: pillarDistance = 7; break;
			case 16: pillarDistance = 7; break;
			default: pillarDistance = 8; break;
		}
		switch (dir) {
			case 0:
				graphicId = graphicBase + 11;
				xOffset = xOffsetBase - 3;
				yOffset = yOffsetBase - 50;
				break;
			case 2:
				graphicId = graphicBase + 6;
				xOffset = xOffsetBase - 1;
				yOffset = yOffsetBase - 12;
				break;
			case 4:
				graphicId = graphicBase + 9;
				xOffset = xOffsetBase - 30;
				yOffset = yOffsetBase - 12;
				break;
			case 6:
				graphicId = graphicBase + 8;
				xOffset = xOffsetBase - 23;
				yOffset = yOffsetBase - 53;
				break;
		}
		Graphics_drawImageMasked(graphicId, xOffset, yOffset, COLOR_MASK_GREEN);
		tiles[numTiles].xOffset = xOffset;
		tiles[numTiles].yOffset = yOffset;
		tiles[numTiles++].graphicId = graphicId;
		for (int i = 1; i < length; i++) {
			xOffsetBase += xAdd;
			yOffsetBase += yAdd;
			if (i == 1 || i == length - 1) {
				continue; // part of 2-tile graphic at i=0 or i=length-2
			}
			if (i == length - 2) {
				switch (dir) {
					case 0:
						graphicId = graphicBase + 9;
						xOffset = xOffsetBase + 1;
						yOffset = yOffsetBase - 24;
						break;
					case 2:
						graphicId = graphicBase + 8;
						xOffset = xOffsetBase + 7;
						yOffset = yOffsetBase - 39;
						break;
					case 4:
						graphicId = graphicBase + 11;
						xOffset = xOffsetBase - 34;
						yOffset = yOffsetBase - 35;
						break;
					case 6:
						graphicId = graphicBase + 6;
						xOffset = xOffsetBase - 29;
						yOffset = yOffsetBase - 22;
						break;
				}
			} else if (i == pillarDistance) {
				if (dir == 0 || dir == 4) {
					graphicId = graphicBase + 13;
					xOffset = xOffsetBase;
				} else {
					graphicId = graphicBase + 12;
					xOffset = xOffsetBase + 7;
				}
				yOffset = yOffsetBase - 38;
			} else {
				if (dir == 0 || dir == 4) {
					graphicId = graphicBase + 10;
					xOffset = xOffsetBase;
					yOffset = yOffsetBase - 37;
				} else {
					graphicId = graphicBase + 7;
					xOffset = xOffsetBase + 7;
					yOffset = yOffsetBase - 38;
				}
			}
			Graphics_drawImageMasked(graphicId, xOffset, yOffset, COLOR_MASK_GREEN);
			tiles[numTiles].xOffset = xOffset;
			tiles[numTiles].yOffset = yOffset;
			tiles[numTiles++].graphicId = graphicId;
		}
	}
	if (dir == 0 || dir == 6) {
		// draw in opposite order
		for (int i = numTiles - 1; i >= 0; i--) {
			Graphics_drawImageMasked(tiles[i].graphicId,
				tiles[i].xOffset, tiles[i].yOffset, COLOR_MASK_GREEN);
		}
	}
}

static void drawBuildingGhostFort()
{
	int fullyObstructed = 0;
	int placementObstructed = 0;
	if (formation_totals_get_num_legions() >= 6 || city_finance_out_of_money()) {
		fullyObstructed = 1;
		placementObstructed = 1;
	}

	int numTilesFort = building_properties_for_type(BUILDING_FORT)->size;
	numTilesFort *= numTilesFort;
	int numTilesGround = building_properties_for_type(BUILDING_FORT_GROUND)->size;
	numTilesGround *= numTilesGround;

	int orientationIndex = Data_State.map.orientation / 2;
	int gridOffsetFort = Data_State.map.current.gridOffset;
	int gridOffsetGround = gridOffsetFort + fortGroundGridOffsets[orientationIndex];

	for (int i = 0; i < numTilesFort; i++) {
		int tileOffset = gridOffsetFort + tileGridOffsets[orientationIndex][i];
		if (map_terrain_is(tileOffset, TERRAIN_NOT_CLEAR)) {
			placementObstructed = 1;
		}
	}
	for (int i = 0; i < numTilesGround; i++) {
		int tileOffset = gridOffsetGround + tileGridOffsets[orientationIndex][i];
		if (map_terrain_is(tileOffset, TERRAIN_NOT_CLEAR)) {
			placementObstructed = 1;
		}
	}

	int xOffsetBase = Data_CityView.selectedTile.xOffsetInPixels;
	int yOffsetBase = Data_CityView.selectedTile.yOffsetInPixels;
	int xOffsetGround = xOffsetBase + fortGroundXViewOffsets[orientationIndex];
	int yOffsetGround = yOffsetBase + fortGroundYViewOffsets[orientationIndex];

	if (placementObstructed) {
		for (int i = 0; i < numTilesFort; i++) {
			int tileOffset = gridOffsetFort + tileGridOffsets[orientationIndex][i];
			int tileObstructed = 0;
			if (map_terrain_is(tileOffset, TERRAIN_NOT_CLEAR)) {
				tileObstructed = 1;
			}
			int xOffset = xOffsetBase + xViewOffsets[i];
			int yOffset = yOffsetBase + yViewOffsets[i];
			if (fullyObstructed || tileObstructed) {
				drawFlatTile(xOffset, yOffset, COLOR_MASK_RED);
			} else {
				drawFlatTile(xOffset, yOffset, COLOR_MASK_GREEN);
			}
		}
		for (int i = 0; i < numTilesGround; i++) {
			int tileOffset = gridOffsetGround + tileGridOffsets[orientationIndex][i];
			int tileObstructed = 0;
			if (map_terrain_is(tileOffset, TERRAIN_NOT_CLEAR)) {
				tileObstructed = 1;
			}
			int xOffset = xOffsetGround + xViewOffsets[i];
			int yOffset = yOffsetGround + yViewOffsets[i];
			if (fullyObstructed || tileObstructed) {
				drawFlatTile(xOffset, yOffset, COLOR_MASK_RED);
			} else {
				drawFlatTile(xOffset, yOffset, COLOR_MASK_GREEN);
			}
		}
	} else {
		int graphicId = image_group(GROUP_BUILDING_FORT);
		if (orientationIndex == 0 || orientationIndex == 3) {
			// draw fort first
			Graphics_drawIsometricFootprint(graphicId, xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId, xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
			// then ground
			Graphics_drawIsometricFootprint(graphicId + 1, xOffsetGround, yOffsetGround, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId + 1, xOffsetGround, yOffsetGround, COLOR_MASK_GREEN);
		} else {
			// draw ground first
			Graphics_drawIsometricFootprint(graphicId + 1, xOffsetGround, yOffsetGround, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId + 1, xOffsetGround, yOffsetGround, COLOR_MASK_GREEN);
			// then fort
			Graphics_drawIsometricFootprint(graphicId, xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId, xOffsetBase, yOffsetBase, COLOR_MASK_GREEN);
		}
	}
}

static void drawBuildingGhostHippodrome()
{
	int fullyObstructed = 0;
	int placementObstructed = 0;
	if (Data_CityInfo.buildingHippodromePlaced || city_finance_out_of_money()) {
		fullyObstructed = 1;
		placementObstructed = 1;
	}
	int numTiles = 25;
	int orientationIndex = Data_State.map.orientation / 2;
	int gridOffset1 = Data_State.map.current.gridOffset;
	int gridOffset2 = gridOffset1 + map_grid_delta(5, 0);
	int gridOffset3 = gridOffset1 + map_grid_delta(10, 0);

	for (int i = 0; i < numTiles; i++) {
		int tileOffset = gridOffset1 + tileGridOffsets[orientationIndex][i];
		if (map_terrain_is(tileOffset, TERRAIN_NOT_CLEAR)) {
			placementObstructed = 1;
		}
	}
	for (int i = 0; i < numTiles; i++) {
		int tileOffset = gridOffset2 + tileGridOffsets[orientationIndex][i];
		if (map_terrain_is(tileOffset, TERRAIN_NOT_CLEAR)) {
			placementObstructed = 1;
		}
	}
	for (int i = 0; i < numTiles; i++) {
		int tileOffset = gridOffset3 + tileGridOffsets[orientationIndex][i];
		if (map_terrain_is(tileOffset, TERRAIN_NOT_CLEAR)) {
			placementObstructed = 1;
		}
	}

	int xOffsetBase1 = Data_CityView.selectedTile.xOffsetInPixels;
	int yOffsetBase1 = Data_CityView.selectedTile.yOffsetInPixels;
	int xOffsetBase2 = xOffsetBase1 + hippodromeXViewOffsets[orientationIndex];
	int yOffsetBase2 = yOffsetBase1 + hippodromeYViewOffsets[orientationIndex];
	int xOffsetBase3 = xOffsetBase2 + hippodromeXViewOffsets[orientationIndex];
	int yOffsetBase3 = yOffsetBase2 + hippodromeYViewOffsets[orientationIndex];
	if (placementObstructed) {
		for (int i = 0; i < numTiles; i++) {
			int tileOffset = gridOffset1 + tileGridOffsets[orientationIndex][i];
			int tileObstructed = 0;
			if (map_terrain_is(tileOffset, TERRAIN_NOT_CLEAR)) {
				tileObstructed = 1;
			}
			int xOffset = xOffsetBase1 + xViewOffsets[i];
			int yOffset = yOffsetBase1 + yViewOffsets[i];
			if (fullyObstructed || tileObstructed) {
				drawFlatTile(xOffset, yOffset, COLOR_MASK_RED);
			} else {
				drawFlatTile(xOffset, yOffset, COLOR_MASK_GREEN);
			}
		}
		for (int i = 0; i < numTiles; i++) {
			int tileOffset = gridOffset2 + tileGridOffsets[orientationIndex][i];
			int tileObstructed = 0;
			if (map_terrain_is(tileOffset, TERRAIN_NOT_CLEAR)) {
				tileObstructed = 1;
			}
			int xOffset = xOffsetBase2 + xViewOffsets[i];
			int yOffset = yOffsetBase2 + yViewOffsets[i];
			if (fullyObstructed || tileObstructed) {
				drawFlatTile(xOffset, yOffset, COLOR_MASK_RED);
			} else {
				drawFlatTile(xOffset, yOffset, COLOR_MASK_GREEN);
			}
		}
		for (int i = 0; i < numTiles; i++) {
			int tileOffset = gridOffset3 + tileGridOffsets[orientationIndex][i];
			int tileObstructed = 0;
			if (map_terrain_is(tileOffset, TERRAIN_NOT_CLEAR)) {
				tileObstructed = 1;
			}
			int xOffset = xOffsetBase3 + xViewOffsets[i];
			int yOffset = yOffsetBase3 + yViewOffsets[i];
			if (fullyObstructed || tileObstructed) {
				drawFlatTile(xOffset, yOffset, COLOR_MASK_RED);
			} else {
				drawFlatTile(xOffset, yOffset, COLOR_MASK_GREEN);
			}
		}
	} else {
		if (orientationIndex == 0) {
			int graphicId = image_group(GROUP_BUILDING_HIPPODROME_2);
			// part 1
			Graphics_drawIsometricFootprint(graphicId,
				xOffsetBase1, yOffsetBase1, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId,
				xOffsetBase1, yOffsetBase1, COLOR_MASK_GREEN);
			// part 2
			Graphics_drawIsometricFootprint(graphicId + 2,
				xOffsetBase2, yOffsetBase2, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId + 2,
				xOffsetBase2, yOffsetBase2, COLOR_MASK_GREEN);
			// part 3
			Graphics_drawIsometricFootprint(graphicId + 4,
				xOffsetBase3, yOffsetBase3, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId + 4,
				xOffsetBase3, yOffsetBase3, COLOR_MASK_GREEN);
		} else if (orientationIndex == 1) {
			int graphicId = image_group(GROUP_BUILDING_HIPPODROME_1);
			// part 3
			Graphics_drawIsometricFootprint(graphicId,
				xOffsetBase3, yOffsetBase3, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId,
				xOffsetBase3, yOffsetBase3, COLOR_MASK_GREEN);
			// part 2
			Graphics_drawIsometricFootprint(graphicId + 2,
				xOffsetBase2, yOffsetBase2, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId + 2,
				xOffsetBase2, yOffsetBase2, COLOR_MASK_GREEN);
			// part 1
			Graphics_drawIsometricFootprint(graphicId + 4,
				xOffsetBase1, yOffsetBase1, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId + 4,
				xOffsetBase1, yOffsetBase1, COLOR_MASK_GREEN);
		} else if (orientationIndex == 2) {
			int graphicId = image_group(GROUP_BUILDING_HIPPODROME_2);
			// part 1
			Graphics_drawIsometricFootprint(graphicId + 4,
				xOffsetBase1, yOffsetBase1, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId + 4,
				xOffsetBase1, yOffsetBase1, COLOR_MASK_GREEN);
			// part 2
			Graphics_drawIsometricFootprint(graphicId + 2,
				xOffsetBase2, yOffsetBase2, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId + 2,
				xOffsetBase2, yOffsetBase2, COLOR_MASK_GREEN);
			// part 3
			Graphics_drawIsometricFootprint(graphicId,
				xOffsetBase3, yOffsetBase3, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId,
				xOffsetBase3, yOffsetBase3, COLOR_MASK_GREEN);
		} else if (orientationIndex == 3) {
			int graphicId = image_group(GROUP_BUILDING_HIPPODROME_1);
			// part 3
			Graphics_drawIsometricFootprint(graphicId + 4,
				xOffsetBase3, yOffsetBase3, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId + 4,
				xOffsetBase3, yOffsetBase3, COLOR_MASK_GREEN);
			// part 2
			Graphics_drawIsometricFootprint(graphicId + 2,
				xOffsetBase2, yOffsetBase2, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId + 2,
				xOffsetBase2, yOffsetBase2, COLOR_MASK_GREEN);
			// part 1
			Graphics_drawIsometricFootprint(graphicId,
				xOffsetBase1, yOffsetBase1, COLOR_MASK_GREEN);
			Graphics_drawIsometricTop(graphicId,
				xOffsetBase1, yOffsetBase1, COLOR_MASK_GREEN);
		}
	}
}

static void drawBuildingGhostShipyardWharf()
{
	int dirAbsolute, dirRelative;
	int blockedTiles = Terrain_determineOrientationWatersideSize2(
		Data_State.map.current.x, Data_State.map.current.y, 1,
		&dirAbsolute, &dirRelative);
	if (city_finance_out_of_money()) {
		blockedTiles = 999;
	}
	if (blockedTiles) {
		for (int i = 0; i < 4; i++) {
			int xOffset = Data_CityView.selectedTile.xOffsetInPixels + xViewOffsets[i];
			int yOffset = Data_CityView.selectedTile.yOffsetInPixels + yViewOffsets[i];
			drawFlatTile(xOffset, yOffset, COLOR_MASK_RED);
		}
	} else {
		building_type type = Data_State.selectedBuilding.type;
        const building_properties *props = building_properties_for_type(type);
		int graphicId = image_group(props->image_group) + props->image_offset + dirRelative;
		int xOffset = Data_CityView.selectedTile.xOffsetInPixels;
		int yOffset = Data_CityView.selectedTile.yOffsetInPixels;
		Graphics_drawIsometricFootprint(graphicId, xOffset, yOffset, COLOR_MASK_GREEN);
		Graphics_drawIsometricTop(graphicId, xOffset, yOffset, COLOR_MASK_GREEN);
	}
}

static void drawBuildingGhostDock()
{
	int dirAbsolute, dirRelative;
	int blockedTiles = Terrain_determineOrientationWatersideSize3(
		Data_State.map.current.x, Data_State.map.current.y, 1,
		&dirAbsolute, &dirRelative);
	if (city_finance_out_of_money()) {
		blockedTiles = 999;
	}
	if (blockedTiles) {
		for (int i = 0; i < 9; i++) {
			int xOffset = Data_CityView.selectedTile.xOffsetInPixels + xViewOffsets[i];
			int yOffset = Data_CityView.selectedTile.yOffsetInPixels + yViewOffsets[i];
			drawFlatTile(xOffset, yOffset, COLOR_MASK_RED);
		}
	} else {
		int graphicId;
		switch (dirRelative) {
			case 0: graphicId = image_group(GROUP_BUILDING_DOCK_1); break;
			case 1: graphicId = image_group(GROUP_BUILDING_DOCK_2); break;
			case 2: graphicId = image_group(GROUP_BUILDING_DOCK_3); break;
			default: graphicId = image_group(GROUP_BUILDING_DOCK_4); break;
		}
		int xOffset = Data_CityView.selectedTile.xOffsetInPixels;
		int yOffset = Data_CityView.selectedTile.yOffsetInPixels;
		Graphics_drawIsometricFootprint(graphicId, xOffset, yOffset, COLOR_MASK_GREEN);
		Graphics_drawIsometricTop(graphicId, xOffset, yOffset, COLOR_MASK_GREEN);
	}
}

static void drawBuildingGhostRoad()
{
	int tileObstructed = 0;
	int xOffset = Data_CityView.selectedTile.xOffsetInPixels;
	int yOffset = Data_CityView.selectedTile.yOffsetInPixels;
	int gridOffset = Data_State.map.current.gridOffset;
	int graphicId;
	if (map_terrain_is(gridOffset, TERRAIN_AQUEDUCT)) {
		graphicId = image_group(GROUP_BUILDING_AQUEDUCT);
		if (map_can_place_road_under_aqueduct(gridOffset)) {
			graphicId += map_get_aqueduct_with_road_image(gridOffset);
		} else {
			tileObstructed = 1;
		}
	} else if (map_terrain_is(gridOffset, TERRAIN_NOT_CLEAR)) {
		tileObstructed = 1;
	} else {
		graphicId = image_group(GROUP_TERRAIN_ROAD);
		if (!Terrain_hasTerrainTypeSameXAdjacentTo(gridOffset, TERRAIN_ROAD) &&
			Terrain_hasTerrainTypeSameYAdjacentTo(gridOffset, TERRAIN_ROAD)) {
			graphicId++;
		}
	}
	if (city_finance_out_of_money()) {
		tileObstructed = 1;
	}
	if (tileObstructed) {
		drawFlatTile(xOffset, yOffset, COLOR_MASK_RED);
	} else {
		Graphics_drawIsometricFootprint(graphicId, xOffset, yOffset, COLOR_MASK_GREEN);
		Graphics_drawIsometricTop(graphicId, xOffset, yOffset, COLOR_MASK_GREEN);
	}
}

static void drawFlatTile(int xOffset, int yOffset, color_t mask)
{
	Graphics_drawImageBlend(image_group(GROUP_TERRAIN_FLAT_TILE), xOffset, yOffset, mask);
}
