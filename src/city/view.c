#include "view.h"

#include "core/direction.h"
#include "game/settings.h"
#include "map/grid.h"
#include "map/image.h"
#include "widget/minimap.h"

#define MENUBAR_HEIGHT 24

static struct {
    int screen_width;
    int screen_height;
    int sidebar_collapsed;
    int orientation;
    struct {
        int x;
        int y;
        int pixel_x;
        int pixel_y;
    } camera;
    struct {
        int x;
        int y;
        int width_pixels;
        int height_pixels;
        int width_tiles;
        int height_tiles;
    } viewport;
    struct {
        int x_pixels;
        int y_pixels;
    } selected_tile;
} data;

static int view_to_grid_offset_lookup[VIEW_X_MAX][VIEW_Y_MAX];

static void check_camera_boundaries(void)
{
    int x_min = (VIEW_X_MAX - map_grid_width()) / 2;
    int y_min = (VIEW_Y_MAX - 2 * map_grid_height()) / 2;

    if (data.camera.x < x_min - 1) {
        data.camera.x = x_min - 1;
        data.camera.pixel_x = 0;
    }
    if (data.camera.x > VIEW_X_MAX - x_min - data.viewport.width_tiles - 1) {
        data.camera.x = VIEW_X_MAX - x_min - data.viewport.width_tiles - 1;
        data.camera.pixel_x = 60;
    }
    if (data.camera.y < y_min - 2) {
        data.camera.y = y_min - 1;
        data.camera.pixel_y = 0;
    }
    if (data.camera.y > VIEW_Y_MAX - y_min - data.viewport.height_tiles - 2) {
        data.camera.y = VIEW_Y_MAX - y_min - data.viewport.height_tiles - 2;
        data.camera.pixel_y = 30;
    }
    data.camera.y &= ~1;
}

static void reset_lookup(void)
{
    for (int y = 0; y < VIEW_Y_MAX; y++) {
        for (int x = 0; x < VIEW_X_MAX; x++) {
            view_to_grid_offset_lookup[x][y] = -1;
        }
    }
}

static void calculate_lookup(void)
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
                view_to_grid_offset_lookup[x_view/2][y_view] = -1;
            } else {
                view_to_grid_offset_lookup[x_view/2][y_view] = grid_offset;
            }
            x_view += x_view_step;
            y_view += y_view_step;
        }
        x_view_start += x_view_skip;
        y_view_start += y_view_skip;
    }
}

void city_view_init(void)
{
    calculate_lookup();
    check_camera_boundaries();
    widget_minimap_invalidate();
}

int city_view_orientation(void)
{
    return data.orientation;
}

void city_view_reset_orientation(void)
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
    data.camera.pixel_x = 0;
    data.camera.pixel_y = 0;
    check_camera_boundaries();
}

static void adjust_camera_position_for_pixels(void) 
{
    if (data.camera.pixel_x < 0) {
        data.camera.x--;
        data.camera.pixel_x = 60 + data.camera.pixel_x;
    }
    if (data.camera.pixel_y < 0) {
        data.camera.y -= 2;
        data.camera.pixel_y = 30 + data.camera.pixel_y;
    }
    if (data.camera.pixel_x > 60) {
        data.camera.x++;
        data.camera.pixel_x -= 60;
    }
    if (data.camera.pixel_y > 30) {
        data.camera.y += 2;
        data.camera.pixel_y -= 30;
    }
}

int city_view_scroll(int direction)
{
    if (direction == DIR_8_NONE) {
        return 0;
    }
    int dx = 20 * setting_scroll_speed() / 100;
    int dy = dx;
    switch (direction) {
        case DIR_0_TOP:
            data.camera.pixel_y -= dy;
            break;
        case DIR_1_TOP_RIGHT:
            data.camera.pixel_x += dx;
            data.camera.pixel_y -= dy;
            break;
        case DIR_2_RIGHT:
            data.camera.pixel_x += dx;
            break;
        case DIR_3_BOTTOM_RIGHT:
            data.camera.pixel_x += dx;
            data.camera.pixel_y += dy;
            break;
        case DIR_4_BOTTOM:
            data.camera.pixel_y += dy;
            break;
        case DIR_5_BOTTOM_LEFT:
            data.camera.pixel_x -= dx;
            data.camera.pixel_y += dy;
            break;
        case DIR_6_LEFT:
            data.camera.pixel_x -= dx;
            break;
        case DIR_7_TOP_LEFT:
            data.camera.pixel_x -= dx;
            data.camera.pixel_y -= dy;
            break;
    }
    adjust_camera_position_for_pixels();
    check_camera_boundaries();
    return 1;
}

int city_view_to_grid_offset(int x_view, int y_view)
{
    return view_to_grid_offset_lookup[x_view][y_view];
}

