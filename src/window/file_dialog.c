#include "file_dialog.h"

#include "core/calc.h"
#include "core/config.h"
#include "core/dir.h"
#include "core/encoding.h"
#include "core/file.h"
#include "core/image_group.h"
#include "core/lang.h"
#include "core/string.h"
#include "core/time.h"
#include "editor/empire.h"
#include "empire/xml.h"
#include "game/file.h"
#include "game/file_editor.h"
#include "game/file_io.h"
#include "game/save_version.h"
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
#include "input/scroll.h"
#include "platform/file_manager.h"
#include "scenario/editor.h"
#include "scenario/custom_messages_export_xml.h"
#include "scenario/custom_messages_import_xml.h"
#include "scenario/scenario_events_export_xml.h"
#include "scenario/scenario_events_import_xml.h"
#include "translation/translation.h"
#include "widget/input_box.h"
#include "window/city.h"
#include "window/editor/custom_messages.h"
#include "window/editor/map.h"
#include "window/editor/scenario_events.h"
#include "widget/minimap.h"
#include "window/plain_message_dialog.h"
#include "window/popup_dialog.h"

#include <string.h>

#define NUM_FILES_IN_VIEW 21
#define MAX_FILE_WINDOW_TEXT_WIDTH (16 * BLOCK_SIZE)
#define FILTER_TEXT_SIZE 16
#define MIN_FILTER_SIZE 2

static const time_millis NOT_EXIST_MESSAGE_TIMEOUT = 500;

static void button_toggle_sort_type(int param1, int param2);
static void button_ok_cancel(int is_ok, int param2);
static void button_select_file(int index, int param2);
static void on_scroll(void);
static void input_box_changed(int is_addition_at_end);

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

static generic_button sort_by_button[] = {
    {16, 437, 288, 26, button_toggle_sort_type, button_none, 0, 0}
};

static scrollbar_type scrollbar = { 304, 80, 350, 256, NUM_FILES_IN_VIEW, on_scroll, 1 };

typedef struct {
    const char *extension;
    const char *path;
    char last_loaded_file[FILE_NAME_MAX];
} file_type_data;

static struct {
    time_millis message_not_exist_start_time;
    file_type type;
    file_dialog_type dialog_type;
    int focus_button_id;
    int double_click;
    const dir_listing *file_list;

    dir_listing filtered_file_list;
    uint8_t filter_text[FILTER_TEXT_SIZE];
    enum {
        SORT_BY_NAME,
        SORT_BY_DATE
    } sort_type;
    int sort_by_button_focused;

    file_type_data *file_data;
    char selected_file[FILE_NAME_MAX];
    uint8_t typed_name[FILE_NAME_MAX];

    union {
        saved_game_info save_game;
        scenario_info scenario;
    } info;
    savegame_load_status savegame_info_status;
    int redraw_full_window;
    int selected_index;
} data;

static input_box main_input = {
    .x = 16,
    .y = 48,
    .width_blocks = 18,
    .height_blocks = 2,
    .font = FONT_NORMAL_WHITE,
    .text = data.filter_text,
    .text_length = FILTER_TEXT_SIZE,
    .put_clear_button_outside_box = 1,
    .on_change = input_box_changed
};

static const int MISSION_ID_TO_CITY_ID[] = {
    0, 3, 2, 1, 7, 10, 18, 4, 30, 6, 12, 14, 16, 27, 31, 23, 36, 38, 28, 25
};

static file_type_data saved_game_data = { "sav", "." };
static file_type_data saved_game_data_expanded = { "svx", "." };
static file_type_data scenario_data = { "map", "." };
static file_type_data scenario_data_expanded = { "mapx", "." };
static file_type_data empire_data = { "xml", "custom_empires" };
static file_type_data scenario_event_data = { "xml", "editor/events" };
static file_type_data custom_messages_data = { "xml", "editor/messages" };

