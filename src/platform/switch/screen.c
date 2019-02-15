#include "platform/screen.h"

#include "SDL.h"

#include "game/settings.h"
#include "graphics/graphics.h"
#include "graphics/screen.h"

#include "switch.h"

extern SDL_Texture *current_cursor;
#include "input/mouse.h"

struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} SDL = {0, 0, 0};

static struct {
    int x;
    int y;
} window_pos;

int platform_screen_create(const char *title)
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

    return platform_screen_resize(SWITCH_DISPLAY_WIDTH, SWITCH_DISPLAY_HEIGHT);
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

int platform_screen_resize(int width, int height)
{
    if (SDL.texture) {
        SDL_DestroyTexture(SDL.texture);
        SDL.texture = 0;
    }

    setting_set_display(setting_fullscreen(), width, height);
    SDL.texture = SDL_CreateTexture(SDL.renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        width, height);
    if (SDL.texture) {
        SDL_Log("Texture created (%d x %d)", width, height);
        screen_set_resolution(width, height);
        return 1;
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create texture: %s", SDL_GetError());
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
    SDL_UpdateTexture(SDL.texture, NULL, graphics_canvas(), screen_width() * 4);
    SDL_RenderCopy(SDL.renderer, SDL.texture, NULL, NULL);

    const mouse *mouse = mouse_get();
    SDL_Rect dst;
    dst.x = (mouse->x * SWITCH_PIXEL_WIDTH) / SWITCH_DISPLAY_WIDTH;
    dst.y = (mouse->y * SWITCH_PIXEL_HEIGHT) / SWITCH_DISPLAY_HEIGHT;
    dst.w = (32 * SWITCH_PIXEL_WIDTH) / SWITCH_DISPLAY_WIDTH;
    dst.h = (32 * SWITCH_PIXEL_HEIGHT) / SWITCH_DISPLAY_HEIGHT;
    SDL_RenderCopy(SDL.renderer, current_cursor, NULL, &dst);

    SDL_RenderPresent(SDL.renderer);
}
