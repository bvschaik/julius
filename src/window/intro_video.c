#include "intro_video.h"

#include "graphics/graphics.h"
#include "graphics/screen.h"
#include "graphics/video.h"
#include "graphics/window.h"

static struct {
    int width;
    int height;
} data;

static int init(void)
{
    if (video_start("smk/intro.smk")) {
        video_size(&data.width, &data.height);
        video_init();
        return 1;
    }
    return 0;
}

static void draw_background(void)
{
    graphics_clear_screen();
}

static void draw_foreground(void)
{
    video_draw((screen_width() - data.width) / 2, (screen_height() - data.height) / 2);
}

static void handle_mouse(const mouse *m)
{
    if (m->left.went_up || m->right.went_up || video_is_finished()) {
        video_stop();
        window_go_back();
    }
}

void window_intro_video_show(void)
{
    if (init()) {
        window_type window = {
            WINDOW_INTRO_VIDEO,
            draw_background,
            draw_foreground,
            handle_mouse,
            0
        };
        window_show(&window);
    }
}
