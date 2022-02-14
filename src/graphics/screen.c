#include "screen.h"

#include "city/view.h"
#include "city/warning.h"
#include "graphics/renderer.h"
#include "graphics/window.h"

static struct {
    int width;
    int height;
    struct {
        int x;
        int y;
    } dialog_offset;
} data;

void screen_set_resolution(int width, int height)
{
    data.width = width;
    data.height = height;
    data.dialog_offset.x = (width - 640) / 2;
    data.dialog_offset.y = (height - 480) / 2;

    graphics_renderer()->clear_screen();
    graphics_renderer()->set_clip_rectangle(0, 0, width, height);

    city_view_set_viewport(width, height);
    city_warning_clear_all();
    window_invalidate();
}

int screen_width(void)
{
    return data.width;
}

int screen_height(void)
{
    return data.height;
}

int screen_dialog_offset_x(void)
{
    return data.dialog_offset.x;
}

int screen_dialog_offset_y(void)
{
    return data.dialog_offset.y;
}
