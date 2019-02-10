#include "platform/screen.h"
#include "input/mouse.h"
#include "game/settings.h"
#include "graphics/graphics.h"
#include "graphics/screen.h"

#include <vita2d.h>

// SDL_log
#include <SDL.h>

vita2d_texture *tex_buffer = NULL;

int platform_screen_create(const char *title)
{
    SDL_Log("Creating empty texture\n");
    tex_buffer = vita2d_create_empty_texture_format(960, 544, SCE_GXM_TEXTURE_FORMAT_X8U8U8U8_1RGB);
    SDL_Log("Creating empty texture: done\n");
 
    return platform_screen_resize(960, 544);
}

void platform_screen_destroy(void)
{
}

int platform_screen_resize(int width, int height)
{
    screen_set_resolution(width, height);
    return 1;
}

void platform_screen_set_fullscreen(void)
{
    SDL_Log("User to fullscreen\n");
    setting_set_display(1, 960, 544);
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
{
}

void platform_screen_render(void)
{
    SDL_Log("Rendering!\n");
    vita2d_start_drawing();
    vita2d_draw_texture(tex_buffer, 0, 0);
    const mouse *mouse = mouse_get();
    vita2d_draw_rectangle(mouse->x, mouse->y, 3, 3, RGBA8(255, 0, 0, 255));
    
    vita2d_end_drawing();
    vita2d_wait_rendering_done();
    vita2d_swap_buffers();
}