static int compare_name(const void *va, const void *vb)
{
    const dir_entry *a = (const dir_entry *) va;
    const dir_entry *b = (const dir_entry *) vb;

    return platform_file_manager_compare_filename(a->name, b->name);
}

static int compare_modified_time(const void *va, const void *vb)
{
    const dir_entry *a = (const dir_entry *) va;
    const dir_entry *b = (const dir_entry *) vb;

    if (a->modified_time == b->modified_time) {
        return platform_file_manager_compare_filename(a->name, b->name);
    }

    return b->modified_time - a->modified_time;
}

static void sort_file_list(void)
{
    qsort(data.filtered_file_list.files, data.filtered_file_list.num_files, sizeof(dir_entry),
        data.sort_type == SORT_BY_NAME ? compare_name : compare_modified_time);
}

static void init_filtered_file_list(void)
{
    if (data.filtered_file_list.num_files > 0) {
        free(data.filtered_file_list.files);
        data.filtered_file_list.num_files = 0;
    }
    data.filtered_file_list.files = malloc(sizeof(dir_entry) * data.file_list->num_files);
    memset(data.filtered_file_list.files, 0, sizeof(dir_entry) * data.file_list->num_files);

    const char *filter = 0;
    if (data.dialog_type != FILE_DIALOG_SAVE && *data.filter_text) {
        char filter_utf8[FILTER_TEXT_SIZE];
        encoding_to_utf8(data.filter_text, filter_utf8, FILTER_TEXT_SIZE, encoding_system_uses_decomposed());
        if (strlen(filter_utf8) >= MIN_FILTER_SIZE) {
            filter = filter_utf8;
        }
    }

    for (int i = 0; i < data.file_list->num_files; i++) {
        if (!filter || platform_file_manager_filename_contains(data.file_list->files[i].name, filter)) {
            data.filtered_file_list.files[data.filtered_file_list.num_files] = data.file_list->files[i];
            data.filtered_file_list.num_files++;
        }
    }
    sort_file_list();
}

static int select_correct_index(void)
{
    if (!*data.selected_file) {
        return 0;
    }
    for (int i = 0; i < data.filtered_file_list.num_files; i++) {
        if (strcmp(data.filtered_file_list.files[i].name, data.selected_file) == 0) {
            data.selected_index = i + 1;
            return i;
        }
    }
    data.selected_index = 0;
    return 0;
}

