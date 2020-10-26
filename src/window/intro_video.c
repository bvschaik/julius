#include "intro_video.h"

#include "graphics/graphics.h"
#include "graphics/screen.h"
#include "graphics/video.h"
#include "graphics/window.h"
#include "sound/music.h"

#define NUM_INTRO_VIDEOS 3

static int current_video;

static const char *intro_videos[NUM_INTRO_VIDEOS] = { "smk/logo.smk", "smk/intro.smk", "smk/credits.smk" };

static int start_next_video(void)
{
    graphics_clear_screens();
    while (current_video < NUM_INTRO_VIDEOS) {
        if (video_start(intro_videos[current_video++])) {
            video_init();
            return 1;
        }
    }
    return 0;
}

static int init(void)
{
    current_video = 0;
    return start_next_video();
}

static void draw_background(void)
{
    graphics_clear_screens();
}

static void draw_foreground(void)
{
    video_draw_fullscreen();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (m->left.went_up || m->right.went_up || video_is_finished() || h->enter_pressed) {
        sound_music_stop();
        video_stop();
        if (!start_next_video()) {
            sound_music_play_intro();
            window_go_back();
        }
    }
}

void window_intro_video_show(void)
{
    if (init()) {
        window_type window = {
            WINDOW_INTRO_VIDEO,
            draw_background,
            draw_foreground,
            handle_input
        };
        window_show(&window);
    }
}
