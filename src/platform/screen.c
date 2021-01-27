#include "platform/screen.h"

#include "city/view.h"
#include "core/calc.h"
#include "core/config.h"
#include "game/settings.h"
#include "game/system.h"
#include "graphics/graphics.h"
#include "graphics/menu.h"
#include "graphics/screen.h"
#include "input/cursor.h"
#include "platform/android/android.h"
#include "platform/cursor.h"
#include "platform/switch/switch.h"
#include "platform/vita/vita.h"

#include "SDL.h"

#include <stdlib.h>

static struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture_ui;
    SDL_Texture *texture_city;
    SDL_Texture *cursors[CURSOR_MAX];
} SDL;

static struct {
    SDL_Rect offset;
    SDL_Rect renderer;
} city_texture_position;

static struct {
    int x;
    int y;
    int centered;
} window_pos = {0, 0, 1};

static struct {
    const int WIDTH;
    const int HEIGHT;
} MINIMUM = {640, 480};

static int scale_percentage = 100;
static color_t *framebuffer_ui;
static color_t *framebuffer_city;

static int scale_logical_to_pixels(int logical_value)
{
    return logical_value * scale_percentage / 100;
}

static int scale_pixels_to_logical(int pixel_value)
{
    return pixel_value * 100 / scale_percentage;
}

static int get_max_scale_percentage(int pixel_width, int pixel_height)
{
    int width_scale_pct = pixel_width * 100 / MINIMUM.WIDTH;
    int height_scale_pct = pixel_height * 100 / MINIMUM.HEIGHT;
    return SDL_min(width_scale_pct, height_scale_pct);
}

static void set_scale_percentage(int new_scale, int pixel_width, int pixel_height)
{
#ifdef __vita__
    scale_percentage = 100;
#else
    scale_percentage = calc_bound(new_scale, 50, 500);
#endif

    if (!pixel_width || !pixel_height) {
        return;
    }

    int max_scale_pct = get_max_scale_percentage(pixel_width, pixel_height);
    if (max_scale_pct < scale_percentage) {
        scale_percentage = max_scale_pct;
        SDL_Log("Maximum scale of %i applied", scale_percentage);
    }

    SDL_SetWindowMinimumSize(SDL.window, scale_logical_to_pixels(MINIMUM.WIDTH), scale_logical_to_pixels(MINIMUM.HEIGHT));

    // Scale using nearest neighbour when we scale a multiple of 100%: makes it look sharper
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, (scale_percentage % 100 == 0) ? "nearest" : "linear");
}

#ifdef __ANDROID__
static void set_scale_for_screen(int pixel_width, int pixel_height)
{
    set_scale_percentage(android_get_screen_density() * 100, pixel_width, pixel_height);
    config_set(CONFIG_SCREEN_CURSOR_SCALE, scale_percentage);
    if (SDL.texture) {
        system_init_cursors(scale_percentage);
    }
    SDL_Log("Auto-setting scale to %i", scale_percentage);
}
#endif

int platform_screen_get_scale(void)
{
    return scale_percentage;
}

void platform_screen_get_scaled_params(int* width, int* height)
{
    int fullscreen = setting_fullscreen();
    if (fullscreen == 1) {
        SDL_DisplayMode mode;
        SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(SDL.window), &mode);
        *width = mode.w;
        *height = mode.h;
    }
    else {
        setting_window(width, height);
    }
}

