#include "game/system.h"
#include "graphics/color.h"
#include "input/cursor.h"
#include "platform/screen.h"

#include "SDL.h"

static struct {
    SDL_Cursor *cursors[CURSOR_MAX];
    SDL_Surface *surfaces[CURSOR_MAX];
    cursor_shape current_shape;
    cursor_scale current_scale;
} data;

static const color_t mouse_colors[] = {
    ALPHA_TRANSPARENT,
    ALPHA_TRANSPARENT,
    ALPHA_TRANSPARENT,
    ALPHA_OPAQUE | COLOR_BLACK,
    ALPHA_OPAQUE | COLOR_MOUSE_DARK_GRAY,
    ALPHA_OPAQUE | COLOR_MOUSE_MEDIUM_GRAY,
    ALPHA_OPAQUE | COLOR_MOUSE_LIGHT_GRAY,
    ALPHA_OPAQUE | COLOR_WHITE
};

static SDL_Surface *generate_cursor_surface(const char *data, int width, int height)
{
    // make sure the cursor is a power of two
    int size = 32;
    while (size <= width || size <= height) {
        size *= 2;
    }
    SDL_Surface *cursor_surface =
        SDL_CreateRGBSurface(0, size, size, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    color_t *pixels = cursor_surface->pixels;
    SDL_memset(pixels, 0, sizeof(color_t) * size * size);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixels[y * size + x] = mouse_colors[data[y * width + x] - 32];
        }
    }
    return cursor_surface;
}

static cursor_scale get_cursor_scale(int scale_percentage)
{
    if (scale_percentage <= 100) {
        return CURSOR_SCALE_1;
    } else if (scale_percentage <= 150) {
        return CURSOR_SCALE_1_5;
    } else {
        return CURSOR_SCALE_2;
    }
}

void system_init_cursors(int scale_percentage)
{
    data.current_scale = get_cursor_scale(scale_percentage);
    for (int i = 0; i < CURSOR_MAX; i++) {
        const cursor *c = input_cursor_data(i, data.current_scale);
        if (data.surfaces[i]) {
            SDL_FreeSurface(data.surfaces[i]);
        }
        if (data.cursors[i]) {
            SDL_FreeCursor(data.cursors[i]);
        }
        data.surfaces[i] = generate_cursor_surface(c->data, c->width, c->height);
        if (!system_use_software_cursor()) {
            data.cursors[i] = SDL_CreateColorCursor(data.surfaces[i], c->hotspot_x, c->hotspot_y);
        } else {
            SDL_ShowCursor(SDL_DISABLE);
            platform_screen_generate_mouse_cursor_texture(i, data.current_scale, data.surfaces[i]->pixels);
        }
    }
    system_set_cursor(data.current_shape);
}

void system_set_cursor(int cursor_id)
{
    data.current_shape = cursor_id;
    if (!system_use_software_cursor()) {
        SDL_SetCursor(data.cursors[cursor_id]);
    }
}

int system_use_software_cursor(void)
{
#if defined(__SWITCH__) || defined(__vita__)
    return 1;
#else
    return 0;
#endif
}

cursor_shape system_get_current_cursor_shape(void)
{
    return data.current_shape;
}

cursor_scale system_get_current_cursor_scale(void)
{
    return data.current_scale;
}
