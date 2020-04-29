#include "view.h"

#include "core/direction.h"
#include "graphics/menu.h"
#include "map/grid.h"
#include "map/image.h"
#include "widget/minimap.h"

#define TILE_WIDTH_PIXELS 60
#define TILE_HEIGHT_PIXELS 30
#define HALF_TILE_WIDTH_PIXELS 30
#define HALF_TILE_HEIGHT_PIXELS 15

static struct {
    int screen_width;
    int screen_height;
    int sidebar_collapsed;
    int orientation;
    struct {
        view_tile tile;
        pixel_offset pixel;
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
    if (data.camera.tile.x < x_min - 1) {
        data.camera.tile.x = x_min - 1;
        data.camera.pixel.x = 0;
    }
    if (data.camera.tile.x >= VIEW_X_MAX - x_min - data.viewport.width_tiles) {
        data.camera.tile.x = VIEW_X_MAX - x_min - data.viewport.width_tiles;
        data.camera.pixel.x = 0;
    }
    if (data.camera.tile.y < y_min - 2) {
        data.camera.tile.y = y_min - 1;
        data.camera.pixel.y = 0;
    }
    if (data.camera.tile.y >= ((VIEW_Y_MAX - y_min - data.viewport.height_tiles) & ~1)) {
        data.camera.tile.y = VIEW_Y_MAX - y_min - data.viewport.height_tiles;
        data.camera.pixel.y = 0;
    }
    data.camera.tile.y &= ~1;
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
            x_view_start = 3;
            x_view_skip = 1;
            x_view_step = 1;
            y_view_start = VIEW_X_MAX - 3;
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
            x_view_start = VIEW_Y_MAX;
            x_view_skip = -1;
            x_view_step = -1;
            y_view_start = VIEW_X_MAX - 3;
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

static void adjust_camera_position_for_pixels(void)
{
    while (data.camera.pixel.x < 0) {
        data.camera.tile.x--;
        data.camera.pixel.x += TILE_WIDTH_PIXELS;
    }
    while (data.camera.pixel.y < 0) {
        data.camera.tile.y -= 2;
        data.camera.pixel.y += TILE_HEIGHT_PIXELS;
    }
    while (data.camera.pixel.x >= TILE_WIDTH_PIXELS) {
        data.camera.tile.x++;
        data.camera.pixel.x -= TILE_WIDTH_PIXELS;
    }
    while (data.camera.pixel.y >= TILE_HEIGHT_PIXELS) {
        data.camera.tile.y += 2;
        data.camera.pixel.y -= TILE_HEIGHT_PIXELS;
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
    *x = data.camera.tile.x;
    *y = data.camera.tile.y;
}

void city_view_get_pixel_offset(int *x, int *y)
{
    *x = data.camera.pixel.x;
    *y = data.camera.pixel.y;
}

void city_view_get_camera_in_pixels(int *x, int *y)
{
    *x = data.camera.tile.x * TILE_WIDTH_PIXELS + data.camera.pixel.x;
    *y = data.camera.tile.y * HALF_TILE_HEIGHT_PIXELS + data.camera.pixel.y;
}

void city_view_set_camera(int x, int y)
{
    data.camera.tile.x = x;
    data.camera.tile.y = y;
    check_camera_boundaries();
}

void city_view_set_camera_from_pixel_position(int x, int y)
{
    x = x < 0 ? 0 : x;
    y = y < 0 ? 0 : y;

    data.camera.tile.x = x / TILE_WIDTH_PIXELS;
    data.camera.tile.y = y / HALF_TILE_HEIGHT_PIXELS;
    data.camera.pixel.x = x % TILE_WIDTH_PIXELS;
    data.camera.pixel.y = y % TILE_HEIGHT_PIXELS;
    check_camera_boundaries();
}

int city_view_scroll(int x, int y)
{
    if (!x && !y) {
        return 0;
    }
    data.camera.pixel.x += x;
    data.camera.pixel.y += y;
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

    x_pixels += data.camera.pixel.x;
    y_pixels += data.camera.pixel.y;
    int odd = ((x_pixels - data.viewport.x) / HALF_TILE_WIDTH_PIXELS + (y_pixels - data.viewport.y) / HALF_TILE_HEIGHT_PIXELS) & 1;
    int x_is_odd = ((x_pixels - data.viewport.x) / HALF_TILE_WIDTH_PIXELS) & 1;
    int y_is_odd = ((y_pixels - data.viewport.y) / HALF_TILE_HEIGHT_PIXELS) & 1;
    int x_mod = ((x_pixels - data.viewport.x) % HALF_TILE_WIDTH_PIXELS) / 2;
    int y_mod = (y_pixels - data.viewport.y) % HALF_TILE_HEIGHT_PIXELS;
    int x_view_offset = (x_pixels - data.viewport.x) / TILE_WIDTH_PIXELS;
    int y_view_offset = (y_pixels - data.viewport.y) / HALF_TILE_HEIGHT_PIXELS;
    if (odd) {
        if (x_mod + y_mod >= HALF_TILE_HEIGHT_PIXELS - 1) {
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
    tile->x = data.camera.tile.x + x_view_offset;
    tile->y = data.camera.tile.y + y_view_offset;
    return 1;
}

void city_view_set_selected_view_tile(const view_tile *tile)
{
    int x_view_offset = tile->x - data.camera.tile.x;
    int y_view_offset = tile->y - data.camera.tile.y;
    data.selected_tile.x_pixels = data.viewport.x + TILE_WIDTH_PIXELS * x_view_offset - data.camera.pixel.x;
    if (y_view_offset & 1) {
        data.selected_tile.x_pixels -= HALF_TILE_WIDTH_PIXELS;
    }
    data.selected_tile.y_pixels = data.viewport.y + HALF_TILE_HEIGHT_PIXELS * y_view_offset - HALF_TILE_HEIGHT_PIXELS - data.camera.pixel.y; // TODO why -1?
}

int city_view_tile_to_grid_offset(const view_tile *tile)
{
    int grid_offset = view_to_grid_offset_lookup[tile->x][tile->y];
    return grid_offset < 0 ? 0 : grid_offset;
}

void city_view_go_to_grid_offset(int grid_offset)
{
    int x, y;
    city_view_grid_offset_to_xy_view(grid_offset, &x, &y);
    data.camera.tile.x = x - data.viewport.width_tiles / 2;
    data.camera.tile.y = y - data.viewport.height_tiles / 2;
    data.camera.tile.y &= ~1;
    check_camera_boundaries();
}

static int get_center_grid_offset(void)
{
    int x_center = data.camera.tile.x + data.viewport.width_tiles / 2;
    int y_center = data.camera.tile.y + data.viewport.height_tiles / 2;
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
        data.camera.tile.x = x - data.viewport.width_tiles / 2;
        data.camera.tile.y = y - data.viewport.height_tiles / 2;
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
        data.camera.tile.x = x - data.viewport.width_tiles / 2;
        data.camera.tile.y = y - data.viewport.height_tiles / 2;
    }
    check_camera_boundaries();
}

static void set_viewport(int x_offset, int y_offset, int width, int height)
{
    data.viewport.x = x_offset;
    data.viewport.y = y_offset;
    data.viewport.width_pixels = width - 2;
    data.viewport.height_pixels = height;
    data.viewport.width_tiles = width / TILE_WIDTH_PIXELS;
    data.viewport.height_tiles = height / HALF_TILE_HEIGHT_PIXELS;
}

static void set_viewport_with_sidebar(void)
{
    set_viewport(0, TOP_MENU_HEIGHT, data.screen_width - 160, data.screen_height - TOP_MENU_HEIGHT);
}

static void set_viewport_without_sidebar(void)
{
    set_viewport(0, TOP_MENU_HEIGHT, data.screen_width - 40, data.screen_height - TOP_MENU_HEIGHT);
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

    buffer_write_i32(camera, data.camera.tile.x);
    buffer_write_i32(camera, data.camera.tile.y);
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
    buffer_write_i32(camera, data.camera.tile.x);
    buffer_write_i32(camera, data.camera.tile.y);
}

void city_view_load_scenario_state(buffer *camera)
{
    data.camera.tile.x = buffer_read_i32(camera);
    data.camera.tile.y = buffer_read_i32(camera);
}

void city_view_foreach_map_tile(map_callback *callback)
{
    int odd = 0;
    int y_view = data.camera.tile.y - 8;
    int y_graphic = data.viewport.y - 9 * HALF_TILE_HEIGHT_PIXELS - data.camera.pixel.y;
    for (int y = 0; y < data.viewport.height_tiles + 21; y++) {
        if (y_view >= 0 && y_view < VIEW_Y_MAX) {
            int x_graphic = -(4 * TILE_WIDTH_PIXELS) - data.camera.pixel.x;
            if (odd) {
                x_graphic += data.viewport.x - HALF_TILE_WIDTH_PIXELS;
            } else {
                x_graphic += data.viewport.x;
            }
            int x_view = data.camera.tile.x - 4;
            for (int x = 0; x < data.viewport.width_tiles + 7; x++) {
                if (x_view >= 0 && x_view < VIEW_X_MAX) {
                    int grid_offset = view_to_grid_offset_lookup[x_view][y_view];
                    callback(x_graphic, y_graphic, grid_offset);
                }
                x_graphic += TILE_WIDTH_PIXELS;
                x_view++;
            }
        }
        odd = 1 - odd;
        y_graphic += HALF_TILE_HEIGHT_PIXELS;
        y_view++;
    }
}

void city_view_foreach_valid_map_tile(map_callback *callback1, map_callback *callback2, map_callback *callback3)
{
    int odd = 0;
    int y_view = data.camera.tile.y - 8;
    int y_graphic = data.viewport.y - 9 * HALF_TILE_HEIGHT_PIXELS - data.camera.pixel.y;
    int x_graphic, x_view;
    for (int y = 0; y < data.viewport.height_tiles + 21; y++) {
        if (y_view >= 0 && y_view < VIEW_Y_MAX) {
            if (callback1) {
                x_graphic = -(4 * TILE_WIDTH_PIXELS) - data.camera.pixel.x;
                if (odd) {
                    x_graphic += data.viewport.x - HALF_TILE_WIDTH_PIXELS;
                } else {
                    x_graphic += data.viewport.x;
                }
                x_view = data.camera.tile.x - 4;
                for (int x = 0; x < data.viewport.width_tiles + 7; x++) {
                    if (x_view >= 0 && x_view < VIEW_X_MAX) {
                        int grid_offset = view_to_grid_offset_lookup[x_view][y_view];
                        if (grid_offset >= 0) {
                            callback1(x_graphic, y_graphic, grid_offset);
                        }
                    }
                    x_graphic += TILE_WIDTH_PIXELS;
                    x_view++;
                }
            }
            if (callback2) {
                x_graphic = -(4 * TILE_WIDTH_PIXELS) - data.camera.pixel.x;
                if (odd) {
                    x_graphic += data.viewport.x - HALF_TILE_WIDTH_PIXELS;
                } else {
                    x_graphic += data.viewport.x;
                }
                x_view = data.camera.tile.x - 4;
                for (int x = 0; x < data.viewport.width_tiles + 7; x++) {
                    if (x_view >= 0 && x_view < VIEW_X_MAX) {
                        int grid_offset = view_to_grid_offset_lookup[x_view][y_view];
                        if (grid_offset >= 0) {
                            callback2(x_graphic, y_graphic, grid_offset);
                        }
                    }
                    x_graphic += TILE_WIDTH_PIXELS;
                    x_view++;
                }
            }
            if (callback3) {
                x_graphic = -(4 * TILE_WIDTH_PIXELS) - data.camera.pixel.x;
                if (odd) {
                    x_graphic += data.viewport.x - HALF_TILE_WIDTH_PIXELS;
                } else {
                    x_graphic += data.viewport.x;
                }
                x_view = data.camera.tile.x - 4;
                for (int x = 0; x < data.viewport.width_tiles + 7; x++) {
                    if (x_view >= 0 && x_view < VIEW_X_MAX) {
                        int grid_offset = view_to_grid_offset_lookup[x_view][y_view];
                        if (grid_offset >= 0) {
                            callback3(x_graphic, y_graphic, grid_offset);
                        }
                    }
                    x_graphic += TILE_WIDTH_PIXELS;
                    x_view++;
                }
            }
        }
        odd = 1 - odd;
        y_graphic += HALF_TILE_HEIGHT_PIXELS;
        y_view++;
    }
}

static void do_valid_callback(int view_x, int view_y, int grid_offset, map_callback *callback)
{
    if (grid_offset >= 0 && map_image_at(grid_offset) >= 6) {
        callback(view_x, view_y, grid_offset);
    }
}

void city_view_foreach_tile_in_range(int grid_offset, int size, int radius, map_callback *callback)
{
    int x, y;
    city_view_grid_offset_to_xy_view(grid_offset, &x, &y);
    x = (x - data.camera.tile.x) * TILE_WIDTH_PIXELS - (y & 1) * HALF_TILE_WIDTH_PIXELS - data.camera.pixel.x + data.viewport.x;
    y = (y - data.camera.tile.y - 1) * HALF_TILE_HEIGHT_PIXELS - data.camera.pixel.y + data.viewport.y;
    grid_offset += map_grid_delta(1, 1);

    int x_delta = HALF_TILE_WIDTH_PIXELS;
    int y_delta = HALF_TILE_HEIGHT_PIXELS;
    int x_offset = HALF_TILE_WIDTH_PIXELS;
    int y_offset = TILE_HEIGHT_PIXELS;
    if (size) {
        --size;
        y += HALF_TILE_HEIGHT_PIXELS * size;
        x_offset += HALF_TILE_WIDTH_PIXELS * size;
        y_offset += HALF_TILE_HEIGHT_PIXELS * size;
    } else {
        do_valid_callback(x, y, grid_offset, callback);
    }
    for (int ring = 0; ring < radius; ++ring) {
        int offset_north = -ring - 2;
        int offset_south = ring + size;
        do_valid_callback(x, y + y_offset, map_grid_add_delta(grid_offset, ring + size, ring + size), callback);
        do_valid_callback(x, y - y_offset, map_grid_add_delta(grid_offset, -ring - 2, -ring - 2), callback);
        do_valid_callback(x - x_offset - x_delta, y, map_grid_add_delta(grid_offset, -ring - 2, ring + size), callback);
        do_valid_callback(x + x_offset + x_delta, y, map_grid_add_delta(grid_offset, ring + size, -ring - 2), callback);
        for (int tile = 1; tile < ring * 2 + size + 2; ++tile) {
            do_valid_callback(x + x_delta * tile, y - y_offset + y_delta * tile,
                map_grid_add_delta(grid_offset, tile + offset_north, offset_north), callback);
            do_valid_callback(x - x_delta * tile, y - y_offset + y_delta * tile,
                map_grid_add_delta(grid_offset, offset_north, tile + offset_north), callback);
            do_valid_callback(x + x_delta * tile, y + y_offset - y_delta * tile,
                map_grid_add_delta(grid_offset, offset_south, offset_south - tile), callback);
            do_valid_callback(x - x_delta * tile, y + y_offset - y_delta * tile,
                map_grid_add_delta(grid_offset, offset_south - tile, offset_south), callback);
        }
        x_offset += TILE_WIDTH_PIXELS;
        y_offset += TILE_HEIGHT_PIXELS;
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
