#include "cursor.h"

#include "core/config.h"
#include "core/png_read.h"
#include "game/system.h"
#include "graphics/color.h"
#include "input/cursor.h"
#include "platform/renderer.h"
#include "platform/haiku/haiku.h"
#include "platform/switch/switch.h"
#include "platform/vita/vita.h"

#include "SDL.h"

#include <string.h>

static struct {
    SDL_Cursor *cursors[CURSOR_MAX];
    SDL_Surface *surfaces[CURSOR_MAX];
    cursor_shape current_shape;
    cursor_scale current_scale;
    int disabled;
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

static const cursor *get_valid_cursor(const cursor *c)
{
    const cursor *current = c;
    for (int i = 0; i < CURSOR_TYPE_MAX; i++, current++) {
        if (current->type == CURSOR_TYPE_PNG &&
            (!config_get(CONFIG_SCREEN_COLOR_CURSORS) || !png_load(current->data))) {
            continue;
        }
        return current;
    }
    return 0;
}

static SDL_Surface *generate_cursor_surface(const cursor *c)
{
    int size = platform_cursor_get_texture_size(c);
    SDL_Surface *cursor_surface =
        SDL_CreateRGBSurface(0, size, size, 32,
        COLOR_CHANNEL_RED, COLOR_CHANNEL_GREEN, COLOR_CHANNEL_BLUE, COLOR_CHANNEL_ALPHA);
    if (!cursor_surface) {
        return 0;
    }
    color_t *pixels = cursor_surface->pixels;
    SDL_memset(pixels, 0, sizeof(color_t) * size * size);
    if (c->type == CURSOR_TYPE_PNG) {
        if (!png_read(c->data, pixels, c->offset_x, c->offset_y,
                c->width, c->height, 0, 0, size, c->rotated)) {
            SDL_FreeSurface(cursor_surface);
        }
    } else {
        for (int y = 0; y < c->height; y++) {
            for (int x = 0; x < c->width; x++) {
                pixels[y * size + x] = mouse_colors[c->data[y * c->width + x] - 32];
            }
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
    data.disabled = 0;
    data.current_scale = get_cursor_scale(scale_percentage);
    for (int i = 0; i < CURSOR_MAX; i++) {
        const cursor *list = input_cursor_data(i, data.current_scale);
        if (data.surfaces[i]) {
            SDL_FreeSurface(data.surfaces[i]);
        }
        if (data.cursors[i]) {
            SDL_FreeCursor(data.cursors[i]);
        }
        const cursor *c = get_valid_cursor(list);
        data.surfaces[i] = generate_cursor_surface(c);
#ifndef PLATFORM_USE_SOFTWARE_CURSOR
        data.cursors[i] = SDL_CreateColorCursor(data.surfaces[i], c->hotspot_x, c->hotspot_y);
#else
        SDL_ShowCursor(SDL_DISABLE);
        platform_renderer_generate_mouse_cursor_texture(i, data.surfaces[i]->w, data.surfaces[i]->pixels,
            c->hotspot_x, c->hotspot_y);
#endif
    }
    system_set_cursor(data.current_shape);
}

void system_set_cursor(int cursor_id)
{
    data.current_shape = cursor_id;
#ifndef PLATFORM_USE_SOFTWARE_CURSOR
    SDL_SetCursor(data.cursors[cursor_id]);
#endif
}

void system_show_cursor(void)
{
    data.disabled = 0;
    SDL_ShowCursor(SDL_ENABLE);
}

void system_hide_cursor(void)
{
    data.disabled = 1;
    SDL_ShowCursor(SDL_DISABLE);
}

cursor_shape platform_cursor_get_current_shape(void)
{
    return data.disabled ? CURSOR_DISABLED : data.current_shape;
}

cursor_scale platform_cursor_get_current_scale(void)
{
    return data.current_scale;
}

int platform_cursor_get_texture_size(const cursor *c)
{
    int size = 32;
    while (size < c->width || size < c->height) {
        size *= 2;
    }
    return size;
}
