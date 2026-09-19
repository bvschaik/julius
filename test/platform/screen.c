#include "game/settings.h"
#include "graphics/graphics.h"
#include "graphics/screen.h"
#include "input/cursor.h"
#include "platform/cursor.h"
#include "platform/screen.h"

#include "SDL.h"

#include <stdio.h>

static struct {
    int window_width;
    int window_height;
    int logical_width;
    int logical_height;
} data;

// Replace game state and drawing dependencies; exercise the real SDL window and renderer.
int setting_fullscreen(void)
{
    return 0;
}

void setting_window(int *width, int *height)
{
    *width = data.window_width;
    *height = data.window_height;
}

void setting_set_display(int fullscreen, int width, int height)
{
    data.window_width = width;
    data.window_height = height;
}

void screen_set_resolution(int width, int height)
{
    data.logical_width = width;
    data.logical_height = height;
}

int screen_width(void)
{
    return data.logical_width;
}

int screen_height(void)
{
    return data.logical_height;
}

const void *graphics_canvas(void)
{
    return NULL;
}

const cursor *input_cursor_data(cursor_shape cursor_id, cursor_scale scale)
{
    return NULL;
}

int platform_cursor_get_texture_size(int width, int height)
{
    return 0;
}

static int test_requested_scale(int percentage)
{
    data.window_width = 640;
    data.window_height = 480;
    if (!platform_screen_create("Screen scale test", percentage, 0)) {
        return 1;
    }
    int actual_scale = platform_screen_get_scale();
    int failed = actual_scale != percentage || screen_width() != 640 || screen_height() != 480;
    if (failed) {
        fprintf(stderr, "Requested %d%% for 640 x 480, got %d%% for %d x %d\n",
            percentage, actual_scale, screen_width(), screen_height());
    }
    platform_screen_destroy();
    return failed;
}

static int test_window_fits_display(int logical_width, int logical_height, int percentage, int display_id)
{
    data.window_width = logical_width;
    data.window_height = logical_height;
    if (!platform_screen_create("Screen bounds test", percentage, display_id)) {
        return 1;
    }
    SDL_DisplayMode mode;
    if (SDL_GetDesktopDisplayMode(0, &mode) != 0) {
        platform_screen_destroy();
        return 1;
    }
    int actual_scale = platform_screen_get_scale();
    int width = screen_width() * actual_scale / 100;
    int height = screen_height() * actual_scale / 100;
    int failed = screen_width() < 640 || screen_height() < 480 || width > mode.w || height > mode.h;
    if (failed) {
        fprintf(stderr, "Window %d x %d at %d%% does not fit display %d x %d or minimum 640 x 480\n",
            width, height, actual_scale, mode.w, mode.h);
    }
    platform_screen_destroy();
    return failed;
}

int main(int argc, char **argv)
{
    SDL_SetMainReady();
    SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }
    int failed = test_requested_scale(150);
    failed += test_requested_scale(50);
    failed += test_requested_scale(100);
    failed += test_requested_scale(150);
    failed += test_window_fits_display(3840, 2160, 150, 0);
    failed += test_window_fits_display(640, 480, 500, 0);
    failed += test_window_fits_display(640, 480, 100, -1);
    failed += test_window_fits_display(640, 480, 100, SDL_GetNumVideoDisplays());
    SDL_Quit();
    if (!failed) {
        printf("All 8 screen startup checks passed\n");
    }
    return failed ? 1 : 0;
}
