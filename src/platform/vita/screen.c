#include "platform/screen.h"

#include "core/calc.h"
#include "input/mouse.h"
#include "game/settings.h"
#include "game/system.h"
#include "graphics/graphics.h"
#include "graphics/screen.h"

#include <vita2d.h>
#include "vita.h"

// SDL_log
#include <SDL.h>
static SDL_Renderer *my_renderer;
static SDL_Window *my_window;

vita2d_texture *tex_buffer = NULL;

int platform_screen_create(const char *title, int display_scale_percentage)
{
    if (!my_window) {
        my_window = SDL_CreateWindow("Dummy window used only as event listener",
            0, 0, VITA_DISPLAY_WIDTH, VITA_DISPLAY_HEIGHT, 0);
    }
    if (!my_renderer) {
        my_renderer = SDL_CreateRenderer(my_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    }
    SDL_Log("Creating empty texture");
    tex_buffer = vita2d_create_empty_texture_format(
        VITA_DISPLAY_WIDTH, VITA_DISPLAY_HEIGHT, SCE_GXM_TEXTURE_FORMAT_X8U8U8U8_1RGB);
    SDL_Log("Creating empty texture: done");

    return platform_screen_resize(VITA_DISPLAY_WIDTH, VITA_DISPLAY_HEIGHT);
}

void platform_screen_destroy(void)
{
    if (my_renderer) {
        SDL_DestroyRenderer(my_renderer);
    }
    if (my_window) {
        SDL_DestroyWindow(my_window);
    }
}

int platform_screen_resize(int width, int height)
{
    screen_set_resolution(width, height);
    return 1;
}

void platform_screen_move(int x, int y)
{}

void platform_screen_set_fullscreen(void)
{
    SDL_Log("User to fullscreen");
    setting_set_display(1, VITA_DISPLAY_WIDTH, VITA_DISPLAY_HEIGHT);
}

void platform_screen_set_windowed(void)
{
    int width, height;
    setting_window(&width, &height);
    SDL_Log("User to windowed");
    setting_set_display(0, width, height);
}

void platform_screen_set_window_size(int width, int height)
{
    SDL_Log("User resize to");
    setting_set_display(0, width, height);
}

void platform_screen_center_window(void)
{}

void platform_screen_render(void)
{
    vita2d_start_drawing();
    vita2d_draw_texture(tex_buffer, 0, 0);
    const mouse *mouse = mouse_get();
    if (!mouse->is_touch) {
        vita2d_draw_texture(current_cursor->texture,
            mouse->x - current_cursor->hotspot_x, mouse->y - current_cursor->hotspot_y);
    }
    vita2d_end_drawing();
    vita2d_wait_rendering_done();
    vita2d_swap_buffers();
}

void system_set_mouse_position(int *x, int *y)
{
    *x = calc_bound(*x, 0, VITA_DISPLAY_WIDTH - 1);
    *y = calc_bound(*y, 0, VITA_DISPLAY_HEIGHT - 1);
    SDL_WarpMouseInWindow(my_window, *x, *y);
}

int system_is_fullscreen_only(void)
{
    return 1;
}
