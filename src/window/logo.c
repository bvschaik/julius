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
#include "window/popup_dialog.h"
#include "window/user_path_setup.h"

static int pending_actions;

static void init(int actions)
{
    pending_actions = actions;
    sound_music_play_intro();
}

static int process_pending_actions(void)
{
    if (pending_actions & ACTION_SHOW_MESSAGE_MISSING_PATCH) {
        window_plain_message_dialog_show(TR_NO_PATCH_TITLE, TR_NO_PATCH_MESSAGE, 0);
        pending_actions ^= ACTION_SHOW_MESSAGE_MISSING_PATCH;
        return 1;
    }
    if (pending_actions & ACTION_SHOW_MESSAGE_MISSING_FONTS) {
        window_plain_message_dialog_show(TR_MISSING_FONTS_TITLE, TR_MISSING_FONTS_MESSAGE, 0);
        pending_actions ^= ACTION_SHOW_MESSAGE_MISSING_FONTS;
        return 1;
    }
    if (pending_actions & ACTION_SHOW_MESSAGE_MISSING_EXTRA_ASSETS) {
        window_plain_message_dialog_show(TR_NO_EXTRA_ASSETS_TITLE, TR_NO_EXTRA_ASSETS_MESSAGE, 0);
        pending_actions ^= ACTION_SHOW_MESSAGE_MISSING_EXTRA_ASSETS;
        return 1;
    }
    if (pending_actions & ACTION_SETUP_USER_DIR) {
        window_user_path_setup_show(1);
        pending_actions ^= ACTION_SETUP_USER_DIR;
        return 1;
    }
    if (pending_actions & ACTION_SHOW_MESSAGE_USER_DIR_NOT_WRITABLE) {
        window_plain_message_dialog_show(TR_USER_DIRECTORIES_NOT_WRITEABLE_TITLE,
            TR_USER_DIRECTORIES_NOT_WRITEABLE_TEXT_DETAILED, 0);
        pending_actions ^= ACTION_SHOW_MESSAGE_USER_DIR_NOT_WRITABLE;
        return 1;
    }
    if (pending_actions & ACTION_SHOW_INTRO_VIDEOS) {
        window_intro_video_show();
        pending_actions ^= ACTION_SHOW_INTRO_VIDEOS;
        return 1;
    }
    return 0;
}

static void draw_background(void)
{
    if (process_pending_actions()) {
        return;
    }
    graphics_clear_screen();

    graphics_in_dialog();
    image_draw(image_group(GROUP_LOGO), 0, 0, COLOR_MASK_NONE, SCALE_NONE);
    lang_text_draw_centered_colored(13, 7, 160, 462, 320, FONT_NORMAL_PLAIN, COLOR_WHITE);
    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (pending_actions) {
        return;
    }
    if (m->left.went_up || m->right.went_up) {
        window_main_menu_show(0);
        return;
    }
    if (h->escape_pressed) {
        hotkey_handle_escape();
    }
}

void window_logo_show(int actions)
{
    window_type window = {
        WINDOW_LOGO,
        draw_background,
        0,
        handle_input
    };
    init(actions);
    window_show(&window);
}
