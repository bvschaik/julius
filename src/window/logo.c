#include "logo.h"

#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/window.h"
#include "sound/music.h"

static void init()
{
    sound_music_play_intro();
}

static void draw_background()
{
    graphics_clear_screen();

    graphics_in_dialog();
    image_draw(image_group(GROUP_LOGO), 0, 0);
    lang_text_draw_centered_colored(13, 7, 160, 462, 320, FONT_NORMAL_PLAIN, COLOR_WHITE);
    graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
    if (m->left.went_up || m->right.went_up) {
        UI_Window_goTo(Window_MainMenu);
    }
}

void window_logo_show()
{
    window_type window = {
        Window_Logo,
        draw_background,
        0,
        handle_mouse
    };
    init();
    window_show(&window);
}
