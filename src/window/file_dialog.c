#include "file_dialog.h"

#include "core/calc.h"
#include "core/dir.h"
#include "core/encoding.h"
#include "core/file.h"
#include "core/image_group.h"
#include "core/lang.h"
#include "core/string.h"
#include "core/time.h"
#include "editor/empire.h"
#include "empire/empire_xml.h"
#include "game/file.h"
#include "game/file_editor.h"
#include "game/file_io.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/scrollbar.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "platform/file_manager.h"
#include "scenario/editor.h"
#include "translation/translation.h"
#include "widget/input_box.h"
#include "window/city.h"
#include "window/editor/map.h"
#include "widget/minimap.h"
#include "window/plain_message_dialog.h"

#include <string.h>

#define NUM_FILES_IN_VIEW 21
#define MAX_FILE_WINDOW_TEXT_WIDTH (16 * BLOCK_SIZE)

static const time_millis NOT_EXIST_MESSAGE_TIMEOUT = 500;

static void button_ok_cancel(int is_ok, int param2);
static void button_select_file(int index, int param2);
static void on_scroll(void);

static image_button image_buttons[] = {
    {536, 440, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 0, button_ok_cancel, button_none, 1, 0, 1},
    {584, 440, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 4, button_ok_cancel, button_none, 0, 0, 1},
};
static generic_button file_buttons[] = {
    {32, 88 + 16 * 0, 256, 16, button_select_file, button_none, 0, 0},
    {32, 88 + 16 * 1, 256, 16, button_select_file, button_none, 1, 0},
    {32, 88 + 16 * 2, 256, 16, button_select_file, button_none, 2, 0},
    {32, 88 + 16 * 3, 256, 16, button_select_file, button_none, 3, 0},
    {32, 88 + 16 * 4, 256, 16, button_select_file, button_none, 4, 0},
    {32, 88 + 16 * 5, 256, 16, button_select_file, button_none, 5, 0},
    {32, 88 + 16 * 6, 256, 16, button_select_file, button_none, 6, 0},
    {32, 88 + 16 * 7, 256, 16, button_select_file, button_none, 7, 0},
    {32, 88 + 16 * 8, 256, 16, button_select_file, button_none, 8, 0},
    {32, 88 + 16 * 9, 256, 16, button_select_file, button_none, 9, 0},
    {32, 88 + 16 * 10, 256, 16, button_select_file, button_none, 10, 0},
    {32, 88 + 16 * 11, 256, 16, button_select_file, button_none, 11, 0},
    {32, 88 + 16 * 12, 256, 16, button_select_file, button_none, 12, 0},
    {32, 88 + 16 * 13, 256, 16, button_select_file, button_none, 13, 0},
    {32, 88 + 16 * 14, 256, 16, button_select_file, button_none, 14, 0},
    {32, 88 + 16 * 15, 256, 16, button_select_file, button_none, 15, 0},
    {32, 88 + 16 * 16, 256, 16, button_select_file, button_none, 16, 0},
    {32, 88 + 16 * 17, 256, 16, button_select_file, button_none, 17, 0},
    {32, 88 + 16 * 18, 256, 16, button_select_file, button_none, 18, 0},
    {32, 88 + 16 * 19, 256, 16, button_select_file, button_none, 19, 0},
    {32, 88 + 16 * 20, 256, 16, button_select_file, button_none, 20, 0},
};

static scrollbar_type scrollbar = { 304, 80, 350, 256, NUM_FILES_IN_VIEW, on_scroll, 1 };

typedef struct {
    char extension[5];
    char last_loaded_file[FILE_NAME_MAX];
} file_type_data;

static struct {
    time_millis message_not_exist_start_time;
    file_type type;
    file_dialog_type dialog_type;
    int focus_button_id;
    int double_click;
    const dir_listing *file_list;

    file_type_data *file_data;
    uint8_t typed_name[FILE_NAME_MAX];
    uint8_t previously_seen_typed_name[FILE_NAME_MAX];
    char selected_file[FILE_NAME_MAX];
    union {
        saved_game_info save_game;
        scenario_info scenario;
    } info;
    int has_valid_info;
    int redraw_full_window;
} data;

static const int MISSION_ID_TO_CITY_ID[] = {
    0, 3, 2, 1, 7, 10, 18, 4, 30, 6, 12, 14, 16, 27, 31, 23, 36, 38, 28, 25
};