static void init(file_type type, file_dialog_type dialog_type)
{
    data.type = type;
    if (type == FILE_TYPE_SCENARIO) {
        data.file_data = &scenario_data_expanded;
    } else if (type == FILE_TYPE_EMPIRE) {
        data.file_data = &empire_data;
    } else if (type == FILE_TYPE_SCENARIO_EVENTS) {
        data.file_data = &scenario_event_data;
    } else if (type == FILE_TYPE_CUSTOM_MESSAGES) {
        data.file_data = &custom_messages_data;
    } else {
        data.file_data = &saved_game_data_expanded;
    }
    data.dialog_type = dialog_type;

    data.message_not_exist_start_time = 0;
    data.double_click = 0;
    data.focus_button_id = 0;

    if (strlen(data.file_data->last_loaded_file) > 0) {
        strncpy(data.selected_file, data.file_data->last_loaded_file, FILE_NAME_MAX);
        if (data.dialog_type == FILE_DIALOG_SAVE) {
            file_remove_extension(data.selected_file);
        }
        encoding_from_utf8(data.selected_file, data.typed_name, FILE_NAME_MAX);
        if (data.dialog_type == FILE_DIALOG_SAVE) {
            file_append_extension(data.selected_file, data.file_data->extension);
        }
    } else if (dialog_type == FILE_DIALOG_SAVE) {
        // Suggest default filename
        string_copy(lang_get_string(9, type == FILE_TYPE_SCENARIO ? 7 : 6), data.typed_name, FILE_NAME_MAX);
        encoding_to_utf8(data.typed_name, data.selected_file, FILE_NAME_MAX, encoding_system_uses_decomposed());
        file_append_extension(data.selected_file, data.file_data->extension);
    } else {
        // Use empty string
        data.typed_name[0] = 0;
        data.selected_file[0] = 0;
    }

    if (data.dialog_type != FILE_DIALOG_SAVE) {
        if (type == FILE_TYPE_SCENARIO) {
            data.file_list = dir_find_files_with_extension(scenario_data.path, scenario_data.extension);
            data.file_list = dir_append_files_with_extension(scenario_data_expanded.extension);
        } else if (type == FILE_TYPE_EMPIRE) {
            data.file_list = dir_find_files_with_extension(empire_data.path, empire_data.extension);
        } else if (type == FILE_TYPE_SCENARIO_EVENTS) {
            data.file_list = dir_find_files_with_extension(scenario_event_data.path, scenario_event_data.extension);
        } else if (type == FILE_TYPE_CUSTOM_MESSAGES) {
            data.file_list = dir_find_files_with_extension(custom_messages_data.path, custom_messages_data.extension);
        } else {
            data.file_list = dir_find_files_with_extension(saved_game_data.path, saved_game_data.extension);
            data.file_list = dir_append_files_with_extension(saved_game_data_expanded.extension);
        }
    } else {
        data.file_list = dir_find_files_with_extension(data.file_data->path, data.file_data->extension);
    }
    init_filtered_file_list();
    scrollbar_init(&scrollbar, select_correct_index(), data.filtered_file_list.num_files);

    if (data.dialog_type == FILE_DIALOG_SAVE) {
        main_input.placeholder = 0;
        main_input.text = data.typed_name;
        main_input.text_length = FILE_NAME_MAX;
        main_input.width_blocks = 38;
    } else {
        main_input.placeholder = lang_get_string(CUSTOM_TRANSLATION, TR_SAVE_DIALOG_FILTER);
        main_input.text = data.filter_text;
        main_input.text_length = FILTER_TEXT_SIZE;
        main_input.width_blocks = 18;
    }

    input_box_start(&main_input);
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
            data.savegame_info_status = game_file_io_read_saved_game_info(data.selected_file, &data.info.save_game);
        } else {
            data.savegame_info_status = game_file_io_read_scenario_info(data.selected_file, &data.info.scenario);
        }
    }
    data.redraw_full_window = 1;
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    if (data.redraw_full_window) {
        outer_panel_draw(0, 0, 40, 30);
        inner_panel_draw(16, 80, 18, 22);

        // title
        if (data.message_not_exist_start_time
            && time_get_millis() - data.message_not_exist_start_time < NOT_EXIST_MESSAGE_TIMEOUT) {
            lang_text_draw_centered(43, 2, 32, 14, 554, FONT_LARGE_BLACK);
        } else if (data.dialog_type == FILE_DIALOG_DELETE) {
            lang_text_draw_centered(43, 6, 32, 14, 554, FONT_LARGE_BLACK);
        } else if (data.type == FILE_TYPE_EMPIRE) {
            lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_CUSTOM_EMPIRE_TITLE, 32, 14, 554, FONT_LARGE_BLACK);
        } else if (data.type == FILE_TYPE_SCENARIO_EVENTS) {
            int message_id = TR_EDITOR_SCENARIO_EVENTS_IMPORT_FULL;
            if (data.dialog_type == FILE_DIALOG_SAVE) {
                message_id = TR_EDITOR_SCENARIO_EVENTS_EXPORT_FULL;
            }
            lang_text_draw_centered(CUSTOM_TRANSLATION, message_id, 32, 14, 554, FONT_LARGE_BLACK);
        } else if (data.type == FILE_TYPE_CUSTOM_MESSAGES) {
            int message_id = TR_EDITOR_CUSTOM_MESSAGES_IMPORT_FULL;
            if (data.dialog_type == FILE_DIALOG_SAVE) {
                message_id = TR_EDITOR_CUSTOM_MESSAGES_EXPORT_FULL;
            }
            lang_text_draw_centered(CUSTOM_TRANSLATION, message_id, 32, 14, 554, FONT_LARGE_BLACK);
        } else {
            int text_id = data.dialog_type + (data.type == FILE_TYPE_SCENARIO ? 3 : 0);
            lang_text_draw_centered(43, text_id, 32, 14, 554, FONT_LARGE_BLACK);
        }
        // Proceed? text
        lang_text_draw_right_aligned(43, 5, 362, 447, 164, FONT_NORMAL_BLACK);

        // Sorting text
        int sort_translation = TR_SAVE_DIALOG_SORTING_BY_NAME + data.sort_type;
        int sort_button_text_y = sort_by_button[0].y + sort_by_button[0].height / 2 - 5;
        lang_text_draw_centered(
            CUSTOM_TRANSLATION,
            sort_translation,
            sort_by_button[0].x,
            sort_button_text_y,
            sort_by_button[0].width,
            FONT_NORMAL_BLACK
        );

        // Saved game info
        if (*data.selected_file && data.type != FILE_TYPE_EMPIRE && data.type != FILE_TYPE_SCENARIO_EVENTS
            && data.type != FILE_TYPE_CUSTOM_MESSAGES) {
            if (data.savegame_info_status == SAVEGAME_STATUS_OK) {
                if (data.dialog_type != FILE_DIALOG_SAVE) {
                    if (text_get_width(data.typed_name, FONT_NORMAL_BLACK) > 246) {
                        text_draw_ellipsized(data.typed_name, 362, 55, 246, FONT_NORMAL_BLACK, 0);
                    } else {
                        text_draw_centered(data.typed_name, 362, 55, 246, FONT_NORMAL_BLACK, 0);
                    }
                }
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
                if (data.dialog_type == FILE_DIALOG_SAVE) {
                    text_draw_centered(translation_for(TR_SAVE_DIALOG_NEW_FILE), 362, 246, 246, FONT_NORMAL_BLACK, 0);
                } else {
                    translation_key key = data.savegame_info_status == SAVEGAME_STATUS_INVALID ?
                        TR_SAVE_DIALOG_INVALID_FILE : TR_SAVE_DIALOG_INCOMPATIBLE_VERSION;
                    text_draw_centered(translation_for(key), 362, 241, 246, FONT_LARGE_BLACK, 0);
                }
            }
        } else {
            text_draw_centered(translation_for(TR_SAVE_DIALOG_SELECT_FILE), 362, 246, 246, FONT_NORMAL_BLACK, 0);
        }
        data.redraw_full_window = 0;
    }

    // File list
    uint8_t file[FILE_NAME_MAX];
    for (int i = 0; i < NUM_FILES_IN_VIEW && scrollbar.scroll_position + i < data.filtered_file_list.num_files; i++) {
        font_t font = FONT_NORMAL_GREEN;
        if (data.focus_button_id == i + 1) {
            button_border_draw(24, 85 + 16 * i, MAX_FILE_WINDOW_TEXT_WIDTH + 8, 20, 1);
        }
        encoding_from_utf8(data.filtered_file_list.files[scrollbar.scroll_position + i].name, file, FILE_NAME_MAX);
        if (data.selected_index == scrollbar.scroll_position + i + 1) {
            font = FONT_NORMAL_WHITE;
        }
        text_ellipsize(file, font, MAX_FILE_WINDOW_TEXT_WIDTH);
        text_draw(file, 32, 90 + 16 * i, font, 0);
    }

    input_box_draw(&main_input);
    button_border_draw(
        sort_by_button[0].x,
        sort_by_button[0].y,
        sort_by_button[0].width,
        sort_by_button[0].height,
        data.sort_by_button_focused
    );
    image_buttons_draw(0, 0, image_buttons, 2);
    scrollbar_draw(&scrollbar);
    graphics_reset_dialog();
}

