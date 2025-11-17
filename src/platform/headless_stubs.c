// Stub implementations for headless mode
// These functions are required by the game but do nothing in headless mode

#include "game/system.h"
#include "graphics/color.h"
#include "input/cursor.h"
#include "input/keys.h"

#include <stdlib.h>
#include <string.h>

// Platform screen functions
int platform_screen_create(const char *title, int display_scale_percentage, int display_id)
{
    // No-op: headless mode doesn't create a screen
    return 1;
}

void platform_screen_destroy(void)
{
    // No-op
}

int platform_screen_resize(int pixel_width, int pixel_height)
{
    // No-op
    return 1;
}

void platform_screen_move(int x, int y)
{
    // No-op
}

int platform_screen_get_scale(void)
{
    return 100; // Default 100% scale
}

void platform_screen_set_fullscreen(void)
{
    // No-op
}

void platform_screen_set_windowed(void)
{
    // No-op
}

void platform_screen_set_window_size(int logical_width, int logical_height)
{
    // No-op
}

void platform_screen_center_window(void)
{
    // No-op
}

void platform_screen_clear(void)
{
    // No-op
}

void platform_screen_update(void)
{
    // No-op
}

void platform_screen_render(void)
{
    // No-op
}

void platform_screen_generate_mouse_cursor_texture(int cursor_id, cursor_scale scale, const color_t *cursor_colors)
{
    // No-op
}

// System functions
void system_resize(int width, int height)
{
    // No-op
}

void system_center(void)
{
    // No-op
}

int system_is_fullscreen_only(void)
{
    return 0; // Not fullscreen only
}

void system_set_fullscreen(int fullscreen)
{
    // No-op
}

int system_scale_display(int scale_percentage)
{
    return 100; // Always return 100%
}

int system_can_scale_display(int *min_scale, int *max_scale)
{
    *min_scale = 100;
    *max_scale = 100;
    return 0; // Cannot scale
}

void system_init_cursors(int scale_percentage)
{
    // No-op
}

void system_set_cursor(int cursor_id)
{
    // No-op
}

key_type system_keyboard_key_for_symbol(const char *name)
{
    return KEY_TYPE_NONE;
}

const char *system_keyboard_key_name(key_type key)
{
    return "";
}

const char *system_keyboard_key_modifier_name(key_modifier_type modifier)
{
    return "";
}

void system_keyboard_set_input_rect(int x, int y, int width, int height)
{
    // No-op
}

void system_keyboard_show(void)
{
    // No-op
}

void system_keyboard_hide(void)
{
    // No-op
}

void system_start_text_input(void)
{
    // No-op
}

void system_stop_text_input(void)
{
    // No-op
}

void system_mouse_set_relative_mode(int enabled)
{
    // No-op
}

void system_mouse_get_relative_state(int *x, int *y)
{
    *x = 0;
    *y = 0;
}

void system_move_mouse_cursor(int delta_x, int delta_y)
{
    // No-op
}

void system_set_mouse_position(int *x, int *y)
{
    // Keep position unchanged or clamp to 0
    if (*x < 0) *x = 0;
    if (*y < 0) *y = 0;
}

color_t *system_create_framebuffer(int width, int height)
{
    // Allocate a dummy framebuffer
    return (color_t *)malloc(width * height * sizeof(color_t));
}

// Platform utility functions
int platform_sdl_version_at_least(int major, int minor, int patch)
{
    return 0; // No SDL in headless mode
}

// Sound device functions (all no-ops in headless mode)
void sound_device_open(void)
{
    // No-op
}

void sound_device_close(void)
{
    // No-op
}

void sound_device_init_channels(int num_channels, char filenames[][32])
{
    // No-op
}

int sound_device_is_channel_playing(int channel)
{
    return 0; // No channels playing
}

void sound_device_set_music_volume(int volume_pct)
{
    // No-op
}

void sound_device_set_channel_volume(int channel, int volume_pct)
{
    // No-op
}

int sound_device_play_music(const char *filename, int volume_pct)
{
    return 1; // Success (fake)
}

void sound_device_play_file_on_channel(const char *filename, int channel, int volume_pct)
{
    // No-op
}

void sound_device_play_channel(int channel, int volume_pct)
{
    // No-op
}

void sound_device_play_channel_panned(int channel, int volume_pct, int left_pct, int right_pct)
{
    // No-op
}

void sound_device_stop_music(void)
{
    // No-op
}

void sound_device_stop_channel(int channel)
{
    // No-op
}

void sound_device_use_custom_music_player(int bitdepth, int num_channels, int rate,
                                          const unsigned char *data, int len)
{
    // No-op
}

void sound_device_write_custom_music_data(const unsigned char *data, int len)
{
    // No-op
}

void sound_device_use_default_music_player(void)
{
    // No-op
}