static input_box file_name_input = { 16, 40, 38, 2, FONT_NORMAL_WHITE, 0, data.typed_name, FILE_NAME_MAX };

static file_type_data saved_game_data = { "sav" };
static file_type_data saved_game_data_expanded = { "svx" };
static file_type_data scenario_data = { "map" };
static file_type_data scenario_data_expanded = { "mapx" };
static file_type_data empire_data = { "xml" };

static int find_first_file_with_prefix(const char *prefix)
{
    int len = (int) strlen(prefix);
    if (len == 0) {
        return -1;
    }
    int left = 0;
    int right = data.file_list->num_files;
    while (left < right) {
        int middle = (left + right) / 2;
        if (platform_file_manager_compare_filename_prefix(data.file_list->files[middle], prefix, len) >= 0) {
            right = middle;
        } else {
            left = middle + 1;
        }
    }
    if (platform_file_manager_compare_filename_prefix(data.file_list->files[left], prefix, len) == 0) {
        return left;
    } else {
        return -1;
    }
}

static void scroll_to_typed_text(void)
{
    if (data.file_list->num_files <= NUM_FILES_IN_VIEW) {
        // No need to scroll
        return;
    }
    char name_utf8[FILE_NAME_MAX];
    encoding_to_utf8(data.typed_name, name_utf8, FILE_NAME_MAX, encoding_system_uses_decomposed());
    int index = find_first_file_with_prefix(name_utf8);
    if (index >= 0) {
        scrollbar_reset(&scrollbar, calc_bound(index, 0, data.file_list->num_files - NUM_FILES_IN_VIEW));
    }
}

static void init(file_type type, file_dialog_type dialog_type)
{
    data.type = type;
    if (type == FILE_TYPE_SCENARIO) {
        data.file_data = &scenario_data;
    } else if (type == FILE_TYPE_EMPIRE) {
        data.file_data = &empire_data;
    } else {
        data.file_data = &saved_game_data;
    }
    data.dialog_type = dialog_type;

    data.message_not_exist_start_time = 0;
    data.double_click = 0;
    data.focus_button_id = 0;

    if (strlen(data.file_data->last_loaded_file) > 0) {
        encoding_from_utf8(data.file_data->last_loaded_file, data.typed_name, FILE_NAME_MAX);
        if (data.dialog_type == FILE_DIALOG_SAVE) {
            file_remove_extension((char *) data.typed_name);
        }
    } else if (dialog_type == FILE_DIALOG_SAVE) {
        // Suggest default filename
        string_copy(lang_get_string(9, type == FILE_TYPE_SCENARIO ? 7 : 6), data.typed_name, FILE_NAME_MAX);
        if (type == FILE_TYPE_SAVED_GAME) {
            file_append_extension((char *) data.typed_name, saved_game_data_expanded.extension);
        } else if (type == FILE_TYPE_SCENARIO) {
            file_append_extension((char *)data.typed_name, scenario_data_expanded.extension);
        }
        encoding_to_utf8(data.typed_name, data.file_data->last_loaded_file, FILE_NAME_MAX, 0);
    } else {
        // Use empty string
        data.typed_name[0] = 0;
    }
    string_copy(data.typed_name, data.previously_seen_typed_name, FILE_NAME_MAX);

    if (data.dialog_type != FILE_DIALOG_SAVE) {
        if (type == FILE_TYPE_SCENARIO) {
            data.file_list = dir_find_files_with_extension(".", scenario_data.extension);
            data.file_list = dir_append_files_with_extension(scenario_data_expanded.extension);
        } else if (type == FILE_TYPE_EMPIRE) {
            data.file_list = dir_find_files_with_extension("custom_empires", empire_data.extension);
        } else {
            data.file_list = dir_find_files_with_extension(".", data.file_data->extension);
            data.file_list = dir_append_files_with_extension(saved_game_data_expanded.extension);
        }
    } else {
        if (type == FILE_TYPE_SCENARIO) {
            data.file_list = dir_find_files_with_extension(".", scenario_data_expanded.extension);
        } else {
            data.file_list = dir_find_files_with_extension(".", saved_game_data_expanded.extension);
        }
    }
    scrollbar_init(&scrollbar, 0, data.file_list->num_files);
    scroll_to_typed_text();

    strncpy(data.selected_file, data.file_data->last_loaded_file, FILE_NAME_MAX - 1);
    input_box_start(&file_name_input);
}

