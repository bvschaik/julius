#include "view.h"

#include "Data/State.h"

int city_view_orientation()
{
    return Data_State.map.orientation;
}

void city_view_save_state(buffer *orientation, buffer *camera)
{
    buffer_write_i32(orientation, Data_State.map.orientation);

    buffer_write_i32(camera, Data_State.map.camera.x);
    buffer_write_i32(camera, Data_State.map.camera.y);
}

void city_view_load_state(buffer *orientation, buffer *camera)
{
    Data_State.map.orientation = buffer_read_i32(orientation);
    city_view_load_scenario_state(camera);
}

void city_view_load_scenario_state(buffer *camera)
{
    Data_State.map.camera.x = buffer_read_i32(camera);
    Data_State.map.camera.y = buffer_read_i32(camera);
    
}
