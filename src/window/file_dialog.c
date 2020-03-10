#include "file_dialog.h"

#include "core/calc.h"
#include "core/dir.h"
#include "core/encoding.h"
#include "core/file.h"
#include "core/image_group.h"
#include "core/lang.h"
#include "core/string.h"
#include "core/time.h"
#include "game/file.h"
#include "game/file_editor.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/keyboard.h"
#include "window/city.h"
#include "window/editor/map.h"

#include <string.h>

static void button_ok_cancel(int is_ok, int param2);
static void button_scroll(int is_down, int num_lines);
static void button_select_file(int index, int param2);

static image_button image_buttons[] = {
    {344, 335, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 0, button_ok_cancel, button_none, 1, 0, 1},
    {392, 335, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 4, button_ok_cancel, button_none, 0, 0, 1},
    {464, 120, 39, 26, IB_SCROLL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 8, button_scroll, button_none, 0, 1, 1},
    {464, 300, 39, 26, IB_SCROLL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 12, button_scroll, button_none, 1, 1, 1},
};
static generic_button file_buttons[] = {
    {160, 128, 288, 16, button_select_file, button_none, 0, 0},
    {160, 144, 288, 16, button_select_file, button_none, 1, 0},
    {160, 160, 288, 16, button_select_file, button_none, 2, 0},
    {160, 176, 288, 16, button_select_file, button_none, 3, 0},
    {160, 192, 288, 16, button_select_file, button_none, 4, 0},
    {160, 208, 288, 16, button_select_file, button_none, 5, 0},
    {160, 224, 288, 16, button_select_file, button_none, 6, 0},
    {160, 240, 288, 16, button_select_file, button_none, 7, 0},
    {160, 256, 288, 16, button_select_file, button_none, 8, 0},
    {160, 272, 288, 16, button_select_file, button_none, 9, 0},
    {160, 288, 288, 16, button_select_file, button_none, 10, 0},
    {160, 304, 288, 16, button_select_file, button_none, 11, 0},
};

static const time_millis NOT_EXIST_MESSAGE_TIMEOUT = 500;
static const int MAX_FILE_WINDOW_TEXT_WIDTH = 18 * 16;

typedef struct {
    char extension[4];
    char last_loaded_file[FILE_NAME_MAX];
} file_type_data;

static struct {
    time_millis message_not_exist_start_time;
    file_type type;
    file_dialog_type dialog_type;
    int focus_button_id;
    int scroll_position;
    const dir_listing *file_list;

    file_type_data *file_data;
    uint8_t typed_name[FILE_NAME_MAX];
    char selected_file[FILE_NAME_MAX];
} data;

file_type_data saved_game_data = {"sav"};
file_type_data saved_game_data_expanded = {"svx"};
file_type_data scenario_data = {"map"};

static void init(file_type type, file_dialog_type dialog_type)
{
    data.type = type;
    data.file_data = type == FILE_TYPE_SCENARIO ? &scenario_data : &saved_game_data;
    if (strlen(data.file_data->last_loaded_file) == 0) {
        string_copy(lang_get_string(9, type == FILE_TYPE_SCENARIO ? 7 : 6), data.typed_name, FILE_NAME_MAX);
        encoding_to_utf8(data.typed_name, data.file_data->last_loaded_file, FILE_NAME_MAX, 0);
    } else {
        encoding_from_utf8(data.file_data->last_loaded_file, data.typed_name, FILE_NAME_MAX);
        file_remove_extension(data.typed_name);
    }
    data.dialog_type = dialog_type;
    data.message_not_exist_start_time = 0;
    data.scroll_position = 0;

    if (data.dialog_type != FILE_DIALOG_SAVE) {
        data.file_list = dir_find_files_with_extension(data.file_data->extension);
        data.file_list = dir_append_files_with_extension(saved_game_data_expanded.extension);
    }
    else {
        data.file_list = dir_find_files_with_extension(saved_game_data_expanded.extension);
    }

    strncpy(data.selected_file, data.file_data->last_loaded_file, FILE_NAME_MAX);
    keyboard_start_capture(data.typed_name, FILE_NAME_MAX, 0, MAX_FILE_WINDOW_TEXT_WIDTH, FONT_NORMAL_WHITE);
}

