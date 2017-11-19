#include "CityBuildings.h"

#include "../CityView.h"

#include "../Data/Constants.h"
#include "../Data/State.h"

#include "sound/city.h"

void UI_CityBuildings_scrollMap(int direction)
{
	if (direction == Dir_8_None) {
		return;
	}
	int dx = 1;
	int dy = 2;
	switch (direction) {
		case Dir_0_Top:
			Data_State.map.camera.y -= dy;
			break;
		case Dir_1_TopRight:
			Data_State.map.camera.x += dx;
			Data_State.map.camera.y -= dy;
			break;
		case Dir_2_Right:
			Data_State.map.camera.x += dx;
			break;
		case Dir_3_BottomRight:
			Data_State.map.camera.x += dx;
			Data_State.map.camera.y += dy;
			break;
		case Dir_4_Bottom:
			Data_State.map.camera.y += dy;
			break;
		case Dir_5_BottomLeft:
			Data_State.map.camera.x -= dx;
			Data_State.map.camera.y += dy;
			break;
		case Dir_6_Left:
			Data_State.map.camera.x -= dx;
			break;
		case Dir_7_TopLeft:
			Data_State.map.camera.x -= dx;
			Data_State.map.camera.y -= dy;
			break;
	}
	sound_city_decay_views();
	CityView_checkCameraBoundaries();
}