static int handle_arrow_keys(int direction)
{
    int delta;
    switch (direction) {
        case DIR_0_TOP:
        case DIR_1_TOP_RIGHT:
        case DIR_7_TOP_LEFT:
            delta = -1;
            break;
        case DIR_4_BOTTOM:
        case DIR_3_BOTTOM_RIGHT:
        case DIR_5_BOTTOM_LEFT:
            delta = 1;
            break;
        default:
            return 0;
    }
    int new_index = 0;
    if (data.selected_index == 0) {
        if (delta == 1) {
            new_index = 1;
        } else {
            new_index = data.filtered_file_list.num_files;
        }
    } else {
        new_index = data.selected_index + delta;
        if (new_index == 0) {
            new_index = data.filtered_file_list.num_files;
        } else if (new_index > data.filtered_file_list.num_files) {
            new_index = 1;
        }
    }
    button_select_file(new_index - scrollbar.scroll_position - 1, 0);
    if (data.selected_index > scrollbar.scroll_position + NUM_FILES_IN_VIEW) {
        scrollbar_reset(&scrollbar, data.selected_index - NUM_FILES_IN_VIEW);
    } else if (data.selected_index <= scrollbar.scroll_position) {
        scrollbar_reset(&scrollbar, data.selected_index - 1);
    }
    return 1;
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    data.double_click = m->left.double_click;

    if (input_box_is_accepted()) {
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
        handle_arrow_keys(scroll_for_menu(m)) ||
        input_box_handle_mouse(m_dialog, &main_input) ||
        generic_buttons_handle_mouse(m_dialog, 0, 0, sort_by_button, 1, &data.sort_by_button_focused) ||
        generic_buttons_handle_mouse(m_dialog, 0, 0, file_buttons, NUM_FILES_IN_VIEW, &data.focus_button_id) ||
        image_buttons_handle_mouse(m_dialog, 0, 0, image_buttons, 2, 0)) {
        data.redraw_full_window = 1;
        return;
    }
    if (input_go_back_requested(m, h)) {
        input_box_stop(&main_input);
        window_go_back();
        return;
    }

    if (focus_id != data.focus_button_id || scroll_position != scrollbar.scroll_position) {
        data.redraw_full_window = 1;
    }
}