static void draw_scrollbar_dot(void)
{
    if (data.file_list->num_files > 12) {
        int pct;
        if (data.scroll_position <= 0) {
            pct = 0;
        } else if (data.scroll_position + 12 >= data.file_list->num_files) {
            pct = 100;
        } else {
            pct = calc_percentage(data.scroll_position, data.file_list->num_files - 12);
        }
        int y_offset = calc_adjust_with_percentage(130, pct);
        image_draw(image_group(GROUP_PANEL_BUTTON) + 39, 472, 145 + y_offset);
    }
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    uint8_t file[FILE_NAME_MAX];

    outer_panel_draw(128, 40, 24, 21);
    inner_panel_draw(144, 80, 20, 2);
    inner_panel_draw(144, 120, 20, 13);

    // title
    if (data.message_not_exist_start_time && time_get_millis() - data.message_not_exist_start_time < NOT_EXIST_MESSAGE_TIMEOUT) {
        lang_text_draw_centered(43, 2, 160, 50, 304, FONT_LARGE_BLACK);
    } else if (data.dialog_type == FILE_DIALOG_DELETE) {
        lang_text_draw_centered(43, 6, 160, 50, 304, FONT_LARGE_BLACK);
    } else {
        int text_id = data.dialog_type + (data.type == FILE_TYPE_SCENARIO ? 3 : 0);
        lang_text_draw_centered(43, text_id, 160, 50, 304, FONT_LARGE_BLACK);
    }
    lang_text_draw(43, 5, 224, 342, FONT_NORMAL_BLACK);

    for (int i = 0; i < 12; i++) {
        font_t font = FONT_NORMAL_GREEN;
        if (data.focus_button_id == i + 1) {
            font = FONT_NORMAL_WHITE;
        }
        encoding_from_utf8(data.file_list->files[data.scroll_position + i], file, FILE_NAME_MAX);
        //file_remove_extension(file);
        text_ellipsize(file, font, MAX_FILE_WINDOW_TEXT_WIDTH);
        text_draw(file, 160, 130 + 16 * i, font, 0);
    }

    image_buttons_draw(0, 0, image_buttons, 4);
    text_capture_cursor(keyboard_cursor_position(), keyboard_offset_start(), keyboard_offset_end());
    text_draw(data.typed_name, 160, 90, FONT_NORMAL_WHITE, 0);
    text_draw_cursor(160, 91, keyboard_is_insert());
    draw_scrollbar_dot();

    graphics_reset_dialog();
}

static int handle_scrollbar(const mouse *m)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (data.file_list->num_files <= 12) {
        return 0;
    }
    if (!m_dialog->left.is_down) {
        return 0;
    }
    if (m_dialog->x >= 464 && m_dialog->x <= 496 && m_dialog->y >= 145 && m_dialog->y <= 300) {
        int y_offset = m_dialog->y - 145;
        if (y_offset > 130) {
            y_offset = 130;
        }
        int pct = calc_percentage(y_offset, 130);
        data.scroll_position = calc_adjust_with_percentage(data.file_list->num_files - 12, pct);
        return 1;
    }
    return 0;
}

static void handle_mouse(const mouse *m)
{
    if (m->scrolled == SCROLL_DOWN) {
        button_scroll(1, 3);
    } else if (m->scrolled == SCROLL_UP) {
        button_scroll(0, 3);
    }

    if (keyboard_input_is_accepted()) {
        button_ok_cancel(1, 0);
        return;
    }

    if (m->right.went_up) {
        window_go_back();
        return;
    }
    const mouse *m_dialog = mouse_in_dialog(m);
    if (!generic_buttons_handle_mouse(m_dialog, 0, 0, file_buttons, 12, &data.focus_button_id)) {
        if (!image_buttons_handle_mouse(m_dialog, 0, 0, image_buttons, 4, 0)) {
            handle_scrollbar(m);
        }
    }
}

