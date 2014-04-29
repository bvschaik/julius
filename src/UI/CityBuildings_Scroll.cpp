#include "CityBuildings.h"

#include "../CityView.h"
#include "../Sound.h"

#include "../Data/Constants.h"
#include "../Data/Settings.h"

void UI_CityBuildings_scrollMap(int direction)
{
	if (direction == Direction_None) {
		return;
	}
	int dx = 1;
	int dy = 2;
	switch (direction) {
		case Direction_Top:
			Data_Settings_Map.camera.y -= dy;
			break;
		case Direction_TopRight:
			Data_Settings_Map.camera.x += dx;
			Data_Settings_Map.camera.y -= dy;
			break;
		case Direction_Right:
			Data_Settings_Map.camera.x += dx;
			break;
		case Direction_BottomRight:
			Data_Settings_Map.camera.x += dx;
			Data_Settings_Map.camera.y += dy;
			break;
		case Direction_Bottom:
			Data_Settings_Map.camera.y += dy;
			break;
		case Direction_BottomLeft:
			Data_Settings_Map.camera.x -= dx;
			Data_Settings_Map.camera.y += dy;
			break;
		case Direction_Left:
			Data_Settings_Map.camera.x -= dx;
			break;
		case Direction_TopLeft:
			Data_Settings_Map.camera.x -= dx;
			Data_Settings_Map.camera.y -= dy;
			break;
	}
	Sound_City_decayViews();
	CityView_checkCameraBoundaries();
}
