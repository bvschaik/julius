#include "view.h"

#include "core/direction.h"
#include "map/grid.h"
#include "map/image.h"

#include "Data/CityView.h"
#include "Data/State.h"

#define MENUBAR_HEIGHT 24

static struct {
    int screen_width;
    int screen_height;
} data;

int city_view_orientation()
{
    return Data_State.map.orientation;
}

void city_view_check_camera_boundaries()
{
    int x_min = (165 - Data_State.map.width) / 2;
    int y_min = (323 - 2 * Data_State.map.height) / 2;
    if (Data_State.map.camera.x < x_min - 1) {
        Data_State.map.camera.x = x_min - 1;
    }
    if (Data_State.map.camera.x > 165 - x_min - Data_CityView.widthInTiles) {
        Data_State.map.camera.x = 165 - x_min - Data_CityView.widthInTiles;
    }
    if (Data_State.map.camera.y < y_min) {
        Data_State.map.camera.y = y_min;
    }
    if (Data_State.map.camera.y > 327 - y_min - Data_CityView.heightInTiles) {
        Data_State.map.camera.y = 327 - y_min - Data_CityView.heightInTiles;
    }
    Data_State.map.camera.y &= ~1;
}

static void reset_lookup()
{
    for (int y = 0; y < VIEW_Y_MAX; y++) {
        for (int x = 0; x < VIEW_X_MAX; x++) {
            Data_CityView.viewToGridOffsetLookup[x][y] = -1;
        }
    }
}

void city_view_calculate_lookup()
{
    reset_lookup();
    int y_view_start;
    int y_view_skip;
    int y_view_step;
    int x_view_start;
    int x_view_skip;
    int x_view_step;
    switch (Data_State.map.orientation) {
        default:
        case DIR_0_TOP:
            x_view_start = VIEW_X_MAX - 1;
            x_view_skip = -1;
            x_view_step = 1;
            y_view_start = 1;
            y_view_skip = 1;
            y_view_step = 1;
            break;
        case DIR_2_RIGHT:
            x_view_start = 1;
            x_view_skip = 1;
            x_view_step = 1;
            y_view_start = VIEW_X_MAX - 1;
            y_view_skip = 1;
            y_view_step = -1;
            break;
        case DIR_4_BOTTOM:
            x_view_start = VIEW_X_MAX - 1;
            x_view_skip = 1;
            x_view_step = -1;
            y_view_start = VIEW_Y_MAX - 2;
            y_view_skip = -1;
            y_view_step = -1;
            break;
        case DIR_6_LEFT:
            x_view_start = VIEW_Y_MAX - 2;
            x_view_skip = -1;
            x_view_step = -1;
            y_view_start = VIEW_X_MAX - 1;
            y_view_skip = -1;
            y_view_step = 1;
            break;
    }

    for (int y = 0; y < GRID_SIZE; y++) {
        int x_view = x_view_start;
        int y_view = y_view_start;
        for (int x = 0; x < GRID_SIZE; x++) {
            int grid_offset = x + GRID_SIZE * y;
            if (map_image_at(grid_offset) < 6) {
                Data_CityView.viewToGridOffsetLookup[x_view/2][y_view] = -1;
            } else {
                Data_CityView.viewToGridOffsetLookup[x_view/2][y_view] = grid_offset;
            }
            x_view += x_view_step;
            y_view += y_view_step;
        }
        x_view_start += x_view_skip;
        y_view_start += y_view_skip;
    }
}

int city_view_to_grid_offset(int x_view, int y_view)
{
    return Data_CityView.viewToGridOffsetLookup[x_view][y_view];
}

void city_view_grid_offset_to_xy_view(int grid_offset, int *x_view, int *y_view)
{
    *x_view = *y_view = 0;
    for (int y = 0; y < VIEW_Y_MAX; y++) {
        for (int x = 0; x < VIEW_X_MAX; x++) {
            if (Data_CityView.viewToGridOffsetLookup[x][y] == grid_offset) {
                *x_view = x;
                *y_view = y;
                return;
            }
        }
    }
}

int city_view_pixels_to_grid_offset(int x_pixels, int y_pixels)
{
    if (x_pixels < Data_CityView.xOffsetInPixels ||
            x_pixels >= Data_CityView.xOffsetInPixels + Data_CityView.widthInPixels ||
            y_pixels < Data_CityView.yOffsetInPixels ||
            y_pixels >= Data_CityView.yOffsetInPixels + Data_CityView.heightInPixels) {
        return 0;
    }

    int odd = ((x_pixels - Data_CityView.xOffsetInPixels) / 30 + (y_pixels - Data_CityView.yOffsetInPixels) / 15) & 1;
    int x_is_odd = ((x_pixels - Data_CityView.xOffsetInPixels) / 30) & 1;
    int y_is_odd = ((y_pixels - Data_CityView.yOffsetInPixels) / 15) & 1;
    int x_mod = ((x_pixels - Data_CityView.xOffsetInPixels) % 30) / 2;
    int y_mod = (y_pixels - Data_CityView.yOffsetInPixels) % 15;
    int x_view_offset = (x_pixels - Data_CityView.xOffsetInPixels) / 60;
    int y_view_offset = (y_pixels - Data_CityView.yOffsetInPixels) / 15;
    if (odd) {
        if (x_mod + y_mod >= 15 - 1) {
            y_view_offset++;
            if (x_is_odd && !y_is_odd) {
                x_view_offset++;
            }
        }
    } else {
        if (y_mod > x_mod) {
            y_view_offset++;
        } else if (x_is_odd && y_is_odd) {
            x_view_offset++;
        }
    }
    Data_CityView.selectedTile.xOffsetInPixels =
        Data_CityView.xOffsetInPixels + 60 * x_view_offset;
    if (y_view_offset & 1) {
        Data_CityView.selectedTile.xOffsetInPixels -= 30;
    }
    Data_CityView.selectedTile.yOffsetInPixels =
        Data_CityView.yOffsetInPixels + 15 * y_view_offset - 15; // TODO why -1?
    int x_view = Data_CityView.xInTiles + x_view_offset;
    int y_view = Data_CityView.yInTiles + y_view_offset;
    int grid_offset = Data_CityView.viewToGridOffsetLookup[x_view][y_view];
    return grid_offset < 0 ? 0 : grid_offset;
}

