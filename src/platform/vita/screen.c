#include "platform/screen.h"

#include "city/view.h"
#include "core/config.h"
#include "input/mouse.h"
#include "game/settings.h"
#include "game/system.h"
#include "graphics/graphics.h"
#include "graphics/menu.h"
#include "graphics/screen.h"

#include <vita2d.h>
#include "vita.h"

// SDL_log
#include <SDL.h>
static SDL_Renderer *my_renderer;
static SDL_Window *my_window;

SDL_Rect city_texture_position;

vita2d_texture *tex_buffer_ui = NULL;
vita2d_texture *tex_buffer_city = NULL;

static void create_textures(void)
{
    SDL_Log("Creating empty textures\n");
    if (!tex_buffer_ui) {
        tex_buffer_ui = vita2d_create_empty_texture_format(VITA_DISPLAY_WIDTH, VITA_DISPLAY_HEIGHT, SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ARGB);
    }
    if (config_get(CONFIG_UI_ZOOM) && !tex_buffer_city) {
        tex_buffer_city = vita2d_create_empty_texture_format(VITA_DISPLAY_WIDTH * 2, VITA_DISPLAY_HEIGHT * 2, SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ARGB);
    }
    SDL_Log("Creating empty textures: done\n");
}

int platform_screen_create(const char *title, int display_scale_percentage)
{
    if (!my_window) {
        my_window = SDL_CreateWindow("Dummy window used only as event listener", 0, 0, VITA_DISPLAY_WIDTH, VITA_DISPLAY_HEIGHT, 0);
    }
    if (!my_renderer) {
        my_renderer = SDL_CreateRenderer(my_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    }
    SDL_Log("Creating empty texture\n");
    tex_buffer = vita2d_create_empty_texture_format(VITA_DISPLAY_WIDTH, VITA_DISPLAY_HEIGHT, SCE_GXM_TEXTURE_FORMAT_X8U8U8U8_1RGB);
    SDL_Log("Creating empty texture: done\n");

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
    SDL_Log("User to fullscreen\n");
    setting_set_display(1, VITA_DISPLAY_WIDTH, VITA_DISPLAY_HEIGHT);
}

void platform_screen_set_windowed(void)
{
    int width, height;
    setting_window(&width, &height);
    SDL_Log("User to windowed \n");
    setting_set_display(0, width, height);
}

void platform_screen_set_window_size(int width, int height)
{
    SDL_Log("User resize to \n");
    setting_set_display(0, width, height);
}

void platform_screen_center_window(void)
{}

void platform_screen_render(void)
{
    vita2d_start_drawing();
    vita2d_clear_screen();
    if (config_get(CONFIG_UI_ZOOM)) {
        city_view_get_scaled_viewport(&city_texture_position.x, &city_texture_position.y,
            &city_texture_position.w, &city_texture_position.h);
        float scale = city_view_get_scale() / 100.0f;
        scale = 1 / scale;
        vita2d_draw_texture_part_scale(tex_buffer_city, 0, TOP_MENU_HEIGHT,
            city_texture_position.x, city_texture_position.y,
            city_texture_position.w, city_texture_position.h,
            scale, scale);
    }
    vita2d_draw_texture(tex_buffer_ui, 0, 0);

    const mouse *mouse = mouse_get();
    vita2d_draw_texture(current_cursor->texture, mouse->x - current_cursor->hotspot_x, mouse->y - current_cursor->hotspot_y);
    vita2d_end_drawing();
    vita2d_wait_rendering_done();
    vita2d_swap_buffers();
}

void platform_screen_warp_mouse(int x, int y)
{
    SDL_WarpMouseInWindow(my_window, x, y);
}

int system_is_fullscreen_only(void)
{
    return 1;
}

void system_reload_textures(void)
{
    create_textures();
    screen_set_resolution(VITA_DISPLAY_WIDTH, VITA_DISPLAY_HEIGHT);
}

int system_save_screen_buffer(void *pixels)
{
    unsigned int *buffer = vita2d_get_current_fb();
    memcpy(pixels, buffer, sizeof(unsigned int) * VITA_DISPLAY_WIDTH * VITA_DISPLAY_HEIGHT);
    return 1;
}