static int further_filter_file_list(void)
{
    if (data.dialog_type == FILE_DIALOG_SAVE || !*data.filter_text) {
        return 0;
    }

    char filter[FILTER_TEXT_SIZE];
    encoding_to_utf8(data.filter_text, filter, FILTER_TEXT_SIZE, encoding_system_uses_decomposed());
    if (strlen(filter) < MIN_FILTER_SIZE) {
        return 0;
    }
    int current_index = 0;
    for (int i = 0; i < data.filtered_file_list.num_files; i++) {
        if (platform_file_manager_filename_contains(data.filtered_file_list.files[i].name, filter)) {
            if (i > current_index) {
                data.filtered_file_list.files[current_index] = data.filtered_file_list.files[i];
            }
            current_index++;
        }
    }
    int result = data.filtered_file_list.num_files > current_index;
    data.filtered_file_list.num_files = current_index;
    return result;
}

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
        if (platform_file_manager_compare_filename_prefix(data.file_list->files[middle].name, prefix, len) >= 0) {
            right = middle;
        } else {
            left = middle + 1;
        }
    }
    if (left < data.file_list->num_files &&
        platform_file_manager_compare_filename_prefix(data.file_list->files[left].name, prefix, len) == 0) {
        return left;
    } else {
        return -1;
    }
}

static void input_box_changed(int is_addition_at_end)
{
    int scroll_index = 0;
    if (data.dialog_type == FILE_DIALOG_SAVE) {
        encoding_to_utf8(data.typed_name, data.selected_file, FILE_NAME_MAX, encoding_system_uses_decomposed());
        if (data.file_list->num_files > NUM_FILES_IN_VIEW) {
            scroll_index = find_first_file_with_prefix(data.selected_file);
        }
        file_append_extension(data.selected_file, data.file_data->extension);
        if (scroll_index >= 0 && strcmp(data.selected_file, data.filtered_file_list.files[scroll_index].name) == 0) {
            data.selected_index = scroll_index + 1;
        } else {
            data.selected_index = 0;
        }
        window_request_refresh();
    } else {
        if (is_addition_at_end) {
            if (!further_filter_file_list()) {
                return;
            }
        } else {
            init_filtered_file_list();
        }
        scroll_index = select_correct_index();
        data.redraw_full_window = 1;
    }
    scrollbar_init(&scrollbar, scroll_index, data.filtered_file_list.num_files);
}

