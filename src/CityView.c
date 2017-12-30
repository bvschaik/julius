#include "CityView.h"

#include "Data/CityView.h"
#include "Data/Screen.h"
#include "Data/State.h"

#include "core/direction.h"
#include "map/grid.h"
#include "map/image.h"

static void setViewport(int xOffset, int yOffset, int widthInTiles, int heightInTiles);

void CityView_setViewport()
{
	if (Data_State.sidebarCollapsed) {
		CityView_setViewportWithoutSidebar();
	} else {
		CityView_setViewportWithSidebar();
	}
}

void CityView_setViewportWithoutSidebar()
{
	setViewport(0, 24,
		(Data_Screen.width - 40) / 60,
		(Data_Screen.height - 24) / 15);
}

void CityView_setViewportWithSidebar()
{
	setViewport(0, 24,
		(Data_Screen.width - 160) / 60,
		(Data_Screen.height - 24) / 15);
}

static void setViewport(int xOffset, int yOffset, int widthInTiles, int heightInTiles)
{
	Data_CityView.xOffsetInPixels = xOffset;
	Data_CityView.yOffsetInPixels = yOffset;
	Data_CityView.widthInPixels = widthInTiles * 60 - 2;
	Data_CityView.heightInPixels = heightInTiles * 15;
	Data_CityView.widthInTiles = widthInTiles;
	Data_CityView.heightInTiles = heightInTiles;
	Data_CityView.xInTiles = GRID_SIZE / 2;
	Data_CityView.yInTiles = GRID_SIZE;
}

static void resetLookup()
{
	for (int y = 0; y < VIEW_Y_MAX; y++) {
		for (int x = 0; x < VIEW_X_MAX; x++) {
			ViewToGridOffset(x,y) = -1;
		}
	}
}

void CityView_calculateLookup()
{
	resetLookup();
	int yViewStart;
	int yViewSkip;
	int yViewStep;
	int xViewStart;
	int xViewSkip;
	int xViewStep;
	switch (Data_State.map.orientation) {
		default:
		case DIR_0_TOP:
			xViewStart = VIEW_X_MAX - 1;
			xViewSkip = -1;
			xViewStep = 1;
			yViewStart = 1;
			yViewSkip = 1;
			yViewStep = 1;
			break;
		case DIR_2_RIGHT:
			xViewStart = 1;
			xViewSkip = 1;
			xViewStep = 1;
			yViewStart = VIEW_X_MAX - 1;
			yViewSkip = 1;
			yViewStep = -1;
			break;
		case DIR_4_BOTTOM:
			xViewStart = VIEW_X_MAX - 1;
			xViewSkip = 1;
			xViewStep = -1;
			yViewStart = VIEW_Y_MAX - 2;
			yViewSkip = -1;
			yViewStep = -1;
			break;
		case DIR_6_LEFT:
			xViewStart = VIEW_Y_MAX - 2;
			xViewSkip = -1;
			xViewStep = -1;
			yViewStart = VIEW_X_MAX - 1;
			yViewSkip = -1;
			yViewStep = 1;
			break;
	}

	for (int y = 0; y < GRID_SIZE; y++) {
		int xView = xViewStart;
		int yView = yViewStart;
		for (int x = 0; x < GRID_SIZE; x++) {
			int gridOffset = x + GRID_SIZE * y;
			if (map_image_at(gridOffset) < 6) {
				ViewToGridOffset(xView/2, yView) = -1;
			} else {
				ViewToGridOffset(xView/2, yView) = gridOffset;
			}
			xView += xViewStep;
			yView += yViewStep;
		}
		xViewStart += xViewSkip;
		yViewStart += yViewSkip;
	}
}

void CityView_goToGridOffset(int gridOffset)
{
	int x, y;
	CityView_gridOffsetToXYCoords(gridOffset, &x, &y);
	Data_State.map.camera.x = x - Data_CityView.widthInTiles / 2;
	Data_State.map.camera.y = y - Data_CityView.heightInTiles / 2;
	Data_State.map.camera.y &= ~1;
	CityView_checkCameraBoundaries();
}

void CityView_checkCameraBoundaries()
{
	int xMin = (165 - Data_State.map.width) / 2;
	int yMin = (323 - 2 * Data_State.map.height) / 2;
	if (Data_State.map.camera.x < xMin - 1) {
		Data_State.map.camera.x = xMin - 1;
	}
	if (Data_State.map.camera.x > 165 - xMin - Data_CityView.widthInTiles) {
		Data_State.map.camera.x = 165 - xMin - Data_CityView.widthInTiles;
	}
	if (Data_State.map.camera.y < yMin) {
		Data_State.map.camera.y = yMin;
	}
	if (Data_State.map.camera.y > 327 - yMin - Data_CityView.heightInTiles) {
		Data_State.map.camera.y = 327 - yMin - Data_CityView.heightInTiles;
	}
	Data_State.map.camera.y &= ~1;
}