static const char *get_chosen_filename(void)
{
    // Check if we should work with the selected file
    uint8_t selected_name[FILE_NAME_MAX];
    encoding_from_utf8(data.selected_file, selected_name, FILE_NAME_MAX);

    if (string_equals(selected_name, data.typed_name)) {
        // user has not modified the string after selecting it: use filename
        return data.selected_file;
    }

    // We should use the typed name, which needs to be converted to UTF-8...
#ifdef __APPLE__
    int use_decomposed = 1;
#else
    int use_decomposed = 0;
#endif
    static char typed_file[FILE_NAME_MAX];
    encoding_to_utf8(data.typed_name, typed_file, FILE_NAME_MAX, use_decomposed);
    return typed_file;
}

static void button_ok_cancel(int is_ok, int param2)
{
    if (!is_ok) {
        window_go_back();
        return;
    }

    const char *filename = get_chosen_filename();

    if (data.dialog_type != FILE_DIALOG_SAVE && !file_exists(filename)) {
        data.message_not_exist_start_time = time_get_millis();
        return;
    }
    if (data.dialog_type == FILE_DIALOG_LOAD) {
        if (data.type == FILE_TYPE_SAVED_GAME) {
            if (game_file_load_saved_game(filename)) {
                keyboard_stop_capture();
                window_city_show();
            } else {
                data.message_not_exist_start_time = time_get_millis();
                return;
            }
        } else if (data.type == FILE_TYPE_SCENARIO) {
            if (game_file_editor_load_scenario(filename)) {
                keyboard_stop_capture();
                window_editor_map_show();
            } else {
                data.message_not_exist_start_time = time_get_millis();
                return;
            }
        }
    } else if (data.dialog_type == FILE_DIALOG_SAVE) {
        keyboard_stop_capture();
        if (data.type == FILE_TYPE_SAVED_GAME) {
            if (!file_has_extension(filename, saved_game_data_expanded.extension)) {
                file_append_extension(filename, saved_game_data_expanded.extension);
            }
            game_file_write_saved_game(filename);
            window_city_show();
        } else if (data.type == FILE_TYPE_SCENARIO) {
            game_file_editor_write_scenario(filename);
            window_editor_map_show();
        }
    } else if (data.dialog_type == FILE_DIALOG_DELETE) {
        if (game_file_delete_saved_game(filename)) {
            dir_find_files_with_extension(data.file_data->extension);
            dir_append_files_with_extension(saved_game_data_expanded.extension);

            if (data.scroll_position + 12 >= data.file_list->num_files) {
                --data.scroll_position;
            }
            if (data.scroll_position < 0) {
                data.scroll_position = 0;
            }
        }
    }

    strncpy(data.file_data->last_loaded_file, filename, FILE_NAME_MAX);
}

static void button_scroll(int is_down, int num_lines)
{
    if (data.file_list->num_files > 12) {
        if (is_down) {
            data.scroll_position += num_lines;
            if (data.scroll_position > data.file_list->num_files - 12) {
                data.scroll_position = data.file_list->num_files - 12;
            }
        } else {
            data.scroll_position -= num_lines;
            if (data.scroll_position < 0) {
                data.scroll_position = 0;
            }
        }
        data.message_not_exist_start_time = 0;
    }
}

static void button_select_file(int index, int param2)
{
    if (index < data.file_list->num_files) {
        strncpy(data.selected_file, data.file_list->files[data.scroll_position + index], FILE_NAME_MAX - 1);
        encoding_from_utf8(data.selected_file, data.typed_name, FILE_NAME_MAX);
        keyboard_refresh();
        data.message_not_exist_start_time = 0;
    }
}

void window_file_dialog_show(file_type type, file_dialog_type dialog_type)
{
    window_type window = {
        WINDOW_FILE_DIALOG,
        window_draw_underlying_window,
        draw_foreground,
        handle_mouse
    };
    init(type, dialog_type);
    window_show(&window);
}
