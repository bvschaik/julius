#include "video.h"

#include "game/settings.h"
#include "game/system.h"
#include "graphics/graphics.h"
#include "graphics/screen.h"
#include "graphics/video.h"
#include "graphics/window.h"

static struct {
    void (*callback)(void);
} data;

static int init(const char *filename, void (*callback)(void))
{
    if (video_start(filename)) {
        data.callback = callback;
        video_init(0);
        return 1;
    }
    return 0;
}

static void draw_background(void)
{
    graphics_clear_screen();
    if (setting_fullscreen()) {
        system_hide_cursor();
    } else {
        system_show_cursor();
    }
}

static void draw_foreground(void)
{
    video_draw(0, 0, screen_width(), screen_height());
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (m->left.went_up || m->right.went_up || video_is_finished()) {
        video_stop();
        system_show_cursor();
        data.callback();
    }
}

void window_video_show(const char *filename, void (*callback)(void))
{
    if (init(filename, callback)) {
        window_type window = {
            WINDOW_VIDEO,
            draw_background,
            draw_foreground,
            handle_input
        };
        window_show(&window);
    } else {
        callback();
    }
}