static const char *prepare_filename(void)
{
    static char filename[FILE_NAME_MAX];
    memset(filename, 0, sizeof(filename));
    if (data.type == FILE_TYPE_EMPIRE) {
        strncpy(filename, "custom_empires/", FILE_NAME_MAX - 1);
    } else if (data.type == FILE_TYPE_SCENARIO_EVENTS) {
        strncpy(filename, "editor/events/", FILE_NAME_MAX - 1);
    } else if (data.type == FILE_TYPE_CUSTOM_MESSAGES) {
        strncpy(filename, "editor/messages/", FILE_NAME_MAX - 1);
    }
    strncat(filename, data.selected_file, sizeof(filename) - strlen(filename));
    return filename;
}

static void confirm_save_file(int accepted, int checked)
{
    if (!accepted) {
        return;
    }
    const char *filename = prepare_filename();
    input_box_stop(&main_input);
    if (checked) {
        config_set(CONFIG_UI_ASK_CONFIRMATION_ON_FILE_OVERWRITE, 0);
    }
    if (data.type == FILE_TYPE_SAVED_GAME) {
        if (!game_file_write_saved_game(filename)) {
            window_plain_message_dialog_show(TR_SAVEGAME_NOT_ABLE_TO_SAVE_TITLE,
                TR_SAVEGAME_NOT_ABLE_TO_SAVE_MESSAGE, 1);
            return;
        }
        window_city_show();
    } else if (data.type == FILE_TYPE_SCENARIO) {
        game_file_editor_write_scenario(filename);
        window_editor_map_show();
    } else if (data.type == FILE_TYPE_SCENARIO_EVENTS) {
        scenario_events_export_to_xml(filename);
        window_editor_scenario_events_show(1);
    } else if (data.type == FILE_TYPE_CUSTOM_MESSAGES) {
        custom_messages_export_to_xml(filename);
        window_editor_custom_messages_show();
    }
}

