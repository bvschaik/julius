#include "CityBuildings_private.h"
//#include "../Data/Constants.h"

static void drawBuildingGhostDraggableReservoir();
static void drawBuildingGhostAqueduct();
static void drawBuildingGhostFountain();
static void drawBuildingGhostBathhouse();
static void drawBuildingGhostBridge();
static void drawBuildingGhostFort();
static void drawBuildingGhostShipyardWharf();
static void drawBuildingGhostDock();
static void drawBuildingGhostRoad();
static void drawBuildingGhostDefault();
static void drawFlatTile(int xOffset, int yOffset, Color mask);

void drawSelectedBuildingGhost()
{
	if (!Data_CityView.selectedTile.gridOffset || Data_CityView.isScrolling) {
		return;
	}
	// TODO if (!dword_9DA7C8) return;
	if (Data_State.selectedBuilding.type <= 0) {
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
	// TODO
}

static void drawBuildingGhostDraggableReservoir()
{
	// TODO
}

static void drawBuildingGhostAqueduct()
{
	// TODO
}

static void drawBuildingGhostFountain()
{
	int gridOffset = Data_CityView.selectedTile.gridOffset;
	int xOffset = Data_CityView.selectedTile.xOffsetInPixels;
	int yOffset = Data_CityView.selectedTile.yOffsetInPixels;

	int graphicId = GraphicId(Constant_BuildingProperties[Building_Fountain].graphicCategory);
	if (Data_CityInfo.treasury <= -5000) {
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
	// TODO
}

static void drawBuildingGhostBridge()
{
	// TODO
}

static void drawBuildingGhostFort()
{
	// TODO
}

static void drawBuildingGhostShipyardWharf()
{
	// TODO
}

static void drawBuildingGhostDock()
{
	// TODO
}

static void drawBuildingGhostRoad()
{
	// TODO
}

static void drawFlatTile(int xOffset, int yOffset, Color mask)
{
	Graphics_drawIsometricFootprint(GraphicId(ID_Graphic_FlatTile), xOffset, yOffset, mask);
}
