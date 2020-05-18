#include "slide.h"

#include "city/view.h"
#include "core/time.h"
#include "graphics/graphics.h"
#include "graphics/menu.h"
#include "graphics/window.h"
#include "sound/effect.h"
#include "widget/sidebar/common.h"
#include "window/city.h"

#define SIDEBAR_SLIDE_STEPS 94

// sliding sidebar progress to x offset translation
static const int PROGRESS_TO_X_OFFSET[SIDEBAR_SLIDE_STEPS] = {
    1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 24, 25,
    27, 28, 30, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49,
    51, 54, 56, 59, 61, 64, 67, 70, 73, 76, 80, 83, 87,
    91, 95, 99, 102, 106, 109, 113, 116, 119, 122, 125,
    127, 130, 132, 135, 137, 139, 141, 143, 144, 146,
    147, 149, 150, 152, 153, 154, 155, 156, 157, 158,
    159, 160, 161, 162, 163, 164, 165
};

static struct {
    time_millis slide_start;
    int progress;
    collapsed_draw_function collapsed_callback;
    expanded_draw_function expanded_callback;
} data;

static void update_progress(void)
{
    time_millis now = time_get_millis();
    time_millis diff = now - data.slide_start;
    data.progress = diff / 5;
}

static void draw_sliding_foreground(void)
{
    window_request_refresh();
    update_progress();
    if (data.progress >= SIDEBAR_SLIDE_STEPS) {
        city_view_toggle_sidebar();
        window_city_show();
        window_draw(1);
        return;
    }

    int height = sidebar_common_get_height();
    int x_offset = sidebar_common_get_x_offset_expanded();
    graphics_set_clip_rectangle(x_offset, TOP_MENU_HEIGHT, SIDEBAR_EXPANDED_WIDTH, sidebar_common_get_height());

    if (city_view_is_sidebar_collapsed()) {
        x_offset += PROGRESS_TO_X_OFFSET[SIDEBAR_SLIDE_STEPS - data.progress];
    } else {
        x_offset += PROGRESS_TO_X_OFFSET[data.progress];
    }

    data.collapsed_callback();
    data.expanded_callback(x_offset);

    graphics_reset_clip_rectangle();
}

void sidebar_slide(collapsed_draw_function collapsed_callback, expanded_draw_function expanded_callback)
{
    data.progress = 0;
    data.slide_start = time_get_millis();
    data.collapsed_callback = collapsed_callback;
    data.expanded_callback = expanded_callback;
    city_view_start_sidebar_toggle();
    sound_effect_play(SOUND_EFFECT_SIDEBAR);

    window_type window = {
        WINDOW_SLIDING_SIDEBAR,
        window_city_draw,
        draw_sliding_foreground,
        0,
        0
    };
    window_show(&window);
}
