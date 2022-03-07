#include "graphics.h"

#include "graphics/renderer.h"
#include "graphics/screen.h"

static void set_translation(int x, int y)
{
    if (x != 0 || y != 0) {
        graphics_renderer()->set_viewport(x, y, screen_width() - x, screen_height() - y);
    } else {
        graphics_renderer()->reset_viewport();
    }
}

void graphics_in_dialog(void)
{
    set_translation(screen_dialog_offset_x(), screen_dialog_offset_y());
}

void graphics_in_dialog_with_size(int width, int height)
{
    set_translation((screen_width() - width) / 2, (screen_height() - height) / 2);
}

void graphics_reset_dialog(void)
{
    graphics_renderer()->reset_viewport();
}

void graphics_set_clip_rectangle(int x, int y, int width, int height)
{
    graphics_renderer()->set_clip_rectangle(x, y, width, height);
}

void graphics_reset_clip_rectangle(void)
{
    graphics_renderer()->reset_clip_rectangle();
}

void graphics_clear_screen(void)
{
    graphics_renderer()->clear_screen();
}

void graphics_draw_line(int x_start, int x_end, int y_start, int y_end, color_t color)
{
    graphics_renderer()->draw_line(x_start, x_end, y_start, y_end, color);
}

void graphics_draw_rect(int x, int y, int width, int height, color_t color)
{
    graphics_renderer()->draw_rect(x, width, y, height, color);
}

void graphics_draw_inset_rect(int x, int y, int width, int height)
{
    int x_end = x + width - 1;
    int y_end = y + height - 1;
    graphics_renderer()->draw_line(x, x_end, y, y, COLOR_INSET_DARK);
    graphics_renderer()->draw_line(x_end, x_end, y, y_end, COLOR_INSET_LIGHT);
    graphics_renderer()->draw_line(x, x_end, y_end, y_end, COLOR_INSET_LIGHT);
    graphics_renderer()->draw_line(x, x, y, y_end, COLOR_INSET_DARK);
}

void graphics_fill_rect(int x, int y, int width, int height, color_t color)
{
    graphics_renderer()->fill_rect(x, width, y, height, color);
}

void graphics_shade_rect(int x, int y, int width, int height, int darkness)
{
    color_t alpha = (0x11 * darkness) << COLOR_BITSHIFT_ALPHA;
    graphics_renderer()->fill_rect(x, width, y, height, alpha);
}

int graphics_save_to_image(int image_id, int x, int y, int width, int height)
{
    return graphics_renderer()->save_image_from_screen(image_id, x, y, width, height);
}

void graphics_draw_from_image(int image_id, int x, int y)
{
    graphics_renderer()->draw_image_to_screen(image_id, x, y);
}
