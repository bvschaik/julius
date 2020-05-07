#include "platform/screen.h"

#include "SDL.h"

#include "city/view.h"
#include "core/config.h"
#include "game/settings.h"
#include "game/system.h"
#include "graphics/graphics.h"
#include "graphics/menu.h"
#include "graphics/screen.h"
#include "input/mouse.h"

#include "switch.h"

struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture_ui;
    SDL_Texture *texture_city;
} SDL;

static struct {
    SDL_Rect offset;
    SDL_Rect renderer;
} city_texture_position;

static struct {
    int x;
    int y;
} window_pos;

int platform_screen_create(const char *title, int display_scale_percentage)
{
    int width, height;
    int fullscreen = 1;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

    if (fullscreen) {
        SDL_DisplayMode mode;
        SDL_GetDesktopDisplayMode(0, &mode);
        width = mode.w;
        height = mode.h;
    } else {
        setting_window(&width, &height);
    }

    platform_screen_destroy();

    SDL_Log("Creating screen %d x %d, fullscreen? %d\n", width, height, fullscreen);
    Uint32 flags = SDL_WINDOW_RESIZABLE;
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    SDL.window = SDL_CreateWindow(title,
        0, 0,
        width, height, flags);
    if (!SDL.window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create window: %s", SDL_GetError());
        return 0;
    }

    SDL.renderer = SDL_CreateRenderer(SDL.window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (!SDL.renderer) {
        SDL_Log("Unable to create renderer, trying software renderer: %s", SDL_GetError());
        SDL.renderer = SDL_CreateRenderer(SDL.window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_SOFTWARE);
        if (!SDL.renderer) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create renderer: %s", SDL_GetError());
            return 0;
        }
    }

    SDL_SetRenderDrawColor(SDL.renderer, 0, 0, 0, 0xff);
    return platform_screen_resize(SWITCH_DISPLAY_WIDTH, SWITCH_DISPLAY_HEIGHT);
}

static int create_textures(int width, int height)
{
    if (SDL.texture_ui) {
        SDL_DestroyTexture(SDL.texture_ui);
        SDL.texture_ui = 0;
    }
    if (SDL.texture_city) {
        SDL_DestroyTexture(SDL.texture_city);
        SDL.texture_city = 0;
    }

    SDL.texture_ui = SDL_CreateTexture(SDL.renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        width, height);

    int city_texture_error;

    if (config_get(CONFIG_UI_ZOOM)) {
        SDL.texture_city = SDL_CreateTexture(SDL.renderer,
            SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
            width * 2, height * 2);
        city_texture_position.renderer.x = 0;
        city_texture_position.renderer.y = TOP_MENU_HEIGHT;
        city_texture_position.renderer.h = height * 2 - TOP_MENU_HEIGHT;
        SDL_SetTextureBlendMode(SDL.texture_ui, SDL_BLENDMODE_BLEND);
        city_texture_error = SDL.texture_city == 0;
    } else {
        city_texture_error = 0;
        SDL_SetTextureBlendMode(SDL.texture_ui, SDL_BLENDMODE_NONE);
    }

    if (SDL.texture_ui && !city_texture_error) {
        SDL_Log("Textures created (%d x %d)", width, height);
        return 1;
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create textures: %s", SDL_GetError());
        return 0;
    }
}

void platform_screen_destroy(void)
{
    if (SDL.texture_ui) {
        SDL_DestroyTexture(SDL.texture_ui);
        SDL.texture_ui = 0;
    }
    if (SDL.texture_city) {
        SDL_DestroyTexture(SDL.texture_city);
        SDL.texture_city = 0;
    }
    if (SDL.renderer) {
        SDL_DestroyRenderer(SDL.renderer);
        SDL.renderer = 0;
    }
    if (SDL.window) {
        SDL_DestroyWindow(SDL.window);
        SDL.window = 0;
    }
}

int platform_screen_resize(int width, int height)
{
    setting_set_display(setting_fullscreen(), width, height);
    if (create_textures(width, height)) {
        screen_set_resolution(width, height);
        return 1;
    } else {
        return 0;
    }
}