int platform_screen_create(const char *title, int display_scale_percentage)
{
    set_scale_percentage(display_scale_percentage, 0, 0);

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

#ifdef __ANDROID__
    // Fix for wrong colors on some android devices
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
#endif

    SDL_Log("Creating screen %d x %d, %s, driver: %s", width, height,
        fullscreen ? "fullscreen" : "windowed", SDL_GetCurrentVideoDriver());
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

    SDL_Log("Creating renderer");
    SDL.renderer = SDL_CreateRenderer(SDL.window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (!SDL.renderer) {
        SDL_Log("Unable to create renderer, trying software renderer: %s", SDL_GetError());
        SDL.renderer = SDL_CreateRenderer(SDL.window, -1, SDL_RENDERER_SOFTWARE);
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
    SDL_SetRenderDrawColor(SDL.renderer, 0, 0, 0, 0xff);
    set_scale_percentage(display_scale_percentage, width, height);
    return platform_screen_resize(width, height, 1);
}

static void destroy_screen_textures(void)
{
    if (SDL.texture_ui) {
        SDL_DestroyTexture(SDL.texture_ui);
        SDL.texture_ui = 0;
    }
    if (SDL.texture_city) {
        SDL_DestroyTexture(SDL.texture_city);
        SDL.texture_city = 0;
    }
}

static int create_textures(int width, int height)
{
    destroy_screen_textures();

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
        city_texture_position.renderer.h = height - TOP_MENU_HEIGHT;
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
    destroy_screen_textures();
    if (SDL.renderer) {
        SDL_DestroyRenderer(SDL.renderer);
        SDL.renderer = 0;
    }
    if (SDL.window) {
        SDL_DestroyWindow(SDL.window);
        SDL.window = 0;
    }
}

int platform_screen_resize(int pixel_width, int pixel_height, int save)
{
#ifdef __ANDROID__
    set_scale_for_screen(pixel_width, pixel_height);
#endif

    int logical_width = scale_pixels_to_logical(pixel_width);
    int logical_height = scale_pixels_to_logical(pixel_height);

    if (save) {
        setting_set_display(setting_fullscreen(), logical_width, logical_height);
    }

    SDL_RenderSetLogicalSize(SDL.renderer, logical_width, logical_height);

    if (create_textures(logical_width, logical_height)) {
        screen_set_resolution(logical_width, logical_height);
        return 1;
    }
    else {
        return 0;
    }
}

int system_scale_display(int display_scale_percentage)
{
    int width, height;
    SDL_GetWindowSize(SDL.window, &width, &height);
    set_scale_percentage(display_scale_percentage, width, height);
    platform_screen_resize(width, height, 1);
    return scale_percentage;
}

int system_get_max_display_scale(void)
{
    int width, height;
    SDL_GetWindowSize(SDL.window, &width, &height);
    return get_max_scale_percentage(width, height);
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
    int display = SDL_GetWindowDisplayIndex(SDL.window);
    SDL_DisplayMode mode;
    SDL_GetDesktopDisplayMode(display, &mode);
    SDL_Log("User to fullscreen %d x %d on display %d", mode.w, mode.h, display);
    if (0 != SDL_SetWindowFullscreen(SDL.window, SDL_WINDOW_FULLSCREEN_DESKTOP)) {
        SDL_Log("Unable to enter fullscreen: %s", SDL_GetError());
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
    int display = SDL_GetWindowDisplayIndex(SDL.window);
    SDL_Log("User to windowed %d x %d on display %d", pixel_width, pixel_height, display);
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
    int display = SDL_GetWindowDisplayIndex(SDL.window);
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
    SDL_Log("User resize to %d x %d on display %d", pixel_width, pixel_height, display);
    if (SDL_GetWindowGrab(SDL.window) == SDL_TRUE) {
        SDL_SetWindowGrab(SDL.window, SDL_FALSE);
    }
    setting_set_display(0, pixel_width, pixel_height);
}

void platform_screen_center_window(void)
{
    int display = SDL_GetWindowDisplayIndex(SDL.window);
    SDL_SetWindowPosition(SDL.window,
        SDL_WINDOWPOS_CENTERED_DISPLAY(display), SDL_WINDOWPOS_CENTERED_DISPLAY(display));
    window_pos.centered = 1;
}

#ifdef PLATFORM_USE_SOFTWARE_CURSOR
static void draw_software_mouse_cursor(void)
{
    const mouse *mouse = mouse_get();
    if (!mouse->is_touch) {
        cursor_shape current_cursor_shape = platform_cursor_get_current_shape();
        const cursor *c = input_cursor_data(current_cursor_shape, platform_cursor_get_current_scale());
        if (c) {
            int size = platform_cursor_get_texture_size(c->width, c->height);
            size = calc_adjust_with_percentage(size, calc_percentage(100, scale_percentage));
            SDL_Rect dst;
            dst.x = mouse->x - c->hotspot_x;
            dst.y = mouse->y - c->hotspot_y;
            dst.w = size;
            dst.h = size;
            SDL_RenderCopy(SDL.renderer, SDL.cursors[current_cursor_shape], NULL, &dst);
        }
    }
}
#endif

#ifdef _WIN32
void platform_screen_recreate_texture(void)
{
    // On Windows, if ctrl + alt + del is pressed during fullscreen, the rendering context may be lost for a few frames
    // after restoring the window, preventing the texture from being recreated. This forces an attempt to recreate the
    // texture every frame to bypass that issue.
    if (!SDL.texture_ui && SDL.renderer && setting_fullscreen()) {
        SDL_DisplayMode mode;
        SDL_GetWindowDisplayMode(SDL.window, &mode);
        screen_set_resolution(scale_pixels_to_logical(mode.w), scale_pixels_to_logical(mode.h));
        create_textures(screen_width(), screen_height());
    }
}
#endif

void platform_screen_render(void)
{
    SDL_RenderClear(SDL.renderer);
    if (config_get(CONFIG_UI_ZOOM)) {
        city_view_get_unscaled_viewport(&city_texture_position.offset.x, &city_texture_position.offset.y,
            &city_texture_position.renderer.w, &city_texture_position.offset.h);
        city_view_get_scaled_viewport(&city_texture_position.offset.x, &city_texture_position.offset.y,
            &city_texture_position.offset.w, &city_texture_position.offset.h);
        SDL_UpdateTexture(SDL.texture_city, &city_texture_position.offset, graphics_canvas(CANVAS_CITY), screen_width() * 4 * 2);
        SDL_RenderCopy(SDL.renderer, SDL.texture_city, &city_texture_position.offset, &city_texture_position.renderer);
    }
    SDL_UpdateTexture(SDL.texture_ui, NULL, graphics_canvas(CANVAS_UI), screen_width() * 4);
    SDL_RenderCopy(SDL.renderer, SDL.texture_ui, NULL, NULL);
#ifdef PLATFORM_USE_SOFTWARE_CURSOR
    draw_software_mouse_cursor();
#endif
    SDL_RenderPresent(SDL.renderer);
}

void platform_screen_generate_mouse_cursor_texture(int cursor_id, int scale, const color_t *cursor_colors)
{
    if (SDL.cursors[cursor_id]) {
        SDL_DestroyTexture(SDL.cursors[cursor_id]);
        SDL.cursors[cursor_id] = 0;
    }
    const cursor *c = input_cursor_data(cursor_id, scale);
    int size = platform_cursor_get_texture_size(c->width, c->height);
    SDL.cursors[cursor_id] = SDL_CreateTexture(SDL.renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC,
        size, size);
    if (!SDL.cursors[cursor_id]) {
        return;
    }
    SDL_UpdateTexture(SDL.cursors[cursor_id], NULL, cursor_colors, size * sizeof(color_t));
    SDL_SetTextureBlendMode(SDL.cursors[cursor_id], SDL_BLENDMODE_BLEND);
}

void system_set_mouse_position(int *x, int *y)
{
    *x = calc_bound(*x, 0, screen_width() - 1);
    *y = calc_bound(*y, 0, screen_height() - 1);
    SDL_WarpMouseInWindow(SDL.window, scale_logical_to_pixels(*x), scale_logical_to_pixels(*y));
}

int system_is_fullscreen_only(void)
{
#if defined(__ANDROID__) || defined(__SWITCH__) || defined(__vita__)
    return 1;
#else
    return 0;
#endif
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

color_t *system_create_ui_framebuffer(int width, int height)
{
    free(framebuffer_ui);
    framebuffer_ui = (color_t *)malloc((size_t)width * height * sizeof(color_t));
    return framebuffer_ui;
}

color_t *system_create_city_framebuffer(int width, int height)
{
    free(framebuffer_city);
    framebuffer_city = (color_t *)malloc((size_t)width * height * sizeof(color_t) * 4);
    return framebuffer_city;
}

void system_release_city_framebuffer(void)
{
    free(framebuffer_city);
    framebuffer_city = 0;
}
