#include "city/view.h"
#include "core/string.h"
#include "game/game.h"
#include "graphics/graphics.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "map/road_network.h"
#include "map/routing.h"
#include "map/terrain.h"
#include "platform/renderer.h"

#include "SDL.h"

#ifdef DRAW_FPS
static struct {
    int frame_count;
    int last_fps;
    Uint32 last_update_time;
} fps = { 0, 0, 0 };
#endif

#ifdef DRAW_ROUTING
static void draw_routing(int x, int y, int grid_offset)
{
    int tx = map_grid_offset_to_x(grid_offset);
    int ty = map_grid_offset_to_y(grid_offset);
    map_routing_distance_grid *distance = map_routing_get_distance_grid();
    int16_t dist = distance->determined.items[grid_offset];
    if (!dist) {
        return;
    }
    if (tx == distance->dst_x && ty == distance->dst_y) {
        int dst_image_id = assets_get_image_id("UI", "Happy God Icon");
        image_draw(dst_image_id, x + 29 - 10, y + 15 - 10, 0, 1);
    }
    uint8_t text[20];
    string_from_int(text, dist, 0);
    text_draw_centered(text, x, y, 58, FONT_NORMAL_BLACK, COLOR_WHITE);
}
#endif

#ifdef DRAW_HIGHWAY_TERRAIN
static void draw_highway_terrain(int x, int y, int grid_offset)
{
    int terrain = map_terrain_get(grid_offset);
    if (terrain & TERRAIN_HIGHWAY_TOP_LEFT) {
        text_draw_centered("TL", x, y + 6, 58, FONT_NORMAL_BLACK, COLOR_WHITE);
    }
    if (terrain & TERRAIN_HIGHWAY_TOP_RIGHT) {
        text_draw_centered("TR", x, y + 6, 58, FONT_NORMAL_BLACK, COLOR_WHITE);
    }
    if (terrain & TERRAIN_HIGHWAY_BOTTOM_LEFT) {
        text_draw_centered("BL", x, y + 6, 58, FONT_NORMAL_BLACK, COLOR_WHITE);
    }
    if (terrain & TERRAIN_HIGHWAY_BOTTOM_RIGHT) {
        text_draw_centered("BR", x, y + 6, 58, FONT_NORMAL_BLACK, COLOR_WHITE);
    }
}
#endif

#ifdef DRAW_TILE_COORDS
static void draw_tile_coords(int x, int y, int grid_offset)
{
    int tx = map_grid_offset_to_x(grid_offset);
    int ty = map_grid_offset_to_y(grid_offset);
    uint8_t text[20];
    string_from_int(text, tx, 0);
    int len = string_length(text);
    string_copy(",", text + len, 2);
    len++;
    //string_from_int(text + len, ty, 0);
    //text_draw_centered(text, x, y + 4, 58, FONT_SMALL_PLAIN, COLOR_WHITE);
    string_from_int(text, grid_offset, 0);
    text_draw_centered(text, x, y + 10, 58, FONT_SMALL_PLAIN, COLOR_WHITE);
}
#endif

#ifdef DRAW_ROAD_NETWORK_IDS
static void draw_road_network_id(int x, int y, int grid_offset)
{
    int road_network_id = map_road_network_get(grid_offset);
    uint8_t text[20];
    string_from_int(text, road_network_id, 0);
    text_draw_centered(text, x, y + 4, 58, FONT_NORMAL_BLACK, COLOR_WHITE);
}
#endif

void debug_draw_city(void)
{
#ifdef DRAW_ROUTING
    city_view_foreach_valid_map_tile(draw_routing);
#endif
#ifdef DRAW_HIGHWAY_TERRAIN
    city_view_foreach_valid_map_tile(draw_highway_terrain);
#endif
#ifdef DRAW_TILE_COORDS
    city_view_foreach_valid_map_tile(draw_tile_coords);
#endif
#ifdef DRAW_ROAD_NETWORK_IDS
    city_view_foreach_valid_map_tile(draw_road_network_id);
#endif
}

#ifdef DRAW_FPS
void debug_run_and_draw(void)
{
    time_millis time_before_run = SDL_GetTicks();
    time_set_millis(time_before_run);

    game_run();
    Uint32 time_between_run_and_draw = SDL_GetTicks();
    game_draw();
    Uint32 time_after_draw = SDL_GetTicks();

    fps.frame_count++;
    if (time_after_draw - fps.last_update_time > 1000) {
        fps.last_fps = fps.frame_count;
        fps.last_update_time = time_after_draw;
        fps.frame_count = 0;
    }
    if (window_is(WINDOW_CITY) || window_is(WINDOW_CITY_MILITARY) || window_is(WINDOW_SLIDING_SIDEBAR)) {
        int y_offset = 24;
        int y_offset_text = y_offset + 5;
        graphics_fill_rect(0, y_offset, 100, 20, COLOR_WHITE);
        text_draw_number(fps.last_fps,
            'f', "", 5, y_offset_text, FONT_NORMAL_PLAIN, COLOR_FONT_RED);
        text_draw_number(time_between_run_and_draw - time_before_run,
            'g', "", 40, y_offset_text, FONT_NORMAL_PLAIN, COLOR_FONT_RED);
        text_draw_number(time_after_draw - time_between_run_and_draw,
            'd', "", 70, y_offset_text, FONT_NORMAL_PLAIN, COLOR_FONT_RED);
    }
    platform_renderer_render();
}
#endif
