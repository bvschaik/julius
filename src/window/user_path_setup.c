#include "user_path_setup.h"

#include "core/encoding.h"
#include "core/file.h"
#include "core/image_group.h"
#include "core/string.h"
#include "game/system.h"
#include "graphics/image_button.h"
#include "graphics/generic_button.h"
#include "graphics/panel.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "platform/file_manager.h"
#include "platform/prefs.h"
#include "platform/user_path.h"
#include "window/plain_message_dialog.h"
#include "window/popup_dialog.h"
#include "window/select_list.h"

#include <string.h>

static struct {
    int show_window;
    int window_status;
    unsigned int button_in_focus;
    int first_time;
    char user_path[FILE_NAME_MAX];
} data;

static void button_pick_option(const generic_button *button);
static void button_ok_cancel(int is_ok, int param2);

static generic_button path_button = {
    150, 55, 458, 30, button_pick_option
};

static image_button ok_cancel_buttons[] = {
    {270, 100, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 0, button_ok_cancel, button_none, 1, 0, 1},
    {330, 100, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 4, button_ok_cancel, button_none, 0, 0, 1},
};

static void init(int first_time)
{
    data.show_window = !first_time;
    snprintf(data.user_path, FILE_NAME_MAX, "%s", pref_user_dir());
    data.button_in_focus = 0;
    data.first_time = first_time;
    if (data.first_time) {
        platform_user_path_copy_campaigns_and_custom_empires();
    }
}

static void cancel(void)
{
    window_go_back();
    if (data.first_time) {
        window_plain_message_dialog_show(TR_USER_DIRECTORIES_CANCELLED_TITLE, TR_USER_DIRECTORIES_CANCELLED_TEXT, 0);
    }
}

static void show_window(int accepted, int checked)
{
    if (!accepted) {
        cancel();
        return;
    }
    data.show_window = 1;
}

static int encode_path(uint8_t *output, const char *input, int output_size)
{
    encoding_from_utf8(input, output, output_size);
    int length = 0;
    while (output[length]) {
        if (output[length] == '\\') {
            output[length] = '/';
        }
        length++;
    }
    return length;
}

static const uint8_t *get_path_text(void)
{
    static uint8_t text[FILE_NAME_MAX];
    const uint8_t *path_text;
    if (*data.user_path) {
        if (strcmp(data.user_path, "./") == 0) {
            path_text = translation_for(TR_CONFIG_USER_PATH_WITH_SUBDIRECTORIES);
        } else if (platform_user_path_recommend() && strcmp(data.user_path, platform_user_path_recommend()) == 0) {
            uint8_t *cursor = string_copy(translation_for(TR_CONFIG_USER_PATH_RECOMMENDED), text, FILE_NAME_MAX);
            cursor += encode_path(cursor, data.user_path, FILE_NAME_MAX - (cursor - text));
            string_copy(string_from_ascii(")"), cursor, FILE_NAME_MAX - (cursor - text));
            path_text = text;
        } else {
            encode_path(text, data.user_path, FILE_NAME_MAX);
            path_text = text;
        }
    } else {
        path_text = translation_for(TR_CONFIG_USER_PATH_DEFAULT);
    }
    return path_text;
}

static void draw_background(void)
{
    graphics_clear_screen();

    if (!data.first_time) {
        window_draw_underlying_window();
    }
    if (!data.show_window) {
        return;
    }
    graphics_in_dialog_with_size(640, 144);

    outer_panel_draw(0, 0, 40, 9);
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_USER_DIRECTORIES_WINDOW_TITLE, 0, 20, 640, FONT_LARGE_BLACK);

    lang_text_draw(CUSTOM_TRANSLATION, TR_USER_DIRETORIES_WINDOW_USER_PATH, 16, 64, FONT_NORMAL_BLACK);

    text_draw_ellipsized(get_path_text(), path_button.x + 10, path_button.y + 9, path_button.width - 20,
        FONT_NORMAL_BLACK, 0);

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    if (!data.show_window) {
        return;
    }

    graphics_in_dialog_with_size(640, 144);

    button_border_draw(path_button.x, path_button.y, path_button.width, path_button.height, data.button_in_focus);
    image_buttons_draw(0, 0, ok_cancel_buttons, 2);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog_with_size(m, 640, 144);
    if (m->right.went_up) {
        window_go_back();
        return;
    }
    if (generic_buttons_handle_mouse(m_dialog, 0, 0, &path_button, 1, &data.button_in_focus) ||
        image_buttons_handle_mouse(m_dialog, 0, 0, ok_cancel_buttons, 2, 0)) {
        return;
    }
    if (h->escape_pressed) {
        hotkey_handle_escape();
    }
}

