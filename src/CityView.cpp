#include "CityView.h"
#include "Data/CityView.h"
#include "Data/Constants.h"
#include "Data/Grid.h"
#include "Data/Settings.h"

void CityView_setViewport(
	int xOffset, int yOffset, int widthInTiles, int heightInTiles)
{
	Data_CityView.xOffsetInPixels = xOffset;
	Data_CityView.yOffsetInPixels = yOffset;
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
	switch (Data_Settings_Map.orientation) {
		default:
		case Direction_Top:
			xViewStart = VIEW_X_MAX - 1;
			xViewSkip = -1;
			xViewStep = 1;
			yViewStart = 1;
			yViewSkip = 1;
			yViewStep = 1;
			break;
		case Direction_Right:
			xViewStart = 1;
			xViewSkip = 1;
			xViewStep = 1;
			yViewStart = VIEW_X_MAX - 1;
			yViewSkip = 1;
			yViewStep = -1;
			break;
		case Direction_Bottom:
			xViewStart = VIEW_X_MAX - 1;
			xViewSkip = 1;
			xViewStep = -1;
			yViewStart = VIEW_Y_MAX - 2;
			yViewSkip = -1;
			yViewStep = -1;
			break;
		case Direction_Left:
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
			if (Data_Grid_graphicIds[gridOffset] < 6) {
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

void CityView_rotateLeft()
{
	int centerGridOffset = ViewToGridOffset(
		Data_Settings_Map.camera.x + Data_CityView.widthInTiles / 2,
		Data_Settings_Map.camera.y + Data_CityView.heightInTiles / 2);

	Data_Settings_Map.orientation += 2;
	if (Data_Settings_Map.orientation > 6) {
		Data_Settings_Map.orientation = Direction_Top;
	}
	CityView_calculateLookup();
	if (centerGridOffset >= 0) {
		int x, y;
		CityView_gridOffsetToXYCoords(centerGridOffset, &x, &y);
		Data_Settings_Map.camera.x = x - Data_CityView.widthInTiles / 2;
		Data_Settings_Map.camera.y = y - Data_CityView.heightInTiles / 2;
		if (Data_Settings_Map.orientation == Direction_Top ||
			Data_Settings_Map.orientation == Direction_Bottom) {
			Data_Settings_Map.camera.x++;
		}
	}
}

void CityView_rotateRight()
{
	int centerGridOffset = ViewToGridOffset(
		Data_Settings_Map.camera.x + Data_CityView.widthInTiles / 2,
		Data_Settings_Map.camera.y + Data_CityView.heightInTiles / 2);

	Data_Settings_Map.orientation -= 2;
	if (Data_Settings_Map.orientation < 0) {
		Data_Settings_Map.orientation = Direction_Left;
	}
	CityView_calculateLookup();
	if (centerGridOffset >= 0) {
		int x, y;
		CityView_gridOffsetToXYCoords(centerGridOffset, &x, &y);
		Data_Settings_Map.camera.x = x - Data_CityView.widthInTiles / 2;
		Data_Settings_Map.camera.y = y - Data_CityView.heightInTiles / 2;
		if (Data_Settings_Map.orientation == Direction_Top ||
			Data_Settings_Map.orientation == Direction_Bottom) {
			Data_Settings_Map.camera.y += 2;
		}
	}
}
