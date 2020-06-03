#include "platform/screen.h"

#include "core/calc.h"
#include "game/settings.h"
#include "game/system.h"
#include "graphics/graphics.h"
#include "graphics/screen.h"

#include "SDL.h"

static struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} SDL;

static struct {
    int x;
    int y;
    int centered;
} window_pos = { 0, 0, 1 };

static struct {
    const int WIDTH;
    const int HEIGHT;
} MINIMUM = { 640, 480 };

static int scale_percentage = 100;

static int scale_logical_to_pixels(int logical_value)
{
    return logical_value * scale_percentage / 100;
}

static int scale_pixels_to_logical(int pixel_value)
{
    return pixel_value * 100 / scale_percentage;
}

int platform_screen_create(const char *title, int display_scale_percentage)
{
    scale_percentage = display_scale_percentage;

    int width, height;
    int fullscreen = system_is_fullscreen_only() ? 1 : setting_fullscreen();
    if (fullscreen) {
        SDL_DisplayMode mode;
        SDL_GetDesktopDisplayMode(0, &mode);
        width = mode.w;
        height = mode.h;
    } else {
        setting_window(&width, &height);
        width = scale_logical_to_pixels(width);
        height = scale_logical_to_pixels(height);
    }

    platform_screen_destroy();

    SDL_Log("Creating screen %d x %d, fullscreen? %d\n", width, height, fullscreen);
    Uint32 flags = SDL_WINDOW_RESIZABLE;
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    SDL.window = SDL_CreateWindow(title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, flags);

    if (!SDL.window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create window: %s", SDL_GetError());
        return 0;
    }

    if (system_is_fullscreen_only()) {
        SDL_GetWindowSize(SDL.window, &width, &height);
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
#if !defined(__APPLE__)
    if (fullscreen && SDL_GetNumVideoDisplays() > 1) {
        SDL_SetWindowGrab(SDL.window, SDL_TRUE);
    }
#endif
    SDL_SetWindowMinimumSize(SDL.window, scale_logical_to_pixels(MINIMUM.WIDTH), scale_logical_to_pixels(MINIMUM.HEIGHT));
    return platform_screen_resize(width, height);
}

void platform_screen_destroy(void)
{
    if (SDL.texture) {
        SDL_DestroyTexture(SDL.texture);
        SDL.texture = 0;
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

int platform_screen_resize(int pixel_width, int pixel_height)
{
    int logical_width = scale_pixels_to_logical(pixel_width);
    int logical_height = scale_pixels_to_logical(pixel_height);

    if (SDL.texture) {
        SDL_DestroyTexture(SDL.texture);
        SDL.texture = 0;
    }

    setting_set_display(setting_fullscreen(), logical_width, logical_height);
    SDL.texture = SDL_CreateTexture(SDL.renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        logical_width, logical_height);

    // Scale using nearest neighbour when we scale a multiple of 100%: makes it look sharper
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, (scale_percentage % 100 == 0) ? "nearest" : "linear");
    SDL_RenderSetLogicalSize(SDL.renderer, logical_width, logical_height);

    if (SDL.texture) {
        SDL_Log("Texture created (%d x %d)", logical_width, logical_height);
        screen_set_resolution(logical_width, logical_height);
        return 1;
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create texture: %s", SDL_GetError());
        return 0;
    }
}

void platform_screen_move(int x, int y)
{
    if (!setting_fullscreen()) {
        window_pos.x = x;
        window_pos.y = y;
        window_pos.centered = 0;
    }
}

void platform_screen_set_fullscreen(void)
{
    SDL_GetWindowPosition(SDL.window, &window_pos.x, &window_pos.y);
    SDL_DisplayMode mode;
    SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(SDL.window), &mode);
    SDL_Log("User to fullscreen %d x %d\n", mode.w, mode.h);
    if (0 != SDL_SetWindowFullscreen(SDL.window, SDL_WINDOW_FULLSCREEN_DESKTOP)) {
        SDL_Log("Unable to enter fullscreen: %s\n", SDL_GetError());
        return;
    }
    SDL_SetWindowDisplayMode(SDL.window, &mode);

#if !defined(__APPLE__)
    if (SDL_GetNumVideoDisplays() > 1) {
        SDL_SetWindowGrab(SDL.window, SDL_TRUE);
    }
#endif
    setting_set_display(1, mode.w, mode.h);
}

void platform_screen_set_windowed(void)
{
    if (system_is_fullscreen_only()) {
        return;
    }
    int logical_width, logical_height;
    setting_window(&logical_width, &logical_height);
    int pixel_width = scale_logical_to_pixels(logical_width);
    int pixel_height = scale_logical_to_pixels(logical_height);
    SDL_Log("User to windowed %d x %d", pixel_width, pixel_height);
    SDL_SetWindowFullscreen(SDL.window, 0);
    SDL_SetWindowSize(SDL.window, pixel_width, pixel_height);
    if (window_pos.centered) {
        platform_screen_center_window();
    }
    if (SDL_GetWindowGrab(SDL.window) == SDL_TRUE) {
        SDL_SetWindowGrab(SDL.window, SDL_FALSE);
    }
    setting_set_display(0, pixel_width, pixel_height);
}

void platform_screen_set_window_size(int logical_width, int logical_height)
{
    if (system_is_fullscreen_only()) {
        return;
    }
    int pixel_width = scale_logical_to_pixels(logical_width);
    int pixel_height = scale_logical_to_pixels(logical_height);
    if (setting_fullscreen()) {
        SDL_SetWindowFullscreen(SDL.window, 0);
    } else {
        SDL_GetWindowPosition(SDL.window, &window_pos.x, &window_pos.y);
    }
    if (SDL_GetWindowFlags(SDL.window) & SDL_WINDOW_MAXIMIZED) {
        SDL_RestoreWindow(SDL.window);
    }
    SDL_SetWindowSize(SDL.window, pixel_width, pixel_height);
    if (window_pos.centered) {
        platform_screen_center_window();
    }
    SDL_Log("User resize to %d x %d\n", pixel_width, pixel_height);
    if (SDL_GetWindowGrab(SDL.window) == SDL_TRUE) {
        SDL_SetWindowGrab(SDL.window, SDL_FALSE);
    }
    setting_set_display(0, pixel_width, pixel_height);
}

void platform_screen_center_window(void)
{
    int display = SDL_GetWindowDisplayIndex(SDL.window);
    SDL_SetWindowPosition(SDL.window, SDL_WINDOWPOS_CENTERED_DISPLAY(display), SDL_WINDOWPOS_CENTERED_DISPLAY(display));
    window_pos.centered = 1;
}

void platform_screen_render(void)
{
    SDL_RenderClear(SDL.renderer);
    SDL_UpdateTexture(SDL.texture, NULL, graphics_canvas(), screen_width() * 4);
    SDL_RenderCopy(SDL.renderer, SDL.texture, NULL, NULL);
    SDL_RenderPresent(SDL.renderer);
}

void system_set_mouse_position(int *x, int *y)
{
    *x = calc_bound(*x, 0, scale_logical_to_pixels(screen_width()) - 1);
    *y = calc_bound(*y, 0, scale_logical_to_pixels(screen_height()) - 1);
    SDL_WarpMouseInWindow(SDL.window, *x, *y);
}

int system_is_fullscreen_only(void)
{
    return 0;
}