void city_view_grid_offset_to_xy_view(int grid_offset, int *x_view, int *y_view)
{
    *x_view = *y_view = 0;
    for (int y = 0; y < VIEW_Y_MAX; y++) {
        for (int x = 0; x < VIEW_X_MAX; x++) {
            if (view_to_grid_offset_lookup[x][y] == grid_offset) {
                *x_view = x;
                *y_view = y;
                return;
            }
        }
    }
}

void city_view_get_selected_tile_pixels(int *x_pixels, int *y_pixels)
{
    *x_pixels = data.selected_tile.x_pixels;
    *y_pixels = data.selected_tile.y_pixels;
}

int city_view_pixels_to_view_tile(int x_pixels, int y_pixels, view_tile *tile)
{
    if (x_pixels < data.viewport.x ||
            x_pixels >= data.viewport.x + data.viewport.width_pixels ||
            y_pixels < data.viewport.y ||
            y_pixels >= data.viewport.y + data.viewport.height_pixels) {
        return 0;
    }

    int odd = ((x_pixels - data.viewport.x) / 30 + (y_pixels - data.viewport.y) / 15) & 1;
    int x_is_odd = ((x_pixels - data.viewport.x) / 30) & 1;
    int y_is_odd = ((y_pixels - data.viewport.y) / 15) & 1;
    int x_mod = ((x_pixels - data.viewport.x) % 30) / 2;
    int y_mod = (y_pixels - data.viewport.y) % 15;
    int x_view_offset = (x_pixels - data.viewport.x) / 60;
    int y_view_offset = (y_pixels - data.viewport.y) / 15;
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
    data.selected_tile.x_pixels = data.viewport.x + 60 * x_view_offset;
    if (y_view_offset & 1) {
        data.selected_tile.x_pixels -= 30;
    }
    data.selected_tile.y_pixels = data.viewport.y + 15 * y_view_offset - 15; // TODO why -1?

    tile->x = data.camera.x + x_view_offset;
    tile->y = data.camera.y + y_view_offset;
    return 1;
}

int city_view_pixels_to_grid_offset(int x_pixels, int y_pixels)
{
    view_tile view;
    if (!city_view_pixels_to_view_tile(x_pixels, y_pixels, &view)) {
        return 0;
    }
    int grid_offset = view_to_grid_offset_lookup[view.x][view.y];
    return grid_offset < 0 ? 0 : grid_offset;
}

void city_view_go_to_grid_offset(int grid_offset)
{
    int x, y;
    city_view_grid_offset_to_xy_view(grid_offset, &x, &y);
    data.camera.x = x - data.viewport.width_tiles / 2;
    data.camera.y = y - data.viewport.height_tiles / 2;
    data.camera.y &= ~1;
    check_camera_boundaries();
}

static int get_center_grid_offset(void)
{
    int x_center = data.camera.x + data.viewport.width_tiles / 2;
    int y_center = data.camera.y + data.viewport.height_tiles / 2;
    return view_to_grid_offset_lookup[x_center][y_center];
}

void city_view_rotate_left(void)
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
        data.camera.x = x - data.viewport.width_tiles / 2;
        data.camera.y = y - data.viewport.height_tiles / 2;
        if (data.orientation == DIR_0_TOP ||
            data.orientation == DIR_4_BOTTOM) {
            data.camera.x++;
        }
    }
    check_camera_boundaries();
}

void city_view_rotate_right(void)
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
        data.camera.x = x - data.viewport.width_tiles / 2;
        data.camera.y = y - data.viewport.height_tiles / 2;
        if (data.orientation == DIR_0_TOP ||
            data.orientation == DIR_4_BOTTOM) {
            data.camera.y += 2;
        }
    }
    check_camera_boundaries();
}

static void set_viewport(int x_offset, int y_offset, int width, int height)
{
    data.viewport.x = x_offset;
    data.viewport.y = y_offset;
    data.viewport.width_pixels = width - 2;
    data.viewport.height_pixels = height * 15;
    data.viewport.width_tiles = width / 60;
    data.viewport.height_tiles = height / 15;
}

static void set_viewport_with_sidebar(void)
{
    set_viewport(0, MENUBAR_HEIGHT, data.screen_width - 160, data.screen_height - MENUBAR_HEIGHT);
}

static void set_viewport_without_sidebar(void)
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

void city_view_get_viewport(int *x, int *y, int *width, int *height)
{
    *x = data.viewport.x;
    *y = data.viewport.y;
    *width = data.viewport.width_pixels;
    *height = data.viewport.height_pixels;
}

void city_view_get_viewport_size_tiles(int *width, int *height)
{
    *width = data.viewport.width_tiles;
    *height = data.viewport.height_tiles;
}

int city_view_is_sidebar_collapsed(void)
{
    return data.sidebar_collapsed;
}

void city_view_start_sidebar_toggle(void)
{
    set_viewport_without_sidebar();
    check_camera_boundaries();
}

void city_view_toggle_sidebar(void)
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

void city_view_save_scenario_state(buffer *camera)
{
    buffer_write_i32(camera, data.camera.x);
    buffer_write_i32(camera, data.camera.y);
}

