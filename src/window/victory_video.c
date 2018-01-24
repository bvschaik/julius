#include "victory_video.h"

#include "graphics/graphics.h"
#include "graphics/screen.h"
#include "graphics/video.h"
#include "graphics/window.h"

static struct {
    int width;
    int height;
    void (*callback)(void);
} data;

static int init(const char *filename, int width, int height, void (*callback)(void))
{
    if (video_start(filename)) {
        data.width = width;
        data.height = height;
        data.callback = callback;
        video_init();
        return 1;
    }
    return 0;
}

static void draw_background()
{
    graphics_clear_screen();
}

static void draw_foreground()
{
    video_draw((screen_width() - data.width) / 2, (screen_height() - data.height) / 2);
}

static void handle_mouse(const mouse *m)
{
    if (m->left.went_up || m->right.went_up || video_is_finished()) {
        video_stop();
        data.callback();
    }
}

void window_victory_video_show(const char *filename, int width, int height, void (*callback)(void))
{
    if (init(filename, width, height, callback)) {
        window_type window = {
            Window_VideoIntermezzo,
            draw_background,
            draw_foreground,
            handle_mouse,
            0
        };
        video_init();
        window_show(&window);
    } else {
        callback();
    }
}
