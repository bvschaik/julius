#include "logo.h"

#include "core/config.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/window.h"
#include "sound/music.h"
#include "window/intro_video.h"
#include "window/main_menu.h"
#include "window/plain_message_dialog.h"

static void init(void)
{
    sound_music_play_intro();
}

static void draw_background(void)
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
        window_main_menu_show(0);
    }
}

void window_logo_show(int show_patch_message)
{
    window_type window = {
        WINDOW_LOGO,
        draw_background,
        0,
        handle_mouse
    };
    init();
    window_show(&window);
    if (show_patch_message == MESSAGE_MISSING_PATCH) {
        window_plain_message_dialog_show(
            "Patch 1.0.1.0 not installed",
            "Your Caesar 3 installation does not have the 1.0.1.0 patch installed. "
            "You can download the patch from:\n"
            "https://bintray.com/bvschaik/caesar3-patches\n"
            "Continue at your own risk."
        );
    } else if (show_patch_message == MESSAGE_MISSING_FONTS) {
        window_plain_message_dialog_show(
            "Missing fonts",
            "Your Caesar 3 installation requires extra font files. "
            "You can download them for your language from:\n"
            "https://bintray.com/bvschaik/caesar3-patches"
        );
    }
    if (config_get(CONFIG_UI_SHOW_INTRO_VIDEO)) {
        window_intro_video_show();
    }
}
