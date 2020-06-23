#include "rotation.h"

#include "city/view.h"
#include "core/config.h"
#include "core/direction.h"
#include "core/time.h"
#include "map/grid.h"

static int rotation = 0;
static int road_orientation = 1;
static time_millis road_last_update = 0;

static void rotate(void){
    rotation+=1;
    if(rotation>3){
        rotation=0;
    }
}

void update_road_orientation(void)
{
    if (!config_get(CONFIG_UI_ROTATE_MANUALLY)) {
        if (time_get_millis() - road_last_update > 1500) {
            road_last_update = time_get_millis();
            road_orientation = road_orientation == 1 ? 2 : 1;
            rotate();
        }
    }
}

int get_road_orientation(void)
{
    return road_orientation;
}

void force_two_orientations(void){ // for composite buildings like hippodrome
    if(rotation == 1 || rotation == 2){
        rotation = 3;
    }
}

int get_rotation(void){
    return rotation;
}

void rotate_by_hotkey(void){
    if(config_get(CONFIG_UI_ROTATE_MANUALLY)){
        rotate();
        road_orientation = road_orientation == 1 ? 2 : 1;
    }
}

void reset_rotation(void){
    rotation = 0;
}

int get_building_orientation(int building_rotation){
    return (2*building_rotation + city_view_orientation())%8;
}

int get_delta_with_rotation(int default_delta){
    if(rotation == 0) {
        return map_grid_delta(default_delta, 0);
    } else if (rotation == 1) {
        return map_grid_delta(0, -default_delta);
    } else if (rotation == 2) {
        return map_grid_delta(-default_delta, 0);
    } else {
        return map_grid_delta(0, default_delta);
    }
}

void get_offset_with_rotation(int offset, int rot, int * x, int * y){
    if(rot == 0) {
        *x = offset;
        *y = 0;
    } else if (rot == 1) {
        *x = 0;
        *y = -offset;
    } else if (rot == 2) {
        *x = -offset;
        *y = 0;
    } else {
        *x = 0;
        *y = offset;
    }
}

int get_corner(int rot){
    switch(rot){
        case DIR_2_RIGHT:
            return 4; // left corner
        case DIR_4_BOTTOM:
            return 8; // bottom corner
        case DIR_6_LEFT:
            return 5; // right corner
        default:
            return 0; // top corner
    }
}