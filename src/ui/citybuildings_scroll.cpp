#include "citybuildings.h"

#include "cityview.h"

#include <sound>
#include <data>
#include <core>

void UI_CityBuildings_scrollMap(int direction)
{
    if (direction == DIR_8_NONE)
    {
        return;
    }
    int dx = 1;
    int dy = 2;
    switch (direction)
    {
    case DIR_0_TOP:
        Data_State.map.camera.y -= dy;
        break;
    case DIR_1_TOP_RIGHT:
        Data_State.map.camera.x += dx;
        Data_State.map.camera.y -= dy;
        break;
    case DIR_2_RIGHT:
        Data_State.map.camera.x += dx;
        break;
    case DIR_3_BOTTOM_RIGHT:
        Data_State.map.camera.x += dx;
        Data_State.map.camera.y += dy;
        break;
    case DIR_4_BOTTOM:
        Data_State.map.camera.y += dy;
        break;
    case DIR_5_BOTTOM_LEFT:
        Data_State.map.camera.x -= dx;
        Data_State.map.camera.y += dy;
        break;
    case DIR_6_LEFT:
        Data_State.map.camera.x -= dx;
        break;
    case DIR_7_TOP_LEFT:
        Data_State.map.camera.x -= dx;
        Data_State.map.camera.y -= dy;
        break;
    }
    sound_city_decay_views();
    CityView_checkCameraBoundaries();
}
