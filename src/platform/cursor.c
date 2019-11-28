#include "game/system.h"
#include "input/cursor.h"

#include "SDL.h"

static SDL_Cursor* cursors[CURSOR_MAX];
static SDL_Surface* cursor_surfaces[CURSOR_MAX];
static int current_cursor_id = CURSOR_ARROW;

static const SDL_Color mouse_colors[] = {
    { 0x00, 0x00, 0x00, 0x00 }, /* Transparent */
    { 0x00, 0x00, 0x00, 0xFF }, /* Black */
    { 0x3F, 0x3F, 0x3F, 0xFF }, /* Dark gray */
    { 0x73, 0x73, 0x73, 0xFF }, /* Medium gray */
    { 0xB3, 0xB3, 0xB3, 0xFF }, /* Light gray */
    { 0xFF, 0xFF, 0xFF, 0xFF }  /* White */
};

static SDL_Surface* generate_cursor_surface(const char* data, int width, int height)
{
    SDL_Surface* cursor_surface = SDL_CreateRGBSurfaceFrom((void*)data, width, height, 8, sizeof(Uint8) * width, 0, 0, 0, 0);

    SDL_LockSurface(cursor_surface);
    SDL_SetPaletteColors(cursor_surface->format->palette, mouse_colors, ' ', 1);
    SDL_SetPaletteColors(cursor_surface->format->palette, &mouse_colors[1], '#', 5);
    SDL_UnlockSurface(cursor_surface);

    return cursor_surface;
}

cursor_scale get_cursor_scale(int scale_percentage)
{
    if (scale_percentage <= 100) {
        return CURSOR_SCALE_1;
    } else if (scale_percentage <= 150) {
        return CURSOR_SCALE_1_5;
    } else {
        return CURSOR_SCALE_2;
    }
}

void platform_init_cursors(int scale_percentage)
{
    cursor_scale cur_scale = get_cursor_scale(scale_percentage);
    for (int i = 0; i < CURSOR_MAX; i++) {
        const cursor* c = input_cursor_data(i, cur_scale);
        cursor_surfaces[i] = generate_cursor_surface(c->data, c->width, c->height);
        cursors[i] = SDL_CreateColorCursor(cursor_surfaces[i], c->hotspot_x, c->hotspot_y);
    }
    system_set_cursor(current_cursor_id);
}

void system_set_cursor(int cursor_id)
{
    current_cursor_id = cursor_id;
    SDL_SetCursor(cursors[cursor_id]);
}
