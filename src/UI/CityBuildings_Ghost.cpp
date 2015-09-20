#include "CityBuildings_private.h"

#include "../BuildingPlacement.h"
#include "../Routing.h"
#include "../Terrain.h"
#include "../TerrainBridge.h"
#include "../TerrainGraphics.h"
#include "../Time.h"

#include "../Data/Formation.h"

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
static void drawFlatTile(int xOffset, int yOffset, Color mask);

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
	if (!Data_Settings_Map.current.gridOffset || Data_CityView.isScrolling) {
		return;
	}
	if (Data_State.selectedBuilding.drawAsOverlay || Data_State.selectedBuilding.type <= 0) {
		return;
	}
	switch (Data_State.selectedBuilding.type) {
		case Building_DraggableReservoir:
			drawBuildingGhostDraggableReservoir();
			break;
		case Building_Aqueduct:
			drawBuildingGhostAqueduct();
			break;
		case Building_Fountain:
			drawBuildingGhostFountain();
			break;
		case Building_Bathhouse:
			drawBuildingGhostBathhouse();
			break;
		case Building_LowBridge:
		case Building_ShipBridge:
			drawBuildingGhostBridge();
			break;
		case Building_FortLegionaries:
		case Building_FortJavelin:
		case Building_FortMounted:
			drawBuildingGhostFort();
			break;
		case Building_Hippodrome:
			drawBuildingGhostHippodrome();
			break;
		case Building_Shipyard:
		case Building_Wharf:
			drawBuildingGhostShipyardWharf();
			break;
		case Building_Dock:
			drawBuildingGhostDock();
			break;
		case Building_Road:
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
		if (Time_getMillis() > Data_State.selectedBuilding.roadLastUpdate + 1500) {
			Data_State.selectedBuilding.roadLastUpdate = Time_getMillis();
			Data_State.selectedBuilding.roadRequired = Data_State.selectedBuilding.roadRequired == 1 ? 2 : 1;
		}
	}
	int type = Data_State.selectedBuilding.type;
	int gridOffset = Data_Settings_Map.current.gridOffset;
	int buildingSize = Constant_BuildingProperties[type].size;
	int graphicId =
		GraphicId(Constant_BuildingProperties[type].graphicCategory) +
		Constant_BuildingProperties[type].graphicOffset;
	if (type == Building_Warehouse) {
		buildingSize = 3;
	}
	int xStart = 0, yStart = 0;
	// determine x and y offset
	switch (Data_Settings_Map.orientation) {
		case Dir_0_Top:
			xStart = Data_Settings_Map.current.x;
			yStart = Data_Settings_Map.current.y;
			break;
		case Dir_2_Right:
			xStart = Data_Settings_Map.current.x - buildingSize + 1;
			yStart = Data_Settings_Map.current.y;
			break;
		case Dir_4_Bottom:
			xStart = Data_Settings_Map.current.x - buildingSize + 1;
			yStart = Data_Settings_Map.current.y - buildingSize + 1;
			break;
		case Dir_6_Left:
			xStart = Data_Settings_Map.current.x;
			yStart = Data_Settings_Map.current.y - buildingSize + 1;
			break;
	}
	// check if we can place building
	if (Data_State.selectedBuilding.meadowRequired) {
		if (!Terrain_existsTileWithinRadiusWithType(xStart, yStart, 3, 1, Terrain_Meadow)) {
			fullyObstructed = 1;
			placementObstructed = 1;
		}
	} else if (Data_State.selectedBuilding.rockRequired) {
		if (!Terrain_existsTileWithinRadiusWithType(xStart, yStart, 2, 1, Terrain_Rock)) {
			fullyObstructed = 1;
			placementObstructed = 1;
		}
	} else if (Data_State.selectedBuilding.treesRequired) {
		if (!Terrain_existsTileWithinRadiusWithType(xStart, yStart, 2, 1, Terrain_Tree | Terrain_Scrub)) {
			fullyObstructed = 1;
			placementObstructed = 1;
		}
	} else if (Data_State.selectedBuilding.waterRequired) {
		if (!Terrain_existsTileWithinRadiusWithType(xStart, yStart, 2, 3, Terrain_Water)) {
			fullyObstructed = 1;
			placementObstructed = 1;
		}
	} else if (Data_State.selectedBuilding.wallRequired) {
		if (!Terrain_existsTileWithinRadiusWithType(xStart, yStart, 2, 0, Terrain_Wall)) {
			fullyObstructed = 1;
			placementObstructed = 1;
		}
	} else if (Data_State.selectedBuilding.type == Building_Gatehouse) {
		int orientation = Terrain_getOrientationGatehouse(
			Data_Settings_Map.current.x, Data_Settings_Map.current.y);
		int graphicOffset;
		if (orientation == 2) {
			graphicOffset = 1;
		} else if (orientation == 1) {
			graphicOffset = 0;
		} else {
			graphicOffset = Data_State.selectedBuilding.roadRequired == 2 ? 1 : 0;
		}
		if (Data_Settings_Map.orientation == Dir_6_Left ||
			Data_Settings_Map.orientation == Dir_2_Right) {
			graphicOffset = 1 - graphicOffset;
		}
		graphicId += graphicOffset;
	} else if (Data_State.selectedBuilding.type == Building_TriumphalArch) {
		int orientation = Terrain_getOrientationTriumphalArch(
			Data_Settings_Map.current.x, Data_Settings_Map.current.y);
		int graphicOffset;
		if (orientation == 2) {
			graphicOffset = 2;
		} else if (orientation == 1) {
			graphicOffset = 0;
		} else {
			graphicOffset = Data_State.selectedBuilding.roadRequired == 2 ? 2 : 0;
		}
		if (Data_Settings_Map.orientation == Dir_6_Left ||
			Data_Settings_Map.orientation == Dir_2_Right) {
			graphicOffset = 2 - graphicOffset;
		}
		graphicId += graphicOffset;
	}
	if (Data_State.selectedBuilding.type == Building_SenateUpgraded && Data_CityInfo.buildingSenatePlaced) {
		fullyObstructed = 1;
		placementObstructed = 1;
	}
	if (Data_State.selectedBuilding.type == Building_Barracks && Data_CityInfo_Buildings.barracks.total) {
		fullyObstructed = 1;
		placementObstructed = 1;
	}

	int numTiles = buildingSize * buildingSize;
	int orientationIndex = Data_Settings_Map.orientation / 2;
	for (int i = 0; i < numTiles; i++) {
		int tileOffset = gridOffset + tileGridOffsets[orientationIndex][i];
		int terrain = Data_Grid_terrain[tileOffset] & Terrain_NotClear;
		if (type == Building_Gatehouse || type == Building_TriumphalArch || type == Building_Plaza) {
			terrain &= ~Terrain_Road;
		}
		if (type == Building_Tower) {
			terrain &= ~Terrain_Wall;
		}
		if (terrain || Data_Grid_walkerIds[tileOffset]) {
			placementObstructed = 1;
		}
	}
	if (type == Building_Plaza && !(Data_Grid_terrain[gridOffset] & Terrain_Road)) {
		placementObstructed = 1;
	}
	if (Data_CityInfo.treasury <= MIN_TREASURY) {
		fullyObstructed = 1;
		placementObstructed = 1;
	}
	int xOffsetBase = Data_CityView.selectedTile.xOffsetInPixels;
	int yOffsetBase = Data_CityView.selectedTile.yOffsetInPixels;
	if (placementObstructed) {
		for (int i = 0; i < numTiles; i++) {
			int tileOffset = gridOffset + tileGridOffsets[orientationIndex][i];
			int tileObstructed = Data_Grid_terrain[tileOffset] & Terrain_NotClear;
			if (type == Building_Gatehouse) {
				tileObstructed &= ~Terrain_Road;
			}
			if (Data_Grid_walkerIds[tileOffset]) {
				tileObstructed = 1;
			}
			int xOffset = xOffsetBase + xViewOffsets[i];
			int yOffset = yOffsetBase + yViewOffsets[i];
			if (fullyObstructed || tileObstructed) {
				drawFlatTile(xOffset, yOffset, Color_MaskRed);
			} else {
				drawFlatTile(xOffset, yOffset, Color_MaskGreen);
			}
		}
	} else {
		// can place, draw ghost
		if (BuildingIsFarm(type)) {
			Graphics_drawIsometricFootprint(graphicId,
				xOffsetBase, yOffsetBase, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId,
				xOffsetBase, yOffsetBase, Color_MaskGreen);
			// fields
			for (int i = 4; i < 9; i++) {
				Graphics_drawIsometricFootprint(graphicId + 1,
					xOffsetBase + xViewOffsets[i],
					yOffsetBase + yViewOffsets[i], Color_MaskGreen);
			}
		} else if (type == Building_Warehouse) {
			Graphics_drawIsometricFootprint(graphicId,
				xOffsetBase, yOffsetBase, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId,
				xOffsetBase, yOffsetBase, Color_MaskGreen);
			Graphics_drawImageMasked(GraphicId(ID_Graphic_Warehouse) + 17,
				xOffsetBase - 4, yOffsetBase - 42, Color_MaskGreen);
			int graphicIdSpace = GraphicId(ID_Graphic_WarehouseStorageEmpty);
			for (int i = 1; i < 9; i++) {
				int xOffset = xOffsetBase + xViewOffsets[i];
				int yOffset = yOffsetBase + yViewOffsets[i];
				Graphics_drawIsometricFootprint(graphicIdSpace,
					xOffset, yOffset, Color_MaskGreen);
				Graphics_drawIsometricTop(graphicIdSpace,
					xOffset, yOffset, Color_MaskGreen);
			}
		} else if (type == Building_Granary) {
			Graphics_drawIsometricFootprint(graphicId,
				xOffsetBase, yOffsetBase, Color_MaskGreen);
			Graphics_drawImageMasked(graphicId + 1,
				xOffsetBase + GraphicSpriteOffsetX(graphicId + 1) - 32,
				yOffsetBase + GraphicSpriteOffsetY(graphicId + 1) - 64,
				Color_MaskGreen);
		} else if (type == Building_HouseVacantLot) {
			int graphicIdLot = GraphicId(ID_Graphic_HouseVacantLot);
			Graphics_drawIsometricFootprint(graphicIdLot,
				xOffsetBase, yOffsetBase, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicIdLot,
				xOffsetBase, yOffsetBase, Color_MaskGreen);
		} else if (type == Building_TriumphalArch) {
			Graphics_drawIsometricFootprint(graphicId,
				xOffsetBase, yOffsetBase, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId,
				xOffsetBase, yOffsetBase, Color_MaskGreen);
			if (graphicId == GraphicId(ID_Graphic_TriumphalArch)) {
				Graphics_drawImageMasked(graphicId + 1,
					xOffsetBase + GraphicSpriteOffsetX(graphicId + 1) + 4,
					yOffsetBase + GraphicSpriteOffsetY(graphicId + 1) - 51,
					Color_MaskGreen);
			} else {
				Graphics_drawImageMasked(graphicId + 1,
					xOffsetBase + GraphicSpriteOffsetX(graphicId + 1) - 33,
					yOffsetBase + GraphicSpriteOffsetY(graphicId + 1) - 56,
					Color_MaskGreen);
			}
		} else if (type != Building_ClearLand) {
			Graphics_drawIsometricFootprint(graphicId,
				xOffsetBase, yOffsetBase, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId,
				xOffsetBase, yOffsetBase, Color_MaskGreen);
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
		int gridOffset = GridOffset(Data_Settings_Map.current.x - 1, Data_Settings_Map.current.y - 1);
		if (Terrain_isReservoir(gridOffset)) {
			placementObstructed = 0;
		} else if (!Terrain_isClear(
				Data_Settings_Map.current.x - 1, Data_Settings_Map.current.y - 1,
				3, Terrain_All, 0)) {
			placementObstructed = 1;
		}
	}
	if (Data_CityInfo.treasury <= MIN_TREASURY) {
		placementObstructed = 1;
	}
	if (Data_State.selectedBuilding.placementInProgress) {
		int xOffsetBase = Data_State.selectedBuilding.reservoirOffsetX;
		int yOffsetBase = Data_State.selectedBuilding.reservoirOffsetY - 30;
		if (placementObstructed) {
			for (int i = 0; i < 9; i++) {
				int xOffset = xOffsetBase + xViewOffsets[i];
				int yOffset = yOffsetBase + yViewOffsets[i];
				drawFlatTile(xOffset, yOffset, Color_MaskRed);
			}
		} else {
			int graphicId = GraphicId(ID_Graphic_Reservoir);
			Graphics_drawIsometricFootprint(graphicId, xOffsetBase, yOffsetBase, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId, xOffsetBase, yOffsetBase, Color_MaskGreen);
		}
	}
	int xOffsetBase = Data_CityView.selectedTile.xOffsetInPixels;
	int yOffsetBase = Data_CityView.selectedTile.yOffsetInPixels - 30;
	if (placementObstructed) {
		for (int i = 0; i < 9; i++) {
			int xOffset = xOffsetBase + xViewOffsets[i];
			int yOffset = yOffsetBase + yViewOffsets[i];
			drawFlatTile(xOffset, yOffset, Color_MaskRed);
		}
	} else {
		int graphicId = GraphicId(ID_Graphic_Reservoir);
		Graphics_drawIsometricFootprint(graphicId, xOffsetBase, yOffsetBase, Color_MaskGreen);
		Graphics_drawIsometricTop(graphicId, xOffsetBase, yOffsetBase, Color_MaskGreen);
		if (Terrain_existsTileWithinAreaWithType(
			Data_Settings_Map.current.x - 2, Data_Settings_Map.current.y,
			5, Terrain_Water)) {
			Graphics_drawImageMasked(graphicId + 1,
				xOffsetBase - 58 + GraphicSpriteOffsetX(graphicId) - 2,
				yOffsetBase + GraphicSpriteOffsetY(graphicId) - (GraphicHeight(graphicId) - 90),
				Color_MaskGreen);
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
		int gridOffset = GridOffset(Data_Settings_Map.current.x, Data_Settings_Map.current.y);
		if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
			placementObstructed = Terrain_getAdjacentRoadTilesForAqueduct(gridOffset) == 2 ? 0 : 1;
		} else if (Data_Grid_terrain[gridOffset] & Terrain_NotClear) {
			placementObstructed = 1;
		}
	}
	if (Data_CityInfo.treasury <= MIN_TREASURY) {
		placementObstructed = 1;
	}
	int xOffset = Data_CityView.selectedTile.xOffsetInPixels;
	int yOffset = Data_CityView.selectedTile.yOffsetInPixels;
	if (placementObstructed) {
		drawFlatTile(xOffset, yOffset, Color_MaskRed);
	} else {
		int gridOffset = Data_Settings_Map.current.gridOffset;
		int graphicId = GraphicId(ID_Graphic_Aqueduct);
		const struct TerrainGraphic *graphic = TerrainGraphicsContext_getAqueduct(gridOffset, 0);
		if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
			int groupOffset = graphic->groupOffset;
			if (!graphic->aqueductOffset) {
				if (Data_Grid_terrain[gridOffset - 162] & Terrain_Road) {
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
		Graphics_drawIsometricFootprint(graphicId, xOffset, yOffset, Color_MaskGreen);
		Graphics_drawIsometricTop(graphicId, xOffset, yOffset, Color_MaskGreen);
	}
}

static void drawBuildingGhostFountain()
{
	int gridOffset = Data_Settings_Map.current.gridOffset;
	int xOffset = Data_CityView.selectedTile.xOffsetInPixels;
	int yOffset = Data_CityView.selectedTile.yOffsetInPixels;

	int graphicId = GraphicId(Constant_BuildingProperties[Building_Fountain].graphicCategory);
	if (Data_CityInfo.treasury <= MIN_TREASURY) {
		drawFlatTile(xOffset, yOffset, Color_MaskRed);
	} else {
		Graphics_drawIsometricFootprint(graphicId, xOffset, yOffset, Color_MaskGreen);
		Graphics_drawIsometricTop(graphicId, xOffset, yOffset, Color_MaskGreen);
		if (Data_Grid_terrain[gridOffset] & Terrain_ReservoirRange) {
			Graphics_drawImageMasked(graphicId + 1,
				xOffset + GraphicSpriteOffsetX(graphicId),
				yOffset + GraphicSpriteOffsetY(graphicId), Color_MaskGreen);
		}
	}
}

static void drawBuildingGhostBathhouse()
{
	int fullyObstructed = 0;
	int placementObstructed = 0;
	int gridOffset = Data_Settings_Map.current.gridOffset;
	int numTiles = 4;
	int orientationIndex = Data_Settings_Map.orientation / 2;
	for (int i = 0; i < numTiles; i++) {
		int tileOffset = gridOffset + tileGridOffsets[orientationIndex][i];
		if (Data_Grid_terrain[tileOffset] & Terrain_NotClear) {
			placementObstructed = 1;
		}
		if (Data_Grid_walkerIds[tileOffset]) {
			placementObstructed = 1;
		}
	}
	if (Data_CityInfo.treasury <= MIN_TREASURY) {
		fullyObstructed = 1;
		placementObstructed = 1;
	}

	int xOffsetBase = Data_CityView.selectedTile.xOffsetInPixels;
	int yOffsetBase = Data_CityView.selectedTile.yOffsetInPixels;
	if (placementObstructed) {
		for (int i = 0; i < numTiles; i++) {
			int tileOffset = gridOffset + tileGridOffsets[orientationIndex][i];
			int tileObstructed = Data_Grid_terrain[tileOffset] & Terrain_NotClear;
			if (Data_Grid_walkerIds[tileOffset]) {
				tileObstructed = 1;
			}
			int xOffset = xOffsetBase + xViewOffsets[i];
			int yOffset = yOffsetBase + yViewOffsets[i];
			if (fullyObstructed || tileObstructed) {
				drawFlatTile(xOffset, yOffset, Color_MaskRed);
			} else {
				drawFlatTile(xOffset, yOffset, Color_MaskGreen);
			}
		}
	} else {
		int graphicId = GraphicId(Constant_BuildingProperties[Building_Bathhouse].graphicCategory);
		Graphics_drawIsometricFootprint(graphicId, xOffsetBase, yOffsetBase, Color_MaskGreen);
		Graphics_drawIsometricTop(graphicId, xOffsetBase, yOffsetBase, Color_MaskGreen);
		int hasWater = 0;
		for (int i = 0; i < numTiles; i++) { // FIXED: was not accurate on rotated maps
			int tileOffset = gridOffset + tileGridOffsets[orientationIndex][i];
			if (Data_Grid_terrain[tileOffset] & Terrain_ReservoirRange) {
				hasWater = 1;
			}
		}
		if (hasWater) {
			Graphics_drawImageMasked(graphicId - 1,
				xOffsetBase + GraphicSpriteOffsetX(graphicId),
				yOffsetBase + GraphicSpriteOffsetY(graphicId),
				Color_MaskGreen);
		}
	}
}

static void drawBuildingGhostBridge()
{
	int length, direction;
	int endGridOffset = TerrainBridge_determineLengthAndDirection(
		Data_Settings_Map.current.x, Data_Settings_Map.current.y,
		Data_State.selectedBuilding.type == Building_LowBridge ? 0 : 1,
		&length, &direction);

	int dir = direction - Data_Settings_Map.orientation;
	if (dir < 0) {
		dir += 8;
	}
	int obstructed = 0;
	if (Data_State.selectedBuilding.type == Building_ShipBridge && length < 5) {
		obstructed = 1;
	} else if (!endGridOffset) {
		obstructed = 1;
	}
	if (Data_CityInfo.treasury <= MIN_TREASURY) {
		obstructed = 1;
	}
	if (obstructed) {
		int xOffset = Data_CityView.selectedTile.xOffsetInPixels;
		int yOffset = Data_CityView.selectedTile.yOffsetInPixels;
		drawFlatTile(xOffset, yOffset, length > 0 ? Color_MaskGreen : Color_MaskRed);
		if (length > 1) {
			switch (dir) {
				case 0:
					xOffset += 29 * (length - 1);
					yOffset -= 15 * (length - 1);
					break;
				case 2:
					xOffset += 29 * (length - 1);
					yOffset += 15 * (length - 1);
					break;
				case 4:
					xOffset -= 29 * (length - 1);
					yOffset += 15 * (length - 1);
					break;
				case 6:
					xOffset -= 29 * (length - 1);
					yOffset -= 15 * (length - 1);
					break;
			}
			drawFlatTile(xOffset, yOffset, Color_MaskRed);
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
	int graphicBase = GraphicId(ID_Graphic_Bridge);
	int graphicId, xOffset, yOffset;
	int xAdd, yAdd;
	switch (dir) {
		case 0: xAdd = 29; yAdd = -15; break;
		case 2: xAdd = 29; yAdd = 15; break;
		case 4: xAdd = -29; yAdd = 15; break;
		case 6: xAdd = -29; yAdd = -15; break;
	}
	if (Data_State.selectedBuilding.type == Building_LowBridge) {
		switch (dir) {
			case 0:
				graphicId = graphicBase + 5;
				xOffset = xOffsetBase;
				yOffset = yOffsetBase - 20;
				break;
			case 2:
				graphicId = graphicBase;
				xOffset = xOffsetBase - 1;
				yOffset = yOffsetBase - 8;
				break;
			case 4:
				graphicId = graphicBase + 3;
				xOffset = xOffsetBase;
				yOffset = yOffsetBase - 8;
				break;
			case 6:
				graphicId = graphicBase + 2;
				xOffset = xOffsetBase + 7;
				yOffset = yOffsetBase - 20;
				break;
		}
		Graphics_drawImageMasked(graphicId, xOffset, yOffset, Color_MaskGreen);
		tiles[numTiles].xOffset = xOffset;
		tiles[numTiles].yOffset = yOffset;
		tiles[numTiles++].graphicId = graphicId;
		for (int i = 1; i < length; i++) {
			xOffsetBase += xAdd;
			yOffsetBase += yAdd;
			if (i == length - 1) {
				switch (dir) {
					case 0:
						graphicId = graphicBase + 3;
						xOffset = xOffsetBase;
						yOffset = yOffsetBase - 8;
						break;
					case 2:
						graphicId = graphicBase + 2;
						xOffset = xOffsetBase + 7;
						yOffset = yOffsetBase - 20;
						break;
					case 4:
						graphicId = graphicBase + 5;
						xOffset = xOffsetBase;
						yOffset = yOffsetBase - 20;
						break;
					case 6:
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
			Graphics_drawImageMasked(graphicId, xOffset, yOffset, Color_MaskGreen);
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
		Graphics_drawImageMasked(graphicId, xOffset, yOffset, Color_MaskGreen);
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
			Graphics_drawImageMasked(graphicId, xOffset, yOffset, Color_MaskGreen);
			tiles[numTiles].xOffset = xOffset;
			tiles[numTiles].yOffset = yOffset;
			tiles[numTiles++].graphicId = graphicId;
		}
	}
	if (dir == 0 || dir == 6) {
		// draw in opposite order
		for (int i = numTiles - 1; i >= 0; i--) {
			Graphics_drawImageMasked(tiles[i].graphicId,
				tiles[i].xOffset, tiles[i].yOffset, Color_MaskGreen);
		}
	}
}

static void drawBuildingGhostFort()
{
	int fullyObstructed = 0;
	int placementObstructed = 0;
	if (Data_Formation_Extra.numForts >= 6 || Data_CityInfo.treasury <= MIN_TREASURY) {
		fullyObstructed = 1;
		placementObstructed = 1;
	}

	int numTilesFort = Constant_BuildingProperties[Building_Fort].size;
	numTilesFort *= numTilesFort;
	int numTilesGround = Constant_BuildingProperties[Building_FortGround].size;
	numTilesGround *= numTilesGround;

	int orientationIndex = Data_Settings_Map.orientation / 2;
	int gridOffsetFort = Data_Settings_Map.current.gridOffset;
	int gridOffsetGround = gridOffsetFort + fortGroundGridOffsets[orientationIndex];

	for (int i = 0; i < numTilesFort; i++) {
		int tileOffset = gridOffsetFort + tileGridOffsets[orientationIndex][i];
		if (Data_Grid_terrain[tileOffset] & Terrain_NotClear) {
			placementObstructed = 1;
		}
	}
	for (int i = 0; i < numTilesGround; i++) {
		int tileOffset = gridOffsetGround + tileGridOffsets[orientationIndex][i];
		if (Data_Grid_terrain[tileOffset] & Terrain_NotClear) {
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
			if (Data_Grid_terrain[tileOffset] & Terrain_NotClear) {
				tileObstructed = 1;
			}
			int xOffset = xOffsetBase + xViewOffsets[i];
			int yOffset = yOffsetBase + yViewOffsets[i];
			if (fullyObstructed || tileObstructed) {
				drawFlatTile(xOffset, yOffset, Color_MaskRed);
			} else {
				drawFlatTile(xOffset, yOffset, Color_MaskGreen);
			}
		}
		for (int i = 0; i < numTilesGround; i++) {
			int tileOffset = gridOffsetGround + tileGridOffsets[orientationIndex][i];
			int tileObstructed = 0;
			if (Data_Grid_terrain[tileOffset] & Terrain_NotClear) {
				tileObstructed = 1;
			}
			int xOffset = xOffsetGround + xViewOffsets[i];
			int yOffset = yOffsetGround + yViewOffsets[i];
			if (fullyObstructed || tileObstructed) {
				drawFlatTile(xOffset, yOffset, Color_MaskRed);
			} else {
				drawFlatTile(xOffset, yOffset, Color_MaskGreen);
			}
		}
	} else {
		int graphicId = GraphicId(ID_Graphic_Fort);
		if (orientationIndex == 0 || orientationIndex == 3) {
			// draw fort first
			Graphics_drawIsometricFootprint(graphicId, xOffsetBase, yOffsetBase, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId, xOffsetBase, yOffsetBase, Color_MaskGreen);
			// then ground
			Graphics_drawIsometricFootprint(graphicId + 1, xOffsetGround, yOffsetGround, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId + 1, xOffsetGround, yOffsetGround, Color_MaskGreen);
		} else {
			// draw ground first
			Graphics_drawIsometricFootprint(graphicId + 1, xOffsetGround, yOffsetGround, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId + 1, xOffsetGround, yOffsetGround, Color_MaskGreen);
			// then fort
			Graphics_drawIsometricFootprint(graphicId, xOffsetBase, yOffsetBase, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId, xOffsetBase, yOffsetBase, Color_MaskGreen);
		}
	}
}

static void drawBuildingGhostHippodrome()
{
	int fullyObstructed = 0;
	int placementObstructed = 0;
	if (Data_CityInfo.buildingHippodromePlaced || Data_CityInfo.treasury <= MIN_TREASURY) {
		fullyObstructed = 1;
		placementObstructed = 1;
	}
	int numTiles = 25;
	int orientationIndex = Data_Settings_Map.orientation / 2;
	int gridOffset1 = Data_Settings_Map.current.gridOffset;
	int gridOffset2 = gridOffset1 + 5;
	int gridOffset3 = gridOffset1 + 10;

	for (int i = 0; i < numTiles; i++) {
		int tileOffset = gridOffset1 + tileGridOffsets[orientationIndex][i];
		if (Data_Grid_terrain[tileOffset] & Terrain_NotClear) {
			placementObstructed = 1;
		}
	}
	for (int i = 0; i < numTiles; i++) {
		int tileOffset = gridOffset2 + tileGridOffsets[orientationIndex][i];
		if (Data_Grid_terrain[tileOffset] & Terrain_NotClear) {
			placementObstructed = 1;
		}
	}
	for (int i = 0; i < numTiles; i++) {
		int tileOffset = gridOffset3 + tileGridOffsets[orientationIndex][i];
		if (Data_Grid_terrain[tileOffset] & Terrain_NotClear) {
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
			if (Data_Grid_terrain[tileOffset] & Terrain_NotClear) {
				tileObstructed = 1;
			}
			int xOffset = xOffsetBase1 + xViewOffsets[i];
			int yOffset = yOffsetBase1 + yViewOffsets[i];
			if (fullyObstructed || tileObstructed) {
				drawFlatTile(xOffset, yOffset, Color_MaskRed);
			} else {
				drawFlatTile(xOffset, yOffset, Color_MaskGreen);
			}
		}
		for (int i = 0; i < numTiles; i++) {
			int tileOffset = gridOffset2 + tileGridOffsets[orientationIndex][i];
			int tileObstructed = 0;
			if (Data_Grid_terrain[tileOffset] & Terrain_NotClear) {
				tileObstructed = 1;
			}
			int xOffset = xOffsetBase2 + xViewOffsets[i];
			int yOffset = yOffsetBase2 + yViewOffsets[i];
			if (fullyObstructed || tileObstructed) {
				drawFlatTile(xOffset, yOffset, Color_MaskRed);
			} else {
				drawFlatTile(xOffset, yOffset, Color_MaskGreen);
			}
		}
		for (int i = 0; i < numTiles; i++) {
			int tileOffset = gridOffset3 + tileGridOffsets[orientationIndex][i];
			int tileObstructed = 0;
			if (Data_Grid_terrain[tileOffset] & Terrain_NotClear) {
				tileObstructed = 1;
			}
			int xOffset = xOffsetBase3 + xViewOffsets[i];
			int yOffset = yOffsetBase3 + yViewOffsets[i];
			if (fullyObstructed || tileObstructed) {
				drawFlatTile(xOffset, yOffset, Color_MaskRed);
			} else {
				drawFlatTile(xOffset, yOffset, Color_MaskGreen);
			}
		}
	} else {
		if (orientationIndex == 0) {
			int graphicId = GraphicId(ID_Graphic_Hippodrome2);
			// part 1
			Graphics_drawIsometricFootprint(graphicId,
				xOffsetBase1, yOffsetBase1, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId,
				xOffsetBase1, yOffsetBase1, Color_MaskGreen);
			// part 2
			Graphics_drawIsometricFootprint(graphicId + 2,
				xOffsetBase2, yOffsetBase2, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId + 2,
				xOffsetBase2, yOffsetBase2, Color_MaskGreen);
			// part 3
			Graphics_drawIsometricFootprint(graphicId + 4,
				xOffsetBase3, yOffsetBase3, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId + 4,
				xOffsetBase3, yOffsetBase3, Color_MaskGreen);
		} else if (orientationIndex == 1) {
			int graphicId = GraphicId(ID_Graphic_Hippodrome1);
			// part 3
			Graphics_drawIsometricFootprint(graphicId,
				xOffsetBase3, yOffsetBase3, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId,
				xOffsetBase3, yOffsetBase3, Color_MaskGreen);
			// part 2
			Graphics_drawIsometricFootprint(graphicId + 2,
				xOffsetBase2, yOffsetBase2, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId + 2,
				xOffsetBase2, yOffsetBase2, Color_MaskGreen);
			// part 1
			Graphics_drawIsometricFootprint(graphicId + 4,
				xOffsetBase1, yOffsetBase1, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId + 4,
				xOffsetBase1, yOffsetBase1, Color_MaskGreen);
		} else if (orientationIndex == 2) {
			int graphicId = GraphicId(ID_Graphic_Hippodrome2);
			// part 1
			Graphics_drawIsometricFootprint(graphicId + 4,
				xOffsetBase1, yOffsetBase1, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId + 4,
				xOffsetBase1, yOffsetBase1, Color_MaskGreen);
			// part 2
			Graphics_drawIsometricFootprint(graphicId + 2,
				xOffsetBase2, yOffsetBase2, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId + 2,
				xOffsetBase2, yOffsetBase2, Color_MaskGreen);
			// part 3
			Graphics_drawIsometricFootprint(graphicId,
				xOffsetBase3, yOffsetBase3, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId,
				xOffsetBase3, yOffsetBase3, Color_MaskGreen);
		} else if (orientationIndex == 3) {
			int graphicId = GraphicId(ID_Graphic_Hippodrome1);
			// part 3
			Graphics_drawIsometricFootprint(graphicId + 4,
				xOffsetBase3, yOffsetBase3, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId + 4,
				xOffsetBase3, yOffsetBase3, Color_MaskGreen);
			// part 2
			Graphics_drawIsometricFootprint(graphicId + 2,
				xOffsetBase2, yOffsetBase2, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId + 2,
				xOffsetBase2, yOffsetBase2, Color_MaskGreen);
			// part 1
			Graphics_drawIsometricFootprint(graphicId,
				xOffsetBase1, yOffsetBase1, Color_MaskGreen);
			Graphics_drawIsometricTop(graphicId,
				xOffsetBase1, yOffsetBase1, Color_MaskGreen);
		}
	}
}

static void drawBuildingGhostShipyardWharf()
{
	int dirAbsolute, dirRelative;
	int blockedTiles = Terrain_determineOrientationWatersideSize2(
		Data_Settings_Map.current.x, Data_Settings_Map.current.y, 1,
		&dirAbsolute, &dirRelative);
	if (Data_CityInfo.treasury <= MIN_TREASURY) {
		blockedTiles = 999;
	}
	if (blockedTiles) {
		for (int i = 0; i < 4; i++) {
			int xOffset = Data_CityView.selectedTile.xOffsetInPixels + xViewOffsets[i];
			int yOffset = Data_CityView.selectedTile.yOffsetInPixels + yViewOffsets[i];
			drawFlatTile(xOffset, yOffset, Color_MaskRed);
		}
	} else {
		int type = Data_State.selectedBuilding.type;
		int graphicId = GraphicId(Constant_BuildingProperties[type].graphicCategory) +
			Constant_BuildingProperties[type].graphicOffset + dirRelative;
		int xOffset = Data_CityView.selectedTile.xOffsetInPixels;
		int yOffset = Data_CityView.selectedTile.yOffsetInPixels;
		Graphics_drawIsometricFootprint(graphicId, xOffset, yOffset, Color_MaskGreen);
		Graphics_drawIsometricTop(graphicId, xOffset, yOffset, Color_MaskGreen);
	}
}

static void drawBuildingGhostDock()
{
	int dirAbsolute, dirRelative;
	int blockedTiles = Terrain_determineOrientationWatersideSize3(
		Data_Settings_Map.current.x, Data_Settings_Map.current.y, 1,
		&dirAbsolute, &dirRelative);
	if (Data_CityInfo.treasury <= MIN_TREASURY) {
		blockedTiles = 999;
	}
	if (blockedTiles) {
		for (int i = 0; i < 9; i++) {
			int xOffset = Data_CityView.selectedTile.xOffsetInPixels + xViewOffsets[i];
			int yOffset = Data_CityView.selectedTile.yOffsetInPixels + yViewOffsets[i];
			drawFlatTile(xOffset, yOffset, Color_MaskRed);
		}
	} else {
		int graphicId;
		switch (dirRelative) {
			case 0: graphicId = GraphicId(ID_Graphic_Dock1); break;
			case 1: graphicId = GraphicId(ID_Graphic_Dock2); break;
			case 2: graphicId = GraphicId(ID_Graphic_Dock3); break;
			default: graphicId = GraphicId(ID_Graphic_Dock4); break;
		}
		int xOffset = Data_CityView.selectedTile.xOffsetInPixels;
		int yOffset = Data_CityView.selectedTile.yOffsetInPixels;
		Graphics_drawIsometricFootprint(graphicId, xOffset, yOffset, Color_MaskGreen);
		Graphics_drawIsometricTop(graphicId, xOffset, yOffset, Color_MaskGreen);
	}
}

static void drawBuildingGhostRoad()
{
	int tileObstructed = 0;
	int xOffset = Data_CityView.selectedTile.xOffsetInPixels;
	int yOffset = Data_CityView.selectedTile.yOffsetInPixels;
	int gridOffset = Data_Settings_Map.current.gridOffset;
	int graphicId;
	if (Data_Grid_terrain[gridOffset] & Terrain_Aqueduct) {
		graphicId = GraphicId(ID_Graphic_Aqueduct);
		if (Routing_canPlaceRoadUnderAqueduct(gridOffset)) {
			graphicId += Routing_getAqueductGraphicOffsetWithRoad(gridOffset);
		} else {
			tileObstructed = 1;
		}
	} else if (Data_Grid_terrain[gridOffset] & Terrain_NotClear) {
		tileObstructed = 1;
	} else {
		graphicId = GraphicId(ID_Graphic_Road);
		if (!Terrain_hasTerrainTypeSameXAdjacentTo(gridOffset, Terrain_Road) &&
			Terrain_hasTerrainTypeSameYAdjacentTo(gridOffset, Terrain_Road)) {
			graphicId++;
		}
	}
	if (Data_CityInfo.treasury <= MIN_TREASURY) {
		tileObstructed = 1;
	}
	if (tileObstructed) {
		drawFlatTile(xOffset, yOffset, Color_MaskRed);
	} else {
		Graphics_drawIsometricFootprint(graphicId, xOffset, yOffset, Color_MaskGreen);
		Graphics_drawIsometricTop(graphicId, xOffset, yOffset, Color_MaskGreen);
	}
}

static void drawFlatTile(int xOffset, int yOffset, Color mask)
{
	Graphics_drawImageBlend(GraphicId(ID_Graphic_FlatTile), xOffset, yOffset, mask);
}