static void draw_mission_info(int x_offset, int y_offset, int box_size)
{
    if (data.info.save_game.custom_mission) {
        text_draw_centered(translation_for(TR_SAVE_DIALOG_CUSTOM_SCENARIO),
            x_offset, y_offset, box_size, FONT_NORMAL_BLACK, 0);
        return;
    }
    if (data.info.save_game.mission == 0) {
        text_draw_centered(translation_for(TR_SAVE_DIALOG_FIRST_MISSION),
            x_offset, y_offset, box_size, FONT_NORMAL_BLACK, 0);
        return;
    }
    translation_key mission_type;
    if (data.info.save_game.mission == 1) {
        mission_type = TR_SAVE_DIALOG_MISSION;
    } else if (data.info.save_game.mission % 2) {
        mission_type = TR_SAVE_DIALOG_MILITARY;
    } else {
        mission_type = TR_SAVE_DIALOG_PEACEFUL;
    }
    int width = text_draw(translation_for(mission_type), x_offset, y_offset, FONT_NORMAL_BLACK, 0);
    width += text_draw_number(data.info.save_game.mission / 2 + 2, '\0', " -", x_offset + width, y_offset,
        FONT_NORMAL_BLACK, COLOR_MASK_NONE);
    lang_text_draw(21, MISSION_ID_TO_CITY_ID[data.info.save_game.mission], x_offset + width, y_offset, FONT_NORMAL_BLACK);
}

static void draw_background(void)
{
    window_draw_underlying_window();
    if (*data.selected_file) {
        if (data.type == FILE_TYPE_SAVED_GAME) {
            data.has_valid_info = game_file_io_read_saved_game_info(data.selected_file, &data.info.save_game);
        } else {
            data.has_valid_info = game_file_io_read_scenario_info(data.selected_file, &data.info.scenario);
        }
    }
    data.redraw_full_window = 1;
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    if (data.redraw_full_window) {
        uint8_t file[FILE_NAME_MAX];

        outer_panel_draw(0, 0, 40, 30);
        inner_panel_draw(16, 80, 18, 22);

        // title
        if (data.message_not_exist_start_time
            && time_get_millis() - data.message_not_exist_start_time < NOT_EXIST_MESSAGE_TIMEOUT) {
            lang_text_draw_centered(43, 2, 32, 10, 554, FONT_LARGE_BLACK);
        } else if (data.dialog_type == FILE_DIALOG_DELETE) {
            lang_text_draw_centered(43, 6, 32, 10, 554, FONT_LARGE_BLACK);
        } else if (data.type == FILE_TYPE_EMPIRE) {
            lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_CUSTOM_EMPIRE_TITLE, 32, 10, 554, FONT_LARGE_BLACK);
        } else {
            int text_id = data.dialog_type + (data.type == FILE_TYPE_SCENARIO ? 3 : 0);
            lang_text_draw_centered(43, text_id, 32, 10, 554, FONT_LARGE_BLACK);
        }
        lang_text_draw_centered(43, 5, 362, 447, 164, FONT_NORMAL_BLACK);

        for (int i = 0; i < NUM_FILES_IN_VIEW; i++) {
            font_t font = FONT_NORMAL_GREEN;
            if (data.focus_button_id == i + 1) {
                font = FONT_NORMAL_WHITE;
            }
            encoding_from_utf8(data.file_list->files[scrollbar.scroll_position + i], file, FILE_NAME_MAX);
            text_ellipsize(file, font, MAX_FILE_WINDOW_TEXT_WIDTH);
            text_draw(file, 32, 90 + 16 * i, font, 0);
        }

        // Saved game info
        if (*data.selected_file && data.type != FILE_TYPE_EMPIRE) {
            if (data.has_valid_info) {
                if (data.type == FILE_TYPE_SAVED_GAME) {
                    draw_mission_info(362, 356, 246);
                    text_draw(translation_for(TR_SAVE_DIALOG_FUNDS), 362, 376, FONT_NORMAL_BLACK, 0);
                    text_draw_money(data.info.save_game.treasury, 494, 376, FONT_NORMAL_BLACK);
                    text_draw(translation_for(TR_SAVE_DIALOG_DATE), 362, 396, FONT_NORMAL_BLACK, 0);
                    lang_text_draw_month_year_max_width(data.info.save_game.month, data.info.save_game.year,
                        500, 396, 108, FONT_NORMAL_BLACK, 0);
                    text_draw(translation_for(TR_SAVE_DIALOG_POPULATION), 362, 416, FONT_NORMAL_BLACK, 0);
                    text_draw_number(data.info.save_game.population, '\0', "",
                        500, 416, FONT_NORMAL_BLACK, COLOR_MASK_NONE);
                    widget_minimap_draw(352, 80, 266, 272);
                } else {
                    widget_minimap_draw(352, 80, 266, 352);
                }
            } else {
                text_draw_centered(translation_for(TR_SAVE_DIALOG_INVALID_FILE), 362, 241, 246, FONT_LARGE_BLACK, 0);
            }
        } else {
            text_draw_centered(translation_for(TR_SAVE_DIALOG_SELECT_FILE), 362, 246, 246, FONT_NORMAL_BLACK, 0);
        }
        data.redraw_full_window = 0;
    }

    input_box_draw(&file_name_input);
    image_buttons_draw(0, 0, image_buttons, 2);
    scrollbar_draw(&scrollbar);
    graphics_reset_dialog();
}