void CityView_gridOffsetToXYCoords(int gridOffset, int *xInTiles, int *yInTiles)
{
	*xInTiles = *yInTiles = 0;
	for (int y = 0; y < VIEW_Y_MAX; y++) {
		for (int x = 0; x < VIEW_X_MAX; x++) {
			if (ViewToGridOffset(x,y) == gridOffset) {
				*xInTiles = x;
				*yInTiles = y;
				return;
			}
		}
	}
}

int CityView_pixelCoordsToGridOffset(int xPixels, int yPixels)
{
	if (xPixels < Data_CityView.xOffsetInPixels ||
		xPixels >= Data_CityView.xOffsetInPixels + Data_CityView.widthInPixels ||
		yPixels < Data_CityView.yOffsetInPixels ||
		yPixels >= Data_CityView.yOffsetInPixels + Data_CityView.heightInPixels) {
		return 0;
	}

	int odd = ((xPixels - Data_CityView.xOffsetInPixels) / 30 + (yPixels - Data_CityView.yOffsetInPixels) / 15) & 1;
	int xOdd = ((xPixels - Data_CityView.xOffsetInPixels) / 30) & 1;
	int yOdd = ((yPixels - Data_CityView.yOffsetInPixels) / 15) & 1;
	int xMod = ((xPixels - Data_CityView.xOffsetInPixels) % 30) / 2;
	int yMod = (yPixels - Data_CityView.yOffsetInPixels) % 15;
	int xViewOffset = (xPixels - Data_CityView.xOffsetInPixels) / 60;
	int yViewOffset = (yPixels - Data_CityView.yOffsetInPixels) / 15;
	if (odd) {
		if (xMod + yMod >= 15 - 1) {
			yViewOffset++;
			if (xOdd && !yOdd) {
				xViewOffset++;
			}
		}
	} else {
		if (yMod > xMod) {
			yViewOffset++;
		} else if (xOdd && yOdd) {
			xViewOffset++;
		}
	}
	Data_CityView.selectedTile.xOffsetInPixels =
		Data_CityView.xOffsetInPixels + 60 * xViewOffset;
	if (yViewOffset & 1) {
		Data_CityView.selectedTile.xOffsetInPixels -= 30;
	}
	Data_CityView.selectedTile.yOffsetInPixels =
		Data_CityView.yOffsetInPixels + 15 * yViewOffset - 15; // TODO why -1?
	int gridOffset = ViewToGridOffset(Data_CityView.xInTiles + xViewOffset, Data_CityView.yInTiles + yViewOffset);
	return gridOffset < 0 ? 0 : gridOffset;
}

void CityView_rotateLeft()
{
	int centerGridOffset = ViewToGridOffset(
		Data_State.map.camera.x + Data_CityView.widthInTiles / 2,
		Data_State.map.camera.y + Data_CityView.heightInTiles / 2);

	Data_State.map.orientation += 2;
	if (Data_State.map.orientation > 6) {
		Data_State.map.orientation = DIR_0_TOP;
	}
	CityView_calculateLookup();
	if (centerGridOffset >= 0) {
		int x, y;
		CityView_gridOffsetToXYCoords(centerGridOffset, &x, &y);
		Data_State.map.camera.x = x - Data_CityView.widthInTiles / 2;
		Data_State.map.camera.y = y - Data_CityView.heightInTiles / 2;
		if (Data_State.map.orientation == DIR_0_TOP ||
			Data_State.map.orientation == DIR_4_BOTTOM) {
			Data_State.map.camera.x++;
		}
	}
}

void CityView_rotateRight()
{
	int centerGridOffset = ViewToGridOffset(
		Data_State.map.camera.x + Data_CityView.widthInTiles / 2,
		Data_State.map.camera.y + Data_CityView.heightInTiles / 2);
	
	Data_State.map.orientation -= 2;
	if (Data_State.map.orientation < 0) {
		Data_State.map.orientation = DIR_6_LEFT;
	}
	CityView_calculateLookup();
	if (centerGridOffset >= 0) {
		int x, y;
		CityView_gridOffsetToXYCoords(centerGridOffset, &x, &y);
		Data_State.map.camera.x = x - Data_CityView.widthInTiles / 2;
		Data_State.map.camera.y = y - Data_CityView.heightInTiles / 2;
		if (Data_State.map.orientation == DIR_0_TOP ||
			Data_State.map.orientation == DIR_4_BOTTOM) {
			Data_State.map.camera.y += 2;
		}
	}
}