void platform_screen_set_fullscreen(void)
{
    SDL_GetWindowPosition(SDL.window, &window_pos.x, &window_pos.y);
    int orig_w, orig_h;
    SDL_GetWindowSize(SDL.window, &orig_w, &orig_h);
    SDL_DisplayMode mode;
    SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(SDL.window), &mode);
    SDL_Log("User to fullscreen %d x %d\n", mode.w, mode.h);
    if (0 != SDL_SetWindowFullscreen(SDL.window, SDL_WINDOW_FULLSCREEN_DESKTOP)) {
        SDL_Log("Unable to enter fullscreen: %s\n", SDL_GetError());
        return;
    }
    SDL_SetWindowDisplayMode(SDL.window, &mode);
    setting_set_display(1, mode.w, mode.h);
}

void platform_screen_set_windowed(void)
{
    int width, height;
    setting_window(&width, &height);
    SDL_Log("User to windowed %d x %d\n", width, height);
    SDL_SetWindowFullscreen(SDL.window, 0);
    SDL_SetWindowSize(SDL.window, width, height);
    SDL_SetWindowPosition(SDL.window, window_pos.x, window_pos.y);
    setting_set_display(0, width, height);
}

void platform_screen_set_window_size(int width, int height)
{
    if (setting_fullscreen()) {
        SDL_SetWindowFullscreen(SDL.window, 0);
    }
    SDL_SetWindowSize(SDL.window, width, height);
    SDL_SetWindowPosition(SDL.window, window_pos.x, window_pos.y);
    SDL_Log("User resize to %d x %d\n", width, height);
    setting_set_display(0, width, height);
}

void platform_screen_center_window(void)
{
    SDL_SetWindowPosition(SDL.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void platform_screen_render(void)
{
    if (config_get(CONFIG_UI_ZOOM)) {
        SDL_RenderClear(SDL.renderer);
        city_view_get_unscaled_viewport(&city_texture_position.offset.x, &city_texture_position.offset.y,
            &city_texture_position.renderer.w, &city_texture_position.offset.h);
        city_view_get_scaled_viewport(&city_texture_position.offset.x, &city_texture_position.offset.y,
            &city_texture_position.offset.w, &city_texture_position.offset.h);
        city_texture_position.renderer.w = city_texture_position.renderer.w * 2 + 1;
        SDL_UpdateTexture(SDL.texture_city, &city_texture_position.offset, graphics_canvas(CANVAS_CITY), screen_width() * 4 * 2);
        SDL_RenderCopy(SDL.renderer, SDL.texture_city, &city_texture_position.offset, &city_texture_position.renderer);
    }
    SDL_UpdateTexture(SDL.texture_ui, NULL, graphics_canvas(CANVAS_UI), screen_width() * 4);
    SDL_RenderCopy(SDL.renderer, SDL.texture_ui, NULL, NULL);

    const mouse *mouse = mouse_get();
    SDL_Rect dst;
    dst.x = ((mouse->x - current_cursor->hotspot_x) * SWITCH_PIXEL_WIDTH) / SWITCH_DISPLAY_WIDTH;
    dst.y = ((mouse->y - current_cursor->hotspot_y) * SWITCH_PIXEL_HEIGHT) / SWITCH_DISPLAY_HEIGHT;
    dst.w = (32 * SWITCH_PIXEL_WIDTH) / SWITCH_DISPLAY_WIDTH;
    dst.h = (32 * SWITCH_PIXEL_HEIGHT) / SWITCH_DISPLAY_HEIGHT;
    SDL_RenderCopy(SDL.renderer, current_cursor->texture, NULL, &dst);

    SDL_RenderPresent(SDL.renderer);
}

void platform_screen_warp_mouse(int x, int y)
{
    SDL_WarpMouseInWindow(SDL.window, x, y);
}

int system_is_fullscreen_only(void)
{
    return 1;
}

void system_reload_textures(void)
{
    int width = screen_width();
    int height = screen_height();
    create_textures(width, height);
    screen_set_resolution(width, height);
}

int system_save_screen_buffer(void *pixels)
{
    return SDL_RenderReadPixels(SDL.renderer, NULL, SDL_PIXELFORMAT_ARGB8888, pixels, screen_width() * sizeof(color_t)) == 0;
}