static void button_ok_cancel(int is_ok, int param2)
{
    if (!is_ok) {
        input_box_stop(&main_input);
        window_go_back();
        return;
    }

    const char *filename = prepare_filename();

    if (data.dialog_type != FILE_DIALOG_SAVE && !file_exists(filename, NOT_LOCALIZED)) {
        data.message_not_exist_start_time = time_get_millis();
        return;
    }

    if (data.dialog_type == FILE_DIALOG_LOAD) {
        if (data.type == FILE_TYPE_SAVED_GAME) {
            int result = game_file_load_saved_game(filename);
            if (result == FILE_LOAD_SUCCESS) {
                input_box_stop(&main_input);
                window_city_show();
            } else if (result == FILE_LOAD_DOES_NOT_EXIST) {
                data.message_not_exist_start_time = time_get_millis();
                return;
            } else if (result == FILE_LOAD_INCOMPATIBLE_VERSION) {
                window_plain_message_dialog_show(TR_SAVEGAME_LARGER_VERSION_TITLE,
                    TR_SAVEGAME_LARGER_VERSION_MESSAGE, 1);
                return;
            } else if (result == FILE_LOAD_WRONG_FILE_FORMAT) {
                window_plain_message_dialog_show(TR_SAVE_DIALOG_INVALID_FILE,
                    TR_SAVE_DIALOG_INVALID_FILE_DESC, 1);
                return;
            }
        } else if (data.type == FILE_TYPE_SCENARIO) {
            if (game_file_editor_load_scenario(filename)) {
                input_box_stop(&main_input);
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
        } else if (data.type == FILE_TYPE_SCENARIO_EVENTS) {
            int result = scenario_events_xml_parse_file(filename);
            if (result) {
                window_editor_scenario_events_show(1);
            } else {
                window_plain_message_dialog_show(TR_EDITOR_UNABLE_TO_LOAD_EVENTS_TITLE, TR_EDITOR_CHECK_LOG_MESSAGE, 1);
                return;
            }
        } else if (data.type == FILE_TYPE_CUSTOM_MESSAGES) {
            int result = custom_messages_xml_parse_file(filename);
            if (result) {
                window_editor_custom_messages_show();
            } else {
                window_plain_message_dialog_show(TR_EDITOR_UNABLE_TO_LOAD_CUSTOM_MESSAGES_TITLE, TR_EDITOR_CHECK_LOG_MESSAGE, 1);
                return;
            }
        }
        strncpy(data.file_data->last_loaded_file, data.selected_file, FILE_NAME_MAX);
    } else if (data.dialog_type == FILE_DIALOG_SAVE) {
        if (config_get(CONFIG_UI_ASK_CONFIRMATION_ON_FILE_OVERWRITE) && file_exists(filename, NOT_LOCALIZED)) {
            window_popup_dialog_show_confirmation(lang_get_string(CUSTOM_TRANSLATION, TR_SAVE_DIALOG_OVERWRITE_FILE),
                lang_get_string(CUSTOM_TRANSLATION, TR_SAVE_DIALOG_OVERWRITE_FILE_DESC),
                lang_get_string(CUSTOM_TRANSLATION, TR_SAVE_DIALOG_OVERWRITE_FILE_DO_NOT_ASK_AGAIN), confirm_save_file);
        } else {
            confirm_save_file(1, 0);
        }
    } else if (data.dialog_type == FILE_DIALOG_DELETE) {
        if (game_file_delete_saved_game(filename)) {
            dir_find_files_with_extension(saved_game_data.path, saved_game_data.extension);
            dir_append_files_with_extension(saved_game_data_expanded.extension);

            init_filtered_file_list();

            if (data.selected_index > 0) {
                if (data.selected_index > data.filtered_file_list.num_files) {
                    data.selected_index = data.filtered_file_list.num_files;
                }
                button_select_file(data.selected_index - scrollbar.scroll_position - 1, 0);
            }
            scrollbar_init(&scrollbar, select_correct_index(), data.filtered_file_list.num_files);
            strncpy(data.file_data->last_loaded_file, data.selected_file, FILE_NAME_MAX);
            window_request_refresh();
        }
    }
}

static void on_scroll(void)
{
    data.message_not_exist_start_time = 0;
}

static void button_toggle_sort_type(int param1, int param2)
{
    if (data.sort_type == SORT_BY_NAME) {
        data.sort_type = SORT_BY_DATE;
    } else {
        data.sort_type = SORT_BY_NAME;
    }
    sort_file_list();
    scrollbar_reset(&scrollbar, select_correct_index());
    data.redraw_full_window = 1;
}

static void button_select_file(int index, int param2)
{
    if (index + scrollbar.scroll_position >= data.filtered_file_list.num_files) {
        if (data.double_click) {
            data.double_click = 0;
        }
        return;
    }
    if (strcmp(data.selected_file, data.filtered_file_list.files[scrollbar.scroll_position + index].name) != 0) {
        data.selected_index = scrollbar.scroll_position + index + 1;
        data.message_not_exist_start_time = 0;
        strncpy(data.selected_file, data.filtered_file_list.files[scrollbar.scroll_position + index].name, FILE_NAME_MAX - 1);
        if (data.dialog_type == FILE_DIALOG_SAVE) {
            file_remove_extension(data.selected_file);
        }
        encoding_from_utf8(data.selected_file, data.typed_name, FILE_NAME_MAX);
        if (data.dialog_type == FILE_DIALOG_SAVE) {
            input_box_refresh_text(&main_input);
            file_append_extension(data.selected_file, data.file_data->extension);
        }
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