static int should_scroll_to_typed_text(void)
{
    if (string_equals(data.previously_seen_typed_name, data.typed_name)) {
        return 0;
    }
    int scroll = 0;
    // Only scroll when adding characters to the typed name
    if (string_length(data.typed_name) > string_length(data.previously_seen_typed_name)) {
        scroll = 1;
    }
    string_copy(data.typed_name, data.previously_seen_typed_name, FILE_NAME_MAX);
    return scroll;
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    data.double_click = m->left.double_click;

    if (input_box_is_accepted(&file_name_input)) {
        button_ok_cancel(1, 0);
        return;
    }
    
    if (data.message_not_exist_start_time &&
        time_get_millis() - data.message_not_exist_start_time >= NOT_EXIST_MESSAGE_TIMEOUT) {
        data.redraw_full_window = 1;
        data.message_not_exist_start_time = 0;
    }

    const mouse *m_dialog = mouse_in_dialog(m);
    int focus_id = data.focus_button_id;
    int scroll_position = scrollbar.scroll_position;
    data.focus_button_id = 0;
    if (scrollbar_handle_mouse(&scrollbar, m_dialog, 1) ||
        input_box_handle_mouse(m_dialog, &file_name_input) ||
        generic_buttons_handle_mouse(m_dialog, 0, 0, file_buttons, NUM_FILES_IN_VIEW, &data.focus_button_id) ||
        image_buttons_handle_mouse(m_dialog, 0, 0, image_buttons, 2, 0)) {
        data.redraw_full_window = 1;
        return;
    }
    if (input_go_back_requested(m, h)) {
        input_box_stop(&file_name_input);
        window_go_back();
        return;
    }

    if (should_scroll_to_typed_text()) {
        scroll_to_typed_text();
        data.redraw_full_window = 1;
    }
    if (focus_id != data.focus_button_id || scroll_position != scrollbar.scroll_position) {
        data.redraw_full_window = 1;
    }
}

static char *get_chosen_filename(void)
{
    // Check if we should work with the selected file
    uint8_t selected_name[FILE_NAME_MAX];
    encoding_from_utf8(data.selected_file, selected_name, FILE_NAME_MAX);

    if (string_equals(selected_name, data.typed_name)) {
        // User has not modified the string after selecting it: use filename
        return data.selected_file;
    }

    // We should use the typed name, which needs to be converted to UTF-8...
    static char typed_file[FILE_NAME_MAX];
    encoding_to_utf8(data.typed_name, typed_file, FILE_NAME_MAX, encoding_system_uses_decomposed());
    return typed_file;
}