static const char *get_path_from_dialog(void)
{
    const char *user_path = data.user_path;
    if (!*user_path) {
        user_path = pref_user_dir();
    }
    if (!*user_path || strcmp(user_path, "./") == 0) {
        user_path = pref_data_dir();
    }
    const char *path = system_show_select_folder_dialog("Please select your user path folder:", user_path);
    if (!path) {
        return 0;
    }
    return path;
}

static void set_paths(int index)
{
    const char *path = "";
    if (index == 0) {
        path = "";
    } else if (index == 1) {
        path = "./";
    } else if (index == 2) {
        const char *recommended = platform_user_path_recommend();
        if (recommended) {
            path = recommended;
        } else if (system_supports_select_folder_dialog()) {
            path = get_path_from_dialog();
        }
    } else if (index == 3 && system_supports_select_folder_dialog()) {
        path = get_path_from_dialog();
    }
    if (!path) {
        return;
    }
    if (*path && strcmp(path, pref_data_dir()) == 0) {
        snprintf(data.user_path, FILE_NAME_MAX, "./");
    } else {
        snprintf(data.user_path, FILE_NAME_MAX, "%s", path);
    }
}

static void button_pick_option(const generic_button *button)
{
    static const uint8_t *texts[4];
    static uint8_t recommended_text[FILE_NAME_MAX];
    const char *recommended = platform_user_path_recommend();
    static int total_options = 2;
    if (!texts[0]) {
        texts[0] = translation_for(TR_CONFIG_USER_PATH_DEFAULT);
        texts[1] = translation_for(TR_CONFIG_USER_PATH_WITH_SUBDIRECTORIES);
        if (recommended) {
            texts[total_options++] = recommended_text;
            uint8_t *cursor = string_copy(translation_for(TR_CONFIG_USER_PATH_RECOMMENDED),
                recommended_text, FILE_NAME_MAX);
            cursor += encode_path(cursor, recommended, FILE_NAME_MAX - (cursor - recommended_text));
            string_copy(string_from_ascii(")"), cursor, FILE_NAME_MAX - (cursor - recommended_text));
        }
        if (system_supports_select_folder_dialog()) {
            texts[total_options++] = translation_for(TR_CONFIG_USER_PATH_CUSTOM);
        }
    };

    window_select_list_show_text(screen_dialog_offset_x(), screen_dialog_offset_y(), button, texts, total_options,
        set_paths);
}

static void copy_user_files(int accepted, int overwrite)
{
    window_go_back();
    char original_path[FILE_NAME_MAX];
    snprintf(original_path, FILE_NAME_MAX, "%s", pref_user_dir());
    pref_save_user_dir(data.user_path);
    platform_user_path_create_subdirectories();
    if (!accepted) {
        return;
    }
    platform_user_path_copy_files(original_path, overwrite);
}

static void button_ok_cancel(int is_ok, int param2)
{
    if (!is_ok) {
        cancel();
        return;
    }
    if (strcmp(data.user_path, pref_user_dir()) == 0) {
        window_go_back();
        return;
    }
    if (!platform_file_manager_is_directory_writeable(data.user_path)) {
        window_plain_message_dialog_show(TR_USER_DIRECTORIES_NOT_WRITEABLE_TITLE,
            TR_USER_DIRECTORIES_NOT_WRITEABLE_TEXT, !data.first_time);
        return;
    }
    window_popup_dialog_show_confirmation(translation_for(TR_USER_DIRECTORIES_USER_PATH_CHANGED_TITLE),
        translation_for(TR_USER_DIRECTORIES_USER_PATH_CHANGED_TEXT),
        translation_for(TR_USER_DIRECTORIES_USER_PATH_CHANGED_OVERWRITE), copy_user_files);
}

static void get_tooltip(tooltip_context *c)
{
    if (data.button_in_focus) {
        const uint8_t *path_text = get_path_text();
        if (text_get_width(path_text, FONT_NORMAL_BLACK) > path_button.width - 20) {
            c->precomposed_text = path_text;
            c->type = TOOLTIP_BUTTON;
        }
    }
}

void window_user_path_setup_show(int first_time)
{
    window_type window = {
        WINDOW_USER_PATH_SETUP,
        draw_background,
        draw_foreground,
        handle_input,
        get_tooltip
    };
    init(first_time);
    window_show(&window);
    if (data.first_time) {
        window_popup_dialog_show_confirmation(translation_for(TR_USER_DIRECTORIES_NOT_SET_UP_TITLE),
            translation_for(TR_USER_DIRECTORIES_NOT_SET_UP_TEXT), 0, show_window);
    }
}
