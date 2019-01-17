#include "platform/screen.h"
#include "platform/dpi.h"

#include "game/settings.h"
#include "graphics/graphics.h"
#include "graphics/screen.h"

#include "SDL.h"
#include <math.h>
#include <stdlib.h>

static struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} SDL;

static struct {
    int x;
    int y;
    int w;
    int h;
    int moving;
} window_pos;

static struct {
    const int WIDTH;
    const int HEIGHT;
} MINIMUM = { 640, 480 };

/* Since resize events are called from a separate thread, we need to prevent unwanted resizes. */
static int prevent_threaded_resize_event = 1;

static void platform_screen_position_window(int width, int height)
{
    int title_height = platform_dpi_get_title_height();

    /* No position information, center window */
    if ((!window_pos.x && !window_pos.y) || !window_pos.w || !window_pos.h) {
        platform_screen_center_window();
    } else if (width != window_pos.w || height != window_pos.h) {
        /* New window size, use the window center as a reference */
        window_pos.x = window_pos.x + (window_pos.w / 2) - (width / 2);
        if (!window_pos.moving) {
            /* If we got a reposition request while moving the window, it was because the window was changed to another monitor with a different DPI.
               Keep the window title bar at the same position in that case. */
            window_pos.y = window_pos.y + (window_pos.h / 2) - (height / 2);
        }
    }

    SDL_Rect monitor_coordinates;
    SDL_GetDisplayBounds(SDL_GetWindowDisplayIndex(SDL.window), &monitor_coordinates);
    int minimum_y = monitor_coordinates.y + title_height;
    int maximum_y = monitor_coordinates.h + title_height;
    int minimum_x = 0;
    int maximum_x = 0;
    int total_displays = SDL_GetNumVideoDisplays();
    for (int display_index = 0; display_index < total_displays; display_index++) {
        SDL_GetDisplayBounds(display_index, &monitor_coordinates);
        minimum_x = (monitor_coordinates.x < minimum_x) ? monitor_coordinates.x : minimum_x;
        maximum_x = (monitor_coordinates.w > maximum_x) ? monitor_coordinates.w : maximum_x;
    }

    if (window_pos.x > maximum_x || window_pos.y > maximum_y) {
        platform_screen_center_window();
    }

    window_pos.x = (window_pos.x < minimum_x) ? minimum_x : window_pos.x;
    window_pos.y = (window_pos.y < minimum_y) ? minimum_y : window_pos.y;
    window_pos.w = width;
    window_pos.h = height;

    SDL_SetWindowPosition(SDL.window, window_pos.x, window_pos.y);
}

