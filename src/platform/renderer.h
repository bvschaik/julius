#ifndef PLATFORM_RENDERER_H
#define PLATFORM_RENDERER_H

#include "graphics/color.h"

#include "SDL.h"

int platform_renderer_init(SDL_Window *window);

int platform_renderer_create_render_texture(int width, int height);

int platform_renderer_lost_render_texture(void);

void platform_renderer_invalidate_target_textures(void);

void platform_renderer_generate_mouse_cursor_texture(int cursor_id, int size, const color_t *pixels,
    int hotspot_x, int hotspot_y);

void platform_renderer_clear(void);

void platform_renderer_render(void);

void platform_renderer_pause(void);

void platform_renderer_resume(void);

void platform_renderer_destroy(void);

#endif // PLATFORM_RENDERER_H
