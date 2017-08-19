#include "CityBuildings.h"

#include "../CityView.h"
#include "../Sound.h"

#include "../Data/Constants.h"
#include "../Data/Settings.h"

void UI_CityBuildings_scrollMap(int direction)
{
	if (direction == Dir_8_None) {
		return;
	}
	int dx = 1;
	int dy = 2;
	switch (direction) {
		case Dir_0_Top:
			Data_Settings_Map.camera.y -= dy;
			break;
		case Dir_1_TopRight:
			Data_Settings_Map.camera.x += dx;
			Data_Settings_Map.camera.y -= dy;
			break;
		case Dir_2_Right:
			Data_Settings_Map.camera.x += dx;
			break;
		case Dir_3_BottomRight:
			Data_Settings_Map.camera.x += dx;
			Data_Settings_Map.camera.y += dy;
			break;
		case Dir_4_Bottom:
			Data_Settings_Map.camera.y += dy;
			break;
		case Dir_5_BottomLeft:
			Data_Settings_Map.camera.x -= dx;
			Data_Settings_Map.camera.y += dy;
			break;
		case Dir_6_Left:
			Data_Settings_Map.camera.x -= dx;
			break;
		case Dir_7_TopLeft:
			Data_Settings_Map.camera.x -= dx;
			Data_Settings_Map.camera.y -= dy;
			break;
	}
	Sound_City_decayViews();
	CityView_checkCameraBoundaries();
}