void city_view_load_scenario_state(buffer *camera)
{
    data.camera.x = buffer_read_i32(camera);
    data.camera.y = buffer_read_i32(camera);
}

void city_view_foreach_map_tile(map_callback *callback)
{
    int odd = 0;
    int y_view = data.camera.y - 8;
    int y_graphic = data.viewport.y - 9*15 - data.camera.pixel_y;
    for (int y = 0; y < data.viewport.height_tiles + 14; y++) {
        if (y_view >= 0 && y_view < VIEW_Y_MAX) {
            int x_graphic = -(4*58 + 8) - data.camera.pixel_x;
            if (odd) {
                x_graphic += data.viewport.x - 30;
            } else {
                x_graphic += data.viewport.x;
            }
            int x_view = data.camera.x - 4;
            for (int x = 0; x < data.viewport.width_tiles + 7; x++) {
                if (x_view >= 0 && x_view < VIEW_X_MAX) {
                    int grid_offset = view_to_grid_offset_lookup[x_view][y_view];
                    callback(x_graphic, y_graphic, grid_offset);
                }
                x_graphic += 60;
                x_view++;
            }
        }
        odd = 1 - odd;
        y_graphic += 15;
        y_view++;
    }
}

void city_view_foreach_valid_map_tile(map_callback *callback1, map_callback *callback2, map_callback *callback3)
{
    int odd = 0;
    int y_view = data.camera.y - 8;
    int y_graphic = data.viewport.y - 9*15 - data.camera.pixel_y;
    int x_graphic, x_view;
    for (int y = 0; y < data.viewport.height_tiles + 14; y++) {
        if (y_view >= 0 && y_view < VIEW_Y_MAX) {
            if (callback1) {
                x_graphic = -(4*58 + 8) - data.camera.pixel_x;
                if (odd) {
                    x_graphic += data.viewport.x - 30;
                } else {
                    x_graphic += data.viewport.x;
                }
                x_view = data.camera.x - 4;
                for (int x = 0; x < data.viewport.width_tiles + 7; x++) {
                    if (x_view >= 0 && x_view < VIEW_X_MAX) {
                        int grid_offset = view_to_grid_offset_lookup[x_view][y_view];
                        if (grid_offset >= 0) {
                            callback1(x_graphic, y_graphic, grid_offset);
                        }
                    }
                    x_graphic += 60;
                    x_view++;
                }
            }
            if (callback2) {
                x_graphic = -(4*58 + 8) - data.camera.pixel_x;
                if (odd) {
                    x_graphic += data.viewport.x - 30;
                } else {
                    x_graphic += data.viewport.x;
                }
                x_view = data.camera.x - 4;
                for (int x = 0; x < data.viewport.width_tiles + 7; x++) {
                    if (x_view >= 0 && x_view < VIEW_X_MAX) {
                        int grid_offset = view_to_grid_offset_lookup[x_view][y_view];
                        if (grid_offset >= 0) {
                            callback2(x_graphic, y_graphic, grid_offset);
                        }
                    }
                    x_graphic += 60;
                    x_view++;
                }
            }
            if (callback3) {
                x_graphic = -(4*58 + 8) - data.camera.pixel_x;
                if (odd) {
                    x_graphic += data.viewport.x - 30;
                } else {
                    x_graphic += data.viewport.x;
                }
                x_view = data.camera.x - 4;
                for (int x = 0; x < data.viewport.width_tiles + 7; x++) {
                    if (x_view >= 0 && x_view < VIEW_X_MAX) {
                        int grid_offset = view_to_grid_offset_lookup[x_view][y_view];
                        if (grid_offset >= 0) {
                            callback3(x_graphic, y_graphic, grid_offset);
                        }
                    }
                    x_graphic += 60;
                    x_view++;
                }
            }
        }
        odd = 1 - odd;
        y_graphic += 15;
        y_view++;
    }
}

void city_view_foreach_minimap_tile(int x_offset, int y_offset, int absolute_x, int absolute_y, int width_tiles, int height_tiles, map_callback *callback)
{
    int odd = 0;
    int y_abs = absolute_y - 4;
    int y_view = y_offset - 4;
    for (int y_rel = -4; y_rel < height_tiles + 4; y_rel++, y_abs++, y_view++) {
        int x_view;
        if (odd) {
            x_view = x_offset - 9;
            odd = 0;
        } else {
            x_view = x_offset - 8;
            odd = 1;
        }
        int x_abs = absolute_x - 4;
        for (int x_rel = -4; x_rel < width_tiles; x_rel++, x_abs++, x_view += 2) {
            if (x_abs >= 0 && x_abs < VIEW_X_MAX && y_abs >= 0 && y_abs < VIEW_Y_MAX) {
                callback(x_view, y_view, view_to_grid_offset_lookup[x_abs][y_abs]);
            }
        }
    }
}