void city_view_go_to_grid_offset(int grid_offset)
{
    int x, y;
    city_view_grid_offset_to_xy_view(grid_offset, &x, &y);
    Data_State.map.camera.x = x - Data_CityView.widthInTiles / 2;
    Data_State.map.camera.y = y - Data_CityView.heightInTiles / 2;
    Data_State.map.camera.y &= ~1;
    city_view_check_camera_boundaries();
}

static int get_center_grid_offset()
{
    int x_center = Data_State.map.camera.x + Data_CityView.widthInTiles / 2;
    int y_center = Data_State.map.camera.y + Data_CityView.heightInTiles / 2;
    return Data_CityView.viewToGridOffsetLookup[x_center][y_center];
}

void city_view_rotate_left()
{
    int center_grid_offset = get_center_grid_offset();

    Data_State.map.orientation += 2;
    if (Data_State.map.orientation > 6) {
        Data_State.map.orientation = DIR_0_TOP;
    }
    city_view_calculate_lookup();
    if (center_grid_offset >= 0) {
        int x, y;
        city_view_grid_offset_to_xy_view(center_grid_offset, &x, &y);
        Data_State.map.camera.x = x - Data_CityView.widthInTiles / 2;
        Data_State.map.camera.y = y - Data_CityView.heightInTiles / 2;
        if (Data_State.map.orientation == DIR_0_TOP ||
            Data_State.map.orientation == DIR_4_BOTTOM) {
            Data_State.map.camera.x++;
        }
    }
}

void city_view_rotate_right()
{
    int center_grid_offset = get_center_grid_offset();
    
    Data_State.map.orientation -= 2;
    if (Data_State.map.orientation < 0) {
        Data_State.map.orientation = DIR_6_LEFT;
    }
    city_view_calculate_lookup();
    if (center_grid_offset >= 0) {
        int x, y;
        city_view_grid_offset_to_xy_view(center_grid_offset, &x, &y);
        Data_State.map.camera.x = x - Data_CityView.widthInTiles / 2;
        Data_State.map.camera.y = y - Data_CityView.heightInTiles / 2;
        if (Data_State.map.orientation == DIR_0_TOP ||
            Data_State.map.orientation == DIR_4_BOTTOM) {
            Data_State.map.camera.y += 2;
        }
    }
}

static void set_viewport(int x_offset, int y_offset, int width, int height)
{
    int width_tiles = width / 60;
    int height_tiles = height / 15;
    Data_CityView.xOffsetInPixels = x_offset;
    Data_CityView.yOffsetInPixels = y_offset;
    Data_CityView.widthInPixels = width_tiles * 60 - 2;
    Data_CityView.heightInPixels = height_tiles * 15;
    Data_CityView.widthInTiles = width_tiles;
    Data_CityView.heightInTiles = height_tiles;
    Data_CityView.xInTiles = GRID_SIZE / 2;
    Data_CityView.yInTiles = GRID_SIZE;
}

static void set_viewport_with_sidebar()
{
    set_viewport(0, MENUBAR_HEIGHT, data.screen_width - 160, data.screen_height - MENUBAR_HEIGHT);
}

static void set_viewport_without_sidebar()
{
    set_viewport(0, MENUBAR_HEIGHT, data.screen_width - 40, data.screen_height - MENUBAR_HEIGHT);
}

void city_view_set_viewport(int screen_width, int screen_height)
{
    data.screen_width = screen_width;
    data.screen_height = screen_height;
    if (Data_State.sidebarCollapsed) {
        set_viewport_without_sidebar();
    } else {
        set_viewport_with_sidebar();
    }
}

int city_view_is_sidebar_collapsed()
{
    return Data_State.sidebarCollapsed;
}

void city_view_start_sidebar_toggle()
{
    set_viewport_without_sidebar();
    city_view_check_camera_boundaries();
}

void city_view_toggle_sidebar()
{
    if (Data_State.sidebarCollapsed) {
        Data_State.sidebarCollapsed = 0;
        set_viewport_with_sidebar();
    } else {
        Data_State.sidebarCollapsed = 1;
        set_viewport_without_sidebar();
    }
    city_view_check_camera_boundaries();
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