static void button_ok_cancel(int is_ok, int param2)
{
    if (!is_ok) {
        input_box_stop(&file_name_input);
        window_go_back();
        return;
    }

    char *chosen_filename = get_chosen_filename();
    char filename[FILE_NAME_MAX];
    memset(filename, 0, sizeof(filename));
    if (data.type == FILE_TYPE_EMPIRE) {
        strncpy(filename, "custom_empires/", FILE_NAME_MAX - 1);
    }
    strncat(filename, chosen_filename, FILE_NAME_MAX - 1);

    if (data.dialog_type != FILE_DIALOG_SAVE && !file_exists(filename, NOT_LOCALIZED)) {
        data.message_not_exist_start_time = time_get_millis();
        return;
    }
    if (data.dialog_type == FILE_DIALOG_LOAD) {
        if (data.type == FILE_TYPE_SAVED_GAME) {
            int result = game_file_load_saved_game(filename);
            if (result == 1) {
                input_box_stop(&file_name_input);
                window_city_show();
            } else if (result == 0) {
                data.message_not_exist_start_time = time_get_millis();
                return;
            } else if (result == -1) {
                window_plain_message_dialog_show(TR_SAVEGAME_LARGER_VERSION_TITLE,
                    TR_SAVEGAME_LARGER_VERSION_MESSAGE, 1);
                return;
            }
        } else if (data.type == FILE_TYPE_SCENARIO) {
            if (game_file_editor_load_scenario(filename)) {
                input_box_stop(&file_name_input);
                window_editor_map_show();
            } else {
                data.message_not_exist_start_time = time_get_millis();
                return;
            }
        } else if (data.type == FILE_TYPE_EMPIRE) {
            int result = empire_xml_parse_file(filename);
            if (result) {
                scenario_editor_set_custom_empire(filename);
                window_editor_empire_show();
            } else {
                window_plain_message_dialog_show(TR_EDITOR_UNABLE_TO_LOAD_EMPIRE_TITLE, TR_EDITOR_UNABLE_TO_LOAD_EMPIRE_MESSAGE, 1);
                return;
            }
        }
    } else if (data.dialog_type == FILE_DIALOG_SAVE) {
        input_box_stop(&file_name_input);
        if (data.type == FILE_TYPE_SAVED_GAME) {
            if (!file_has_extension(filename, saved_game_data_expanded.extension)) {
                file_append_extension(filename, saved_game_data_expanded.extension);
            }
            if (!game_file_write_saved_game(filename)) {
                window_plain_message_dialog_show(TR_SAVEGAME_NOT_ABLE_TO_SAVE_TITLE,
                    TR_SAVEGAME_NOT_ABLE_TO_SAVE_MESSAGE, 1);
                return;
            }
            window_city_show();
        } else if (data.type == FILE_TYPE_SCENARIO) {
            if (!file_has_extension(filename, scenario_data_expanded.extension)) {
                file_append_extension(filename, scenario_data_expanded.extension);
            }
            game_file_editor_write_scenario(filename);
            window_editor_map_show();
        }
    } else if (data.dialog_type == FILE_DIALOG_DELETE) {
        if (game_file_delete_saved_game(filename)) {
            dir_find_files_with_extension(".", data.file_data->extension);
            dir_append_files_with_extension(saved_game_data_expanded.extension);

            if (scrollbar.scroll_position + NUM_FILES_IN_VIEW >= data.file_list->num_files) {
                --scrollbar.scroll_position;

            }
            if (scrollbar.scroll_position < 0) {
                scrollbar.scroll_position = 0;
            }
        }
    }

    strncpy(data.file_data->last_loaded_file, chosen_filename, FILE_NAME_MAX - 1);
}

static void on_scroll(void)
{
    data.message_not_exist_start_time = 0;
}

static void button_select_file(int index, int param2)
{
    if (index < data.file_list->num_files &&
        strcmp(data.selected_file, data.file_list->files[scrollbar.scroll_position + index]) != 0) {
        strncpy(data.selected_file, data.file_list->files[scrollbar.scroll_position + index], FILE_NAME_MAX - 1);
        encoding_from_utf8(data.selected_file, data.typed_name, FILE_NAME_MAX);
        string_copy(data.typed_name, data.previously_seen_typed_name, FILE_NAME_MAX);
        input_box_refresh_text(&file_name_input);
        data.message_not_exist_start_time = 0;
        window_request_refresh();
    }
    if (data.dialog_type != FILE_DIALOG_DELETE && data.double_click) {
        data.double_click = 0;
        button_ok_cancel(1, 0);
    }
}

void window_file_dialog_show(file_type type, file_dialog_type dialog_type)
{
    window_type window = {
        WINDOW_FILE_DIALOG,
        draw_background,
        draw_foreground,
        handle_input
    };
    init(type, dialog_type);
    window_show(&window);
}
