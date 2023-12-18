#include "view.h"

#include "building/construction.h"
#include "core/calc.h"
#include "core/config.h"
#include "core/direction.h"
#include "editor/editor.h"
#include "graphics/menu.h"
#include "graphics/renderer.h"
#include "map/grid.h"
#include "map/image.h"
#include "widget/minimap.h"

#define TILE_WIDTH_PIXELS 60
#define TILE_HEIGHT_PIXELS 30
#define HALF_TILE_WIDTH_PIXELS 30
#define HALF_TILE_HEIGHT_PIXELS 15

static const int X_DIRECTION_FOR_ORIENTATION[] = {1,  1, -1, -1};
static const int Y_DIRECTION_FOR_ORIENTATION[] = {1, -1, -1,  1};

static struct {
    int screen_width;
    int screen_height;
    int sidebar_collapsed;
    int orientation;
    int scale;
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
    int max_scale = city_view_get_max_scale();
    if (max_scale < data.scale) {
        city_view_set_scale(max_scale);
        return;
    }
    int grid_height = map_grid_height() * 2;
    int x_min = (VIEW_X_MAX - map_grid_width()) / 2;
    int y_min = (VIEW_Y_MAX - grid_height) / 2;
    if (data.viewport.width_tiles >= map_grid_width() + 4) {
        data.camera.tile.x = x_min - 1 - (data.viewport.width_tiles - map_grid_width()) / 2;
        data.camera.pixel.x = TILE_WIDTH_PIXELS -
            ((calc_adjust_with_percentage(data.viewport.width_pixels + 2, data.scale) / 2) % TILE_WIDTH_PIXELS);
    } else {
        if (data.camera.tile.x < x_min - 1) {
            data.camera.tile.x = x_min - 1;
            data.camera.pixel.x = 0;
        }
        int max_x_tile = VIEW_X_MAX - x_min - data.viewport.width_tiles;
        int max_x_pixel = TILE_WIDTH_PIXELS -
            (calc_adjust_with_percentage(data.viewport.width_pixels + 2, data.scale) % TILE_WIDTH_PIXELS);
        if (data.camera.tile.x > max_x_tile || (data.camera.tile.x == max_x_tile && data.camera.pixel.x > max_x_pixel)) {
            data.camera.tile.x = max_x_tile;
            data.camera.pixel.x = max_x_pixel;
        }
    }
    if (data.viewport.height_tiles >= grid_height + 4) {
        data.camera.tile.y = (y_min - (data.viewport.height_tiles - grid_height) / 2) & ~1;
        data.camera.pixel.y = TILE_HEIGHT_PIXELS -
            (((calc_adjust_with_percentage(data.viewport.height_pixels, data.scale) + TILE_HEIGHT_PIXELS) / 2) %
            TILE_HEIGHT_PIXELS);
    } else {    
        if (data.camera.tile.y < y_min - 2) {
            data.camera.tile.y = y_min - 1;
            data.camera.pixel.y = 0;
        }
        int max_y_tile = (VIEW_Y_MAX - y_min - data.viewport.height_tiles) & ~1;
        int max_y_pixel = TILE_HEIGHT_PIXELS -
                (calc_adjust_with_percentage(data.viewport.height_pixels, data.scale) % TILE_HEIGHT_PIXELS);
        if (data.camera.tile.y > max_y_tile || (data.camera.tile.y == max_y_tile && data.camera.pixel.y > max_y_pixel)) {
            data.camera.tile.y = max_y_tile;
            data.camera.pixel.y = max_y_pixel;
        }
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

void city_view_set_custom_lookup(int start_offset, int width, int height, int border_size)
{
    reset_lookup();

    int start_x = border_size / 2;
    int end_x = GRID_SIZE - start_x;
    int start_y = (start_offset - start_x) / GRID_SIZE;
    int end_y = start_y + height;

    int x_view_start = VIEW_X_MAX - 1 - start_y;
    int y_view_start = 1 + start_y;

    for (int y = start_y; y < end_y; y++) {
        int x_view = x_view_start + start_x;
        int y_view = y_view_start + start_x;
        for (int x = start_x; x < end_x; x++) {
            view_to_grid_offset_lookup[x_view / 2][y_view] = x + GRID_SIZE * y;
            x_view++;
            y_view++;
        }
        x_view_start--;
        y_view_start++;
    }
}

void city_view_restore_lookup(void)
{
    calculate_lookup();
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
    city_view_set_scale(100);
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

int city_view_get_scale(void)
{
    return data.scale;
}

int city_view_get_max_scale(void)
{
    int max_x_pixels = (map_grid_width() + 4) * TILE_WIDTH_PIXELS;
    int max_y_pixels = (map_grid_height() * 2 + 4) * HALF_TILE_HEIGHT_PIXELS;

    int max_x_scale = calc_percentage(max_x_pixels, data.viewport.width_pixels);
    int max_y_scale = calc_percentage(max_y_pixels, data.viewport.height_pixels);
    int max_scale = max_x_scale > max_y_scale ? max_x_scale : max_y_scale;

    return max_scale < 100 ? 100 : max_scale;
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

static void adjust_for_orientation(int x, int y, int orientation, int *x_out, int *y_out)
{
    switch (orientation) {
        default:
        case DIR_0_TOP:
            *x_out = x;
            *y_out = y;
            break;
        case DIR_2_RIGHT:
            *x_out = y / 2;
            *y_out = (VIEW_X_MAX - x) * 2;
            break;
        case DIR_4_BOTTOM:
            *x_out = VIEW_X_MAX - x;
            *y_out = VIEW_Y_MAX - y;
            break;
        case DIR_6_LEFT:
            *x_out = (VIEW_Y_MAX - y) / 2;
            *y_out = x * 2;
            break;
    }
}

void city_view_get_camera_absolute(int *x_abs, int *y_abs)
{
    int x_offset = data.viewport.width_tiles / 2;
    int y_offset = data.viewport.height_tiles / 2;
    int x_center = data.camera.tile.x + x_offset;
    int y_center = data.camera.tile.y + y_offset;
    int x_center_abs, y_center_abs;
    int to_rotate = (DIR_8_NONE - data.orientation) % DIR_8_NONE;
    adjust_for_orientation(x_center, y_center, to_rotate, &x_center_abs, &y_center_abs);
    *x_abs = x_center_abs - x_offset;
    *y_abs = y_center_abs - y_offset;
}

static void get_screen_pixel_position_for_view_tile(const view_tile *tile, int *x_pixels, int *y_pixels, int scale)
{
    int x_view_offset = tile->x - data.camera.tile.x;
    int y_view_offset = tile->y - data.camera.tile.y;
    int inverted_scale = scale ? calc_percentage(100, data.scale) : 100;

    *x_pixels = data.viewport.x + calc_adjust_with_percentage(TILE_WIDTH_PIXELS * x_view_offset - data.camera.pixel.x,
        inverted_scale);
    if (y_view_offset & 1) {
        *x_pixels -= calc_adjust_with_percentage(HALF_TILE_WIDTH_PIXELS, inverted_scale);
    }
    *y_pixels = data.viewport.y + calc_adjust_with_percentage(HALF_TILE_HEIGHT_PIXELS * y_view_offset
        - HALF_TILE_HEIGHT_PIXELS - data.camera.pixel.y, inverted_scale);
}

static int objective_is_obstructed(const pixel_area *objective, const pixel_area *obstruction)
{
    return obstruction->x < objective->x + objective->width && obstruction->x + obstruction->width > objective->x &&
        obstruction->y < objective->y + objective->height && obstruction->y + obstruction->height > objective->y;
}

void city_view_adjust_camera_from_obstruction(int grid_offset, int size, const pixel_area *obstruction)
{
    int inverted_scale = calc_percentage(100, data.scale);
    int objective_y_offset = calc_adjust_with_percentage(size * HALF_TILE_HEIGHT_PIXELS, inverted_scale);

    view_tile tile;
    city_view_grid_offset_to_xy_view(grid_offset, &tile.x, &tile.y);
    pixel_area objective;
    get_screen_pixel_position_for_view_tile(&tile, &objective.x, &objective.y, 1);
    objective.y -= objective_y_offset;
    objective.width = calc_adjust_with_percentage(size * TILE_WIDTH_PIXELS, inverted_scale);
    objective.height = calc_adjust_with_percentage(size * TILE_HEIGHT_PIXELS, inverted_scale);

    if (!objective_is_obstructed(&objective, obstruction)) {
        return;
    }
    
    enum {
        LEFT = 0,
        RIGHT = 1,
        TOP = 2,
        BOTTOM = 3,
        NUM_SIDES = 4
    };

    int space[NUM_SIDES];

    space[LEFT] = obstruction->x - data.viewport.x - objective.width;
    space[RIGHT] = data.viewport.width_pixels - (obstruction->x + obstruction->width) - objective.width;
    space[TOP] = obstruction->y - data.viewport.y - objective.height;
    space[BOTTOM] = data.viewport.height_pixels - (obstruction->y + obstruction->height) - objective.height;

    // No space to show the objective behind the window anywhere - just keep the camera centered
    if (space[LEFT] < 0 && space[RIGHT] < 0 && space[TOP] < 0 && space[BOTTOM] < 0) {
        return;
    }

    int sides[NUM_SIDES] = { LEFT, RIGHT, TOP, BOTTOM };

    // Sort obstruction sides in descending order of space left
    for (int i = 1; i < NUM_SIDES; i++) {
        int key = sides[i];
        int j = i - 1;

        while (j >= 0 && space[sides[j]] < space[key]) {
            sides[j + 1] = sides[j];
            j--;
        }

        sides[j + 1] = key;
    }

    int scaled_tile_width_pixels = calc_adjust_with_percentage(TILE_WIDTH_PIXELS, inverted_scale);
    int scaled_half_tile_height_pixels = calc_adjust_with_percentage(HALF_TILE_HEIGHT_PIXELS, inverted_scale);

    // Position camera, starting from the side with the most space to the side with the least space
    for (int i = 0; i < NUM_SIDES; i++) {
        int side = sides[i];

        if (space[side] < 0) {
            return;
        }

        if (side == LEFT) {
            data.camera.tile.x = tile.x - (obstruction->x - data.viewport.x) / scaled_tile_width_pixels / 2;
            data.camera.pixel.x = 20;
        } else if (side == RIGHT) {
            int start_x = obstruction->x + obstruction->width;
            int viewable_width = data.viewport.width_pixels - start_x;
            data.camera.tile.x = tile.x - 1 - (start_x + viewable_width / 2) / scaled_tile_width_pixels;
        } else if (side == TOP) {
            data.camera.tile.y = tile.y - 1 - (obstruction->y - data.viewport.y) / scaled_half_tile_height_pixels / 2;
            data.camera.tile.y &= ~1;
        } else { // Bottom
            int start_y = obstruction->y + obstruction->height;
            int viewable_height = data.viewport.height_pixels - start_y;
            data.camera.tile.y = tile.y - 1 - (start_y + viewable_height / 2) / scaled_half_tile_height_pixels;
        }

        check_camera_boundaries();
        get_screen_pixel_position_for_view_tile(&tile, &objective.x, &objective.y, 1);
        objective.y -= objective_y_offset;
        if (!objective_is_obstructed(&objective, obstruction)) {
            return;
        }
        // Recenter objective
        city_view_go_to_grid_offset(grid_offset);
    }
}

void city_view_set_camera(int x, int y)
{
    data.camera.tile.x = x;
    data.camera.tile.y = y;
    check_camera_boundaries();
}

void city_view_set_camera_absolute(int x_abs, int y_abs)
{
    int x_offset = data.viewport.width_tiles / 2;
    int y_offset = data.viewport.height_tiles / 2;
    int x_center_abs = x_abs + x_offset;
    int y_center_abs = y_abs + y_offset;
    int x_center, y_center;
    adjust_for_orientation(x_center_abs, y_center_abs, data.orientation, &x_center, &y_center);
    city_view_set_camera(x_center - x_offset, y_center - y_offset);
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

void city_view_scroll(int x, int y)
{
    data.camera.pixel.x += x;
    data.camera.pixel.y += y;
    adjust_camera_position_for_pixels();
    check_camera_boundaries();
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

    x_pixels = calc_adjust_with_percentage(x_pixels, data.scale);
    y_pixels = calc_adjust_with_percentage(y_pixels, data.scale);

    x_pixels += data.camera.pixel.x;
    y_pixels += data.camera.pixel.y;
    int odd = ((x_pixels - data.viewport.x) / HALF_TILE_WIDTH_PIXELS +
        (y_pixels - data.viewport.y) / HALF_TILE_HEIGHT_PIXELS) & 1;
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
    return tile->x >= 0 && tile->x < VIEW_X_MAX && tile->y >= 0 && tile->y < VIEW_Y_MAX;
}

void city_view_set_selected_view_tile(const view_tile *tile)
{
    get_screen_pixel_position_for_view_tile(tile, &data.selected_tile.x_pixels, &data.selected_tile.y_pixels, 0);
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
    data.camera.pixel.x = 0;
    data.camera.pixel.y = 0;
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
    data.viewport.width_tiles = calc_adjust_with_percentage(width, data.scale) / TILE_WIDTH_PIXELS;
    data.viewport.height_tiles = calc_adjust_with_percentage(height, data.scale) / HALF_TILE_HEIGHT_PIXELS;
}

static void set_viewport_with_sidebar(void)
{
    set_viewport(0, TOP_MENU_HEIGHT, data.screen_width - 160, data.screen_height - TOP_MENU_HEIGHT);
}

static void set_viewport_without_sidebar(void)
{
    set_viewport(0, TOP_MENU_HEIGHT, data.screen_width - 40, data.screen_height - TOP_MENU_HEIGHT);
}

void city_view_set_scale(int scale)
{
    scale = calc_bound(scale, 50, city_view_get_max_scale());
    data.scale = scale;
    if (data.sidebar_collapsed) {
        set_viewport_without_sidebar();
    } else {
        set_viewport_with_sidebar();
    }
    check_camera_boundaries();
    graphics_renderer()->update_scale(scale);
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

void city_view_foreach_valid_map_tile(map_callback *callback)
{
    int odd = 0;
    int y_view = data.camera.tile.y - 8;
    int y_graphic = data.viewport.y - 9 * HALF_TILE_HEIGHT_PIXELS - data.camera.pixel.y;
    for (int y = 0; y < data.viewport.height_tiles + 21; y++) {
        if (y_view >= 0 && y_view < VIEW_Y_MAX) {
            int x_graphic = -(6 * TILE_WIDTH_PIXELS) - data.camera.pixel.x;
            if (odd) {
                x_graphic += data.viewport.x - HALF_TILE_WIDTH_PIXELS;
            } else {
                x_graphic += data.viewport.x;
            }
            int x_view = data.camera.tile.x - 6;
            for (int x = 0; x < data.viewport.width_tiles + 9; x++) {
                if (x_view >= 0 && x_view < VIEW_X_MAX) {
                    int grid_offset = view_to_grid_offset_lookup[x_view][y_view];
                    if (grid_offset >= 0) {
                        callback(x_graphic, y_graphic, grid_offset);
                    }
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

void city_view_foreach_valid_map_tile_row(map_callback *callback1, map_callback *callback2, map_callback *callback3)
{
    int odd = 0;
    int y_view = data.camera.tile.y - 8;
    int y_graphic = data.viewport.y - 9 * HALF_TILE_HEIGHT_PIXELS - data.camera.pixel.y;
    int x_graphic, x_view;
    for (int y = 0; y < data.viewport.height_tiles + 21; y++) {
        if (y_view >= 0 && y_view < VIEW_Y_MAX) {
            if (callback1) {
                x_graphic = -(6 * TILE_WIDTH_PIXELS) - data.camera.pixel.x;
                if (odd) {
                    x_graphic += data.viewport.x - HALF_TILE_WIDTH_PIXELS;
                } else {
                    x_graphic += data.viewport.x;
                }
                x_view = data.camera.tile.x - 6;
                for (int x = 0; x < data.viewport.width_tiles + 9; x++) {
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
                x_graphic = -(6 * TILE_WIDTH_PIXELS) - data.camera.pixel.x;
                if (odd) {
                    x_graphic += data.viewport.x - HALF_TILE_WIDTH_PIXELS;
                } else {
                    x_graphic += data.viewport.x;
                }
                x_view = data.camera.tile.x - 6;
                for (int x = 0; x < data.viewport.width_tiles + 9; x++) {
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
                x_graphic = -(6 * TILE_WIDTH_PIXELS) - data.camera.pixel.x;
                if (odd) {
                    x_graphic += data.viewport.x - HALF_TILE_WIDTH_PIXELS;
                } else {
                    x_graphic += data.viewport.x;
                }
                x_view = data.camera.tile.x - 6;
                for (int x = 0; x < data.viewport.width_tiles + 9; x++) {
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
    x = (x - data.camera.tile.x) * TILE_WIDTH_PIXELS
        - (y & 1) * HALF_TILE_WIDTH_PIXELS - data.camera.pixel.x + data.viewport.x;
    y = (y - data.camera.tile.y - 1) * HALF_TILE_HEIGHT_PIXELS - data.camera.pixel.y + data.viewport.y;
    int orientation_x = X_DIRECTION_FOR_ORIENTATION[data.orientation / 2];
    int orientation_y = Y_DIRECTION_FOR_ORIENTATION[data.orientation / 2];

    // If we are rotated east or west, the pixel location needs to be rotated
    // to match its corresponding grid_offset. Since for east and west
    // only one of the orientations is negative, we can get a negative value
    // which can then be used to properly offset the pixel positions
    int pixel_rotation = orientation_x * orientation_y;

    int rotation_delta = pixel_rotation == -1 ? (2 - size) : 1;
    grid_offset += map_grid_delta(rotation_delta * orientation_x, rotation_delta * orientation_y);
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
    // Basic algorithm: we cycle the radius as successive rings
    // Starting at the innermost ring (determined by size), we first cycle
    // the top, left, right and bottom corners of the ring.
    // Then we stretch from each corner of the ring to reach the next one, closing the ring
    for (int ring = 0; ring < radius; ++ring) {
        int offset_north = -ring - 2;
        int offset_south = ring + size;
        do_valid_callback(
            x, y + y_offset * pixel_rotation,
            map_grid_add_delta(grid_offset, offset_south * orientation_x, offset_south * orientation_y),
            callback);
        do_valid_callback(
            x, y - y_offset * pixel_rotation,
            map_grid_add_delta(grid_offset, offset_north * orientation_x, offset_north * orientation_y),
            callback);
        do_valid_callback(
            x - x_offset - x_delta, y,
            map_grid_add_delta(grid_offset, offset_north * orientation_x, offset_south * orientation_y),
            callback);
        do_valid_callback(
            x + x_offset + x_delta, y,
            map_grid_add_delta(grid_offset, offset_south * orientation_x, offset_north * orientation_y),
            callback);
        for (int tile = 1; tile < ring * 2 + size + 2; ++tile) {
            do_valid_callback(
                x + x_delta * tile, y - y_offset * pixel_rotation + y_delta * pixel_rotation * tile,
                map_grid_add_delta(grid_offset, (tile + offset_north) * orientation_x, offset_north * orientation_y),
                callback);
            do_valid_callback(
                x - x_delta * tile, y - y_offset * pixel_rotation + y_delta * pixel_rotation * tile,
                map_grid_add_delta(grid_offset, offset_north * orientation_x, (tile + offset_north) * orientation_y),
                callback);
            do_valid_callback(
                x + x_delta * tile, y + y_offset * pixel_rotation - y_delta * pixel_rotation * tile,
                map_grid_add_delta(grid_offset, offset_south * orientation_x, (offset_south - tile) * orientation_y),
                callback);
            do_valid_callback(
                x - x_delta * tile, y + y_offset * pixel_rotation - y_delta * pixel_rotation * tile,
                map_grid_add_delta(grid_offset, (offset_south - tile) * orientation_x, offset_south * orientation_y),
                callback);
        }
        x_offset += TILE_WIDTH_PIXELS;
        y_offset += TILE_HEIGHT_PIXELS;
    }
}

void city_view_foreach_minimap_tile(
    int x_offset, int y_offset,
    int absolute_x, int absolute_y,
    int width_tiles, int height_tiles,
    map_callback *callback)
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
