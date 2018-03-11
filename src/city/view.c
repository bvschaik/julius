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
    int sidebar_collapsed;
    int orientation;
    struct {
        int x;
        int y;
    } camera;
} data = {0, 0, 0, 0, {0, 0}};


static void check_camera_boundaries()
{
    int x_min = (165 - Data_State.map.width) / 2;
    int y_min = (323 - 2 * Data_State.map.height) / 2;
    if (data.camera.x < x_min - 1) {
        data.camera.x = x_min - 1;
    }
    if (data.camera.x > 165 - x_min - Data_CityView.widthInTiles) {
        data.camera.x = 165 - x_min - Data_CityView.widthInTiles;
    }
    if (data.camera.y < y_min) {
        data.camera.y = y_min;
    }
    if (data.camera.y > 327 - y_min - Data_CityView.heightInTiles) {
        data.camera.y = 327 - y_min - Data_CityView.heightInTiles;
    }
    data.camera.y &= ~1;
}

static void reset_lookup()
{
    for (int y = 0; y < VIEW_Y_MAX; y++) {
        for (int x = 0; x < VIEW_X_MAX; x++) {
            Data_CityView.viewToGridOffsetLookup[x][y] = -1;
        }
    }
}

static void calculate_lookup()
{
    reset_lookup();
    int y_view_start;
    int y_view_skip;
    int y_view_step;
    int x_view_start;
    int x_view_skip;
    int x_view_step;
    switch (data.orientation) {
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

void city_view_init()
{
    calculate_lookup();
    check_camera_boundaries();
}

int city_view_orientation()
{
    return data.orientation;
}

void city_view_reset_orientation()
{
    data.orientation = 0;
    calculate_lookup();
}

void city_view_get_camera(int *x, int *y)
{
    *x = data.camera.x;
    *y = data.camera.y;
}

void city_view_set_camera(int x, int y)
{
    data.camera.x = x;
    data.camera.y = y;
    check_camera_boundaries();
}

int city_view_scroll(int direction)
{
    if (direction == DIR_8_NONE) {
        return 0;
    }
    int dx = 1;
    int dy = 2;
    switch (direction) {
        case DIR_0_TOP:
            data.camera.y -= dy;
            break;
        case DIR_1_TOP_RIGHT:
            data.camera.x += dx;
            data.camera.y -= dy;
            break;
        case DIR_2_RIGHT:
            data.camera.x += dx;
            break;
        case DIR_3_BOTTOM_RIGHT:
            data.camera.x += dx;
            data.camera.y += dy;
            break;
        case DIR_4_BOTTOM:
            data.camera.y += dy;
            break;
        case DIR_5_BOTTOM_LEFT:
            data.camera.x -= dx;
            data.camera.y += dy;
            break;
        case DIR_6_LEFT:
            data.camera.x -= dx;
            break;
        case DIR_7_TOP_LEFT:
            data.camera.x -= dx;
            data.camera.y -= dy;
            break;
    }
    check_camera_boundaries();
    return 1;
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
    data.camera.x = x - Data_CityView.widthInTiles / 2;
    data.camera.y = y - Data_CityView.heightInTiles / 2;
    data.camera.y &= ~1;
    check_camera_boundaries();
}

static int get_center_grid_offset()
{
    int x_center = data.camera.x + Data_CityView.widthInTiles / 2;
    int y_center = data.camera.y + Data_CityView.heightInTiles / 2;
    return Data_CityView.viewToGridOffsetLookup[x_center][y_center];
}

void city_view_rotate_left()
{
    int center_grid_offset = get_center_grid_offset();

    data.orientation += 2;
    if (data.orientation > 6) {
        data.orientation = DIR_0_TOP;
    }
    calculate_lookup();
    if (center_grid_offset >= 0) {
        int x, y;
        city_view_grid_offset_to_xy_view(center_grid_offset, &x, &y);
        data.camera.x = x - Data_CityView.widthInTiles / 2;
        data.camera.y = y - Data_CityView.heightInTiles / 2;
        if (data.orientation == DIR_0_TOP ||
            data.orientation == DIR_4_BOTTOM) {
            data.camera.x++;
        }
    }
    check_camera_boundaries();
}

void city_view_rotate_right()
{
    int center_grid_offset = get_center_grid_offset();
    
    data.orientation -= 2;
    if (data.orientation < 0) {
        data.orientation = DIR_6_LEFT;
    }
    calculate_lookup();
    if (center_grid_offset >= 0) {
        int x, y;
        city_view_grid_offset_to_xy_view(center_grid_offset, &x, &y);
        data.camera.x = x - Data_CityView.widthInTiles / 2;
        data.camera.y = y - Data_CityView.heightInTiles / 2;
        if (data.orientation == DIR_0_TOP ||
            data.orientation == DIR_4_BOTTOM) {
            data.camera.y += 2;
        }
    }
    check_camera_boundaries();
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
    if (data.sidebar_collapsed) {
        set_viewport_without_sidebar();
    } else {
        set_viewport_with_sidebar();
    }
    check_camera_boundaries();
}

int city_view_is_sidebar_collapsed()
{
    return data.sidebar_collapsed;
}

void city_view_start_sidebar_toggle()
{
    set_viewport_without_sidebar();
    check_camera_boundaries();
}

void city_view_toggle_sidebar()
{
    if (data.sidebar_collapsed) {
        data.sidebar_collapsed = 0;
        set_viewport_with_sidebar();
    } else {
        data.sidebar_collapsed = 1;
        set_viewport_without_sidebar();
    }
    check_camera_boundaries();
}

void city_view_save_state(buffer *orientation, buffer *camera)
{
    buffer_write_i32(orientation, data.orientation);

    buffer_write_i32(camera, data.camera.x);
    buffer_write_i32(camera, data.camera.y);
}

void city_view_load_state(buffer *orientation, buffer *camera)
{
    data.orientation = buffer_read_i32(orientation);
    city_view_load_scenario_state(camera);

    if (data.orientation >= 0 && data.orientation <= 6) {
        // ensure even number
        data.orientation = 2 * (data.orientation / 2);
    } else {
        data.orientation = 0;
    }
}

void city_view_load_scenario_state(buffer *camera)
{
    data.camera.x = buffer_read_i32(camera);
    data.camera.y = buffer_read_i32(camera);
}

void city_view_foreach_map_tile(map_callback *callback)
{
    int odd = 0;
    int yView = Data_CityView.yInTiles - 8;
    int yGraphic = Data_CityView.yOffsetInPixels - 9*15;
    int xGraphic, xView;
    for (int y = 0; y < Data_CityView.heightInTiles + 14; y++) {
        if (yView >= 0 && yView < VIEW_Y_MAX) {
            xGraphic = -(4*58 + 8);
            if (odd) {
                xGraphic += Data_CityView.xOffsetInPixels - 30;
            } else {
                xGraphic += Data_CityView.xOffsetInPixels;
            }
            xView = Data_CityView.xInTiles - 4;
            for (int x = 0; x < Data_CityView.widthInTiles + 7; x++) {
                if (xView >= 0 && xView < VIEW_X_MAX) {
                    int gridOffset = ViewToGridOffset(xView, yView);
                    callback(xGraphic, yGraphic, gridOffset);
                }
                xGraphic += 60;
                xView++;
            }
        }
        odd = 1 - odd;
        yGraphic += 15;
        yView++;
    }
}

void city_view_foreach_valid_map_tile(map_callback *callback1, map_callback *callback2, map_callback *callback3)
{
    int odd = 0;
    int yView = Data_CityView.yInTiles - 8;
    int yGraphic = Data_CityView.yOffsetInPixels - 9*15;
    int xGraphic, xView;
    for (int y = 0; y < Data_CityView.heightInTiles + 14; y++) {
        if (yView >= 0 && yView < VIEW_Y_MAX) {
            if (callback1) {
                xGraphic = -(4*58 + 8);
                if (odd) {
                    xGraphic += Data_CityView.xOffsetInPixels - 30;
                } else {
                    xGraphic += Data_CityView.xOffsetInPixels;
                }
                xView = Data_CityView.xInTiles - 4;
                for (int x = 0; x < Data_CityView.widthInTiles + 7; x++) {
                    if (xView >= 0 && xView < VIEW_X_MAX) {
                        int gridOffset = ViewToGridOffset(xView, yView);
                        if (gridOffset >= 0) {
                            callback1(xGraphic, yGraphic, gridOffset);
                        }
                    }
                    xGraphic += 60;
                    xView++;
                }
            }
            if (callback2) {
                xGraphic = -(4*58 + 8);
                if (odd) {
                    xGraphic += Data_CityView.xOffsetInPixels - 30;
                } else {
                    xGraphic += Data_CityView.xOffsetInPixels;
                }
                xView = Data_CityView.xInTiles - 4;
                for (int x = 0; x < Data_CityView.widthInTiles + 7; x++) {
                    if (xView >= 0 && xView < VIEW_X_MAX) {
                        int gridOffset = ViewToGridOffset(xView, yView);
                        if (gridOffset >= 0) {
                            callback2(xGraphic, yGraphic, gridOffset);
                        }
                    }
                    xGraphic += 60;
                    xView++;
                }
            }
            if (callback3) {
                xGraphic = -(4*58 + 8);
                if (odd) {
                    xGraphic += Data_CityView.xOffsetInPixels - 30;
                } else {
                    xGraphic += Data_CityView.xOffsetInPixels;
                }
                xView = Data_CityView.xInTiles - 4;
                for (int x = 0; x < Data_CityView.widthInTiles + 7; x++) {
                    if (xView >= 0 && xView < VIEW_X_MAX) {
                        int gridOffset = ViewToGridOffset(xView, yView);
                        if (gridOffset >= 0) {
                            callback3(xGraphic, yGraphic, gridOffset);
                        }
                    }
                    xGraphic += 60;
                    xView++;
                }
            }
        }
        odd = 1 - odd;
        yGraphic += 15;
        yView++;
    }
}