static int platform_screen_replace_texture(int width, int height)
{
    double dpi_scale = platform_dpi_get_scale();

    /* Use linear filtering when the scale factor is not an integer. Improves overall image quality. */
    if (fabs(rint(dpi_scale) - dpi_scale) > 0.0001f) {
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    } else {
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    }

    if (SDL.texture) {
        SDL_DestroyTexture(SDL.texture);
        SDL.texture = 0;
    }

    if (SDL_RenderSetLogicalSize(SDL.renderer, (int)(width / dpi_scale), (int)(height / dpi_scale)) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to set renderer size: %s", SDL_GetError());
        return 0;
    }

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

static void platform_screen_compensate_windows_border_sizes(int width, int height)
{
    /* No need to do anything in fullscreen mode */
    if (setting_fullscreen()) {
        return;
    }

    /* In Windows, due to a bug in the way SDL calculates window borders/title bar size when DPI is changed,
       the actual window size may be different than the requested size.
       If that happens, we change the requested window size to compensate. */
    int settings_selected_width;
    int settings_selected_height;
    setting_window(&settings_selected_width, &settings_selected_height);
    double dpi_scale = platform_dpi_get_scale();
    settings_selected_width = (int)(settings_selected_width * dpi_scale);
    settings_selected_height = (int)(settings_selected_height * dpi_scale);
    int width_difference = width - settings_selected_width;
    int height_difference = height - settings_selected_height;

    /* Don't do anything if this function was called because of an outdated, sprurious resize event */
    if (platform_dpi_enabled()) {
        int title_height = platform_dpi_get_title_height();
        if (abs(height_difference) > (int) (title_height * 1.1)) {
            prevent_threaded_resize_event = 1;
            return;
        }
    }

    if (width_difference || height_difference) {
        SDL_SetWindowMinimumSize(SDL.window, ((int) (MINIMUM.WIDTH * dpi_scale)) - width_difference, ((int)(MINIMUM.HEIGHT * dpi_scale)) - height_difference);
        SDL_SetWindowSize(SDL.window, settings_selected_width - width_difference, settings_selected_height - height_difference);
    }
}

int platform_screen_create(const char *title)
{
    int width, height;
    int fullscreen = setting_fullscreen();
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
    Uint32 flags = SDL_WINDOW_RESIZABLE | platform_dpi_enabled();
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    SDL.window = SDL_CreateWindow(title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
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

    platform_dpi_init(SDL.window);

    if (!platform_screen_rescale()) {
        return 0;
    }

    /* Avoid temporary white screen on game load */
    platform_screen_render();

    return 1;
}

void platform_screen_destroy(void)
{
    platform_dpi_shutdown();

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

int platform_screen_resize(int width, int height, platform_screen_event_mode mode)
{
    switch (mode) {
        case NORMAL_EVENT:
            if (prevent_threaded_resize_event) {
                prevent_threaded_resize_event = 0;
                platform_screen_compensate_windows_border_sizes(width, height);
                return 0;
            }
            break;
        case THREADED_EVENT:
            if (prevent_threaded_resize_event) {
                return 0;
            }
            break;
        default:
            break;
    }

    double dpi_scale = platform_dpi_get_scale();

    int fullscreen = setting_fullscreen();
    setting_set_display(fullscreen, (int) (width / dpi_scale), (int)(height / dpi_scale));
    if (!fullscreen) {
        window_pos.w = width;
        window_pos.h = height;
    }
    return platform_screen_replace_texture((int)(width / dpi_scale), (int)(height / dpi_scale));
}

int platform_screen_rescale(void)
{
    prevent_threaded_resize_event = 1;

    double dpi_scale = platform_dpi_get_scale();
    SDL_Log("DPI changed. Scaling window by %f\n", dpi_scale);

    int width;
    int height;
    int min_w = (int)(MINIMUM.WIDTH * dpi_scale);
    int min_h = (int)(MINIMUM.HEIGHT * dpi_scale);

    /* Temporarily reduce minimum size. Prevents bugs on X11 when the fullscreen size is less than the minimum size we want. */
    SDL_SetWindowMinimumSize(SDL.window, 1, 1);

    if (setting_fullscreen()) {
        SDL_DisplayMode mode;
        SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(SDL.window), &mode);
        /* Do not allow fullscreen with less than the minimum size. */
        if (mode.w < min_w || mode.h < min_h)
        {
            SDL_Log("Cannot keep fullscreen. The size would be less than the minimum resolution of %i by %i. Setting windowed mode.", min_w, min_h);
            SDL_SetWindowFullscreen(SDL.window, 0);
            setting_set_display(0, MINIMUM.WIDTH, MINIMUM.HEIGHT);
        } else {
            width = (int)(mode.w / dpi_scale);
            height = (int)(mode.h / dpi_scale);
        }
    }

    /* The minimum size can be restored now. */
    SDL_SetWindowMinimumSize(SDL.window, min_w, min_h);

    if(!setting_fullscreen()) { /* No else-clause: the previous if-clause may remove fullscreen and we need to catch it. */
        setting_window(&width, &height);
        int scaled_width = (int)(width * dpi_scale);
        int scaled_height = (int)(height * dpi_scale);
        SDL_SetWindowSize(SDL.window, scaled_width, scaled_height);
        platform_screen_position_window(scaled_width, scaled_height);
    }
    return platform_screen_replace_texture(width, height);
}

void platform_screen_move(int x, int y, platform_screen_event_mode mode)
{
    window_pos.moving = mode;

    if (!setting_fullscreen() && !(SDL_GetWindowFlags(SDL.window) & SDL_WINDOW_FULLSCREEN_DESKTOP) && mode == THREADED_EVENT) {
        window_pos.x = x;
        window_pos.y = y;
    } else {
        window_pos.moving = 0;
    }
}

void platform_screen_set_fullscreen(void)
{
    prevent_threaded_resize_event = 1;
    double dpi_scale = platform_dpi_get_scale();
    SDL_GetWindowPosition(SDL.window, &window_pos.x, &window_pos.y);
    SDL_GetWindowSize(SDL.window, &window_pos.w, &window_pos.h);
    SDL_DisplayMode mode;
    SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(SDL.window), &mode);
    SDL_Log("User to fullscreen %d x %d\n", mode.w, mode.h);
    if (mode.w < (int)(MINIMUM.WIDTH * dpi_scale) || mode.h < (int)(MINIMUM.HEIGHT * dpi_scale)) {
        SDL_Log("Cannot set fullscreen. The size would be less than the minimum resolution of %i by %i.", (int)(MINIMUM.WIDTH * dpi_scale), (int)(MINIMUM.HEIGHT * dpi_scale));
        return;
    }
    if (0 != SDL_SetWindowFullscreen(SDL.window, SDL_WINDOW_FULLSCREEN_DESKTOP)) {
        SDL_Log("Unable to enter fullscreen: %s\n", SDL_GetError());
        return;
    }
    SDL_SetWindowDisplayMode(SDL.window, &mode);
    setting_set_display(1, (int)(mode.w / dpi_scale), (int)(mode.h / dpi_scale));
    platform_screen_replace_texture((int)(mode.w / dpi_scale), (int)(mode.h / dpi_scale));
}

void platform_screen_set_windowed(void)
{
    prevent_threaded_resize_event = 1;
    double dpi_scale = platform_dpi_get_scale();
    int width, height;
    setting_window(&width, &height);
    SDL_Log("User to windowed %d x %d\n", width, height);
    SDL_SetWindowFullscreen(SDL.window, 0);
    SDL_SetWindowSize(SDL.window, (int)(width * dpi_scale), (int)(height * dpi_scale));
    platform_screen_position_window((int)(width * dpi_scale), (int)(height * dpi_scale));
    setting_set_display(0, width, height);
    platform_screen_replace_texture(width, height);
}

void platform_screen_set_window_size(int width, int height)
{
    prevent_threaded_resize_event = 1;
    double dpi_scale = platform_dpi_get_scale();
    if (setting_fullscreen()) {
        SDL_SetWindowFullscreen(SDL.window, 0);
    }
    if (SDL_GetWindowFlags(SDL.window) & SDL_WINDOW_MAXIMIZED) {
        SDL_RestoreWindow(SDL.window);
    }
    SDL_SetWindowSize(SDL.window, (int)(width * dpi_scale), (int)(height * dpi_scale));
    platform_screen_position_window((int)(width * dpi_scale), (int)(height * dpi_scale));
    SDL_Log("User resize to %d x %d\n", width, height);
    setting_set_display(0, width, height);
    platform_screen_replace_texture(width, height);
}

void platform_screen_center_window(void)
{
    int display = SDL_GetWindowDisplayIndex(SDL.window);
    SDL_SetWindowPosition(SDL.window, SDL_WINDOWPOS_CENTERED_DISPLAY(display), SDL_WINDOWPOS_CENTERED_DISPLAY(display));
    SDL_GetWindowPosition(SDL.window, &window_pos.x, &window_pos.y);
}

void platform_screen_render(void)
{
    platform_dpi_check_change();
    SDL_UpdateTexture(SDL.texture, NULL, graphics_canvas(), screen_width() * 4);
    SDL_RenderCopy(SDL.renderer, SDL.texture, NULL, NULL);
    SDL_RenderPresent(SDL.renderer);
}
