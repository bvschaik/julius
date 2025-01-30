#include "file_dialog.h"

#include "core/calc.h"
#include "core/config.h"
#include "core/dir.h"
#include "core/encoding.h"
#include "core/file.h"
#include "core/image_group.h"
#include "core/lang.h"
#include "core/log.h"
#include "core/string.h"
#include "editor/empire.h"
#include "empire/xml.h"
#include "game/file.h"
#include "game/file_editor.h"
#include "game/file_io.h"
#include "game/save_version.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/list_box.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "platform/file_manager.h"
#include "scenario/editor.h"
#include "scenario/custom_messages_export_xml.h"
#include "scenario/custom_messages_import_xml.h"
#include "scenario/event/export_xml.h"
#include "scenario/event/import_xml.h"
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

static void button_toggle_sort_type(const generic_button *button);
static void button_ok_cancel(int is_ok, int param2);
static void input_box_changed(int is_addition_at_end);
static void draw_file(const list_box_item *item);
static void select_file(unsigned int index, int is_double_click);
static void file_tooltip(const list_box_item *item, tooltip_context *c);  

static image_button image_buttons[] = {
    {536, 440, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 0, button_ok_cancel, button_none, 1, 0, 1},
    {584, 440, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 4, button_ok_cancel, button_none, 0, 0, 1},
};

static generic_button sort_by_button[] = {
    {16, 437, 288, 26, button_toggle_sort_type}
};

typedef struct {
    const char *extension;
    int location;
    char last_loaded_file[FILE_NAME_MAX];
} file_type_data;

static struct {
    file_type type;
    file_dialog_type dialog_type;

    const dir_listing *file_list;
    dir_listing filtered_file_list;
    uint8_t filter_text[FILTER_TEXT_SIZE];
    enum {
        SORT_BY_NAME,
        SORT_BY_DATE
    } sort_type;
    unsigned int sort_by_button_focused;

    file_type_data *file_data;
    char selected_file[FILE_NAME_MAX];
    uint8_t typed_name[FILE_NAME_MAX];
    saved_game_info info;
    savegame_load_status savegame_info_status;
    int redraw_full_window;
} data;

static input_box main_input = {
    .x = 16,
    .y = 48,
    .width_blocks = 18,
    .height_blocks = 2,
    .font = FONT_NORMAL_WHITE,
    .text = data.filter_text,
    .text_length = FILTER_TEXT_SIZE,
    .on_change = input_box_changed
};

static list_box_type list_box = {
    .x = 16,
    .y = 80,
    .width_blocks = 20,
    .height_blocks = 22,
    .item_height = 16,
    .decorate_scrollbar = 0,
    .draw_inner_panel = 1,
    .extend_to_hidden_scrollbar = 0,
    .draw_item = draw_file,
    .on_select = select_file,
    .handle_tooltip = file_tooltip,
};

static const int MISSION_ID_TO_CITY_ID[] = {
    0, 3, 2, 1, 7, 10, 18, 4, 30, 6, 12, 14, 16, 27, 31, 23, 36, 38, 28, 25
};

static file_type_data saved_game_data = { "sav", PATH_LOCATION_SAVEGAME };
static file_type_data saved_game_data_expanded = { "svx", PATH_LOCATION_SAVEGAME };
static file_type_data scenario_data = { "map", PATH_LOCATION_SCENARIO };
static file_type_data scenario_data_expanded = { "mapx", PATH_LOCATION_SCENARIO};
static file_type_data empire_data = { "xml", PATH_LOCATION_EDITOR_CUSTOM_EMPIRES };
static file_type_data scenario_event_data = { "xml", PATH_LOCATION_EDITOR_CUSTOM_EVENTS };
static file_type_data custom_messages_data = { "xml", PATH_LOCATION_EDITOR_CUSTOM_MESSAGES };

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
        data.filtered_file_list.files = 0;
        data.filtered_file_list.num_files = 0;
    }
    data.filtered_file_list.files = malloc(sizeof(dir_entry) * data.file_list->num_files);
    if (!data.filtered_file_list.files) {
        return;
    }
    memset(data.filtered_file_list.files, 0, sizeof(dir_entry) * data.file_list->num_files);

    const char *filter = 0;
    char filter_utf8[FILTER_TEXT_SIZE];
    if (data.dialog_type != FILE_DIALOG_SAVE && *data.filter_text) {
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
            list_box_select_index(&list_box, i);
            return i;
        }
    }
    list_box_select_index(&list_box, LIST_BOX_NO_SELECTION);
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

    if (strlen(data.file_data->last_loaded_file) > 0) {
        snprintf(data.selected_file, FILE_NAME_MAX, "%s", data.file_data->last_loaded_file);
        if (data.dialog_type == FILE_DIALOG_SAVE) {
            file_remove_extension(data.selected_file);
        }
        encoding_from_utf8(data.selected_file, data.typed_name, FILE_NAME_MAX);
        if (data.dialog_type == FILE_DIALOG_SAVE) {
            file_append_extension(data.selected_file, data.file_data->extension, FILE_NAME_MAX);
        }
    } else if (dialog_type == FILE_DIALOG_SAVE) {
        // Suggest default filename
        string_copy(lang_get_string(9, type == FILE_TYPE_SCENARIO ? 7 : 6), data.typed_name, FILE_NAME_MAX);
        encoding_to_utf8(data.typed_name, data.selected_file, FILE_NAME_MAX, encoding_system_uses_decomposed());
        file_append_extension(data.selected_file, data.file_data->extension, FILE_NAME_MAX);
    } else {
        // Use empty string
        data.typed_name[0] = 0;
        data.selected_file[0] = 0;
    }

    if (data.dialog_type != FILE_DIALOG_SAVE) {
        if (type == FILE_TYPE_SCENARIO) {
            data.file_list = dir_find_files_with_extension_at_location(scenario_data.location, scenario_data.extension);
            data.file_list = dir_append_files_with_extension(scenario_data_expanded.extension);
        } else if (type == FILE_TYPE_EMPIRE) {
            data.file_list = dir_find_files_with_extension_at_location(empire_data.location, empire_data.extension);
        } else if (type == FILE_TYPE_SCENARIO_EVENTS) {
            data.file_list = dir_find_files_with_extension_at_location(scenario_event_data.location, scenario_event_data.extension);
        } else if (type == FILE_TYPE_CUSTOM_MESSAGES) {
            data.file_list = dir_find_files_with_extension_at_location(custom_messages_data.location, custom_messages_data.extension);
        } else {
            data.file_list = dir_find_files_with_extension_at_location(saved_game_data.location, saved_game_data.extension);
            data.file_list = dir_append_files_with_extension(saved_game_data_expanded.extension);
        }
    } else {
        data.file_list = dir_find_files_with_extension_at_location(data.file_data->location, data.file_data->extension);
    }
    init_filtered_file_list();
    list_box_init(&list_box, data.filtered_file_list.num_files);

    if (data.dialog_type == FILE_DIALOG_SAVE) {
        main_input.placeholder = 0;
        main_input.text = data.typed_name;
        main_input.text_length = FILE_NAME_MAX;
        main_input.width_blocks = 38;
        main_input.put_clear_button_outside_box = 0;
    } else {
        main_input.placeholder = lang_get_string(CUSTOM_TRANSLATION, TR_SAVE_DIALOG_FILTER);
        main_input.text = data.filter_text;
        main_input.text_length = FILTER_TEXT_SIZE;
        main_input.width_blocks = 18;
        main_input.put_clear_button_outside_box = 1;
    }

    input_box_start(&main_input);
}

static void draw_mission_info(int x_offset, int y_offset, int box_size)
{
    uint8_t text[FILE_NAME_MAX];
    uint8_t *cursor = text;

    if (data.info.origin.type == SAVEGAME_FROM_CUSTOM_SCENARIO) {
        cursor = string_copy(translation_for(TR_SAVE_DIALOG_CUSTOM_SCENARIO), cursor, FILE_NAME_MAX);
        cursor = string_copy(string_from_ascii(" - "), cursor, FILE_NAME_MAX - (int) (cursor - text));
        encoding_from_utf8(data.info.origin.scenario_name, cursor, FILE_NAME_MAX - (int) (cursor - text));
    } else if (data.info.origin.type == SAVEGAME_FROM_ORIGINAL_CAMPAIGN) {
        if (data.info.origin.mission == 0) {
            text_draw_centered(translation_for(TR_SAVE_DIALOG_FIRST_MISSION),
                x_offset, y_offset, box_size, FONT_NORMAL_BLACK, 0);
            return;
        } else {
            translation_key mission_type;
            if (data.info.origin.mission == 1) {
                mission_type = TR_SAVE_DIALOG_MISSION;
            } else if (data.info.origin.mission % 2) {
                mission_type = TR_SAVE_DIALOG_MILITARY;
            } else {
                mission_type = TR_SAVE_DIALOG_PEACEFUL;
            }
            cursor = string_copy(translation_for(mission_type), cursor, FILE_NAME_MAX);
            cursor = string_copy(string_from_ascii(" "), cursor, FILE_NAME_MAX - (int) (cursor - text));            
            cursor += string_from_int(cursor, (data.info.origin.mission + 4) / 2, 0);
            cursor = string_copy(string_from_ascii(" - "), cursor, FILE_NAME_MAX - (int) (cursor - text));
            string_copy(lang_get_string(21, MISSION_ID_TO_CITY_ID[data.info.origin.mission]), cursor,
                FILE_NAME_MAX - (int) (cursor - text));
        }
    } else {
        encoding_from_utf8(data.info.origin.campaign_name, cursor, FILE_NAME_MAX);
        cursor += string_length(text);
        cursor = string_copy(string_from_ascii(" - "), cursor, FILE_NAME_MAX - (int) (cursor - text));
        cursor += string_from_int(cursor, data.info.origin.mission + 1, 0);
        cursor = string_copy(string_from_ascii(" - "), cursor, FILE_NAME_MAX - (int) (cursor - text));
        encoding_from_utf8(data.info.origin.scenario_name, cursor, FILE_NAME_MAX - (int) (cursor - text));
    }

    text_draw_ellipsized(text, x_offset, y_offset, box_size, FONT_NORMAL_BLACK, 0);
}

static void draw_background(void)
{
    window_draw_underlying_window();
    if (*data.selected_file) {
        const char *filename = dir_get_file_at_location(data.selected_file, data.file_data->location);
        if (filename) {
            if (data.type == FILE_TYPE_SAVED_GAME) {
                data.savegame_info_status = game_file_io_read_saved_game_info(filename, 0, &data.info);
            } else {
                data.savegame_info_status = game_file_io_read_scenario_info(filename, &data.info);
            }
        } else {
            data.savegame_info_status = SAVEGAME_STATUS_INVALID;
        }
    }
    data.redraw_full_window = 1;
}

static void draw_file(const list_box_item *item)
{
    uint8_t file[FILE_NAME_MAX];
    font_t font = item->is_selected ? FONT_NORMAL_WHITE : FONT_NORMAL_GREEN;
    encoding_from_utf8(data.filtered_file_list.files[item->index].name, file, FILE_NAME_MAX);
    text_ellipsize(file, font, item->width);
    text_draw(file, item->x, item->y + 2, font, 0);
    if (item->is_focused) {
        button_border_draw(item->x - 4, item->y - 4, item->width + 6, item->height + 4, 1);
    }
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    if (data.redraw_full_window) {
        outer_panel_draw(0, 0, 40, 30);
        list_box_request_refresh(&list_box);

        // title
        if (data.dialog_type == FILE_DIALOG_DELETE) {
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
                    text_draw_money(data.info.treasury, 494, 376, FONT_NORMAL_BLACK);
                    text_draw(translation_for(TR_SAVE_DIALOG_DATE), 362, 396, FONT_NORMAL_BLACK, 0);
                    lang_text_draw_month_year_max_width(data.info.month, data.info.year,
                        500, 396, 108, FONT_NORMAL_BLACK, 0);
                    text_draw(translation_for(TR_SAVE_DIALOG_POPULATION), 362, 416, FONT_NORMAL_BLACK, 0);
                    text_draw_number(data.info.population, '\0', "",
                        500, 416, FONT_NORMAL_BLACK, COLOR_MASK_NONE);
                    widget_minimap_draw(352, 80, 266, 272);
                } else {
                    widget_minimap_draw(352, 80, 266, 352);
                }
            } else {
                if (data.dialog_type == FILE_DIALOG_SAVE) {
                    if (*data.typed_name) {
                        text_draw_centered(translation_for(TR_SAVE_DIALOG_NEW_FILE),
                            362, 246, 246, FONT_NORMAL_BLACK, 0);
                    } else {
                        text_draw_centered(translation_for(TR_SAVE_DIALOG_SELECT_FILE),
                            362, 246, 246, FONT_NORMAL_BLACK, 0);
                    }
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
    list_box_draw(&list_box);
    input_box_draw(&main_input);
    button_border_draw(
        sort_by_button[0].x,
        sort_by_button[0].y,
        sort_by_button[0].width,
        sort_by_button[0].height,
        data.sort_by_button_focused
    );
    image_buttons_draw(0, 0, image_buttons, 2);
    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (input_box_is_accepted()) {
        button_ok_cancel(1, 0);
        return;
    }

    const mouse *m_dialog = mouse_in_dialog(m);

    if (input_box_handle_mouse(m_dialog, &main_input) ||
        list_box_handle_input(&list_box, m_dialog, 1) ||
        generic_buttons_handle_mouse(m_dialog, 0, 0, sort_by_button, 1, &data.sort_by_button_focused) ||
        image_buttons_handle_mouse(m_dialog, 0, 0, image_buttons, 2, 0)) {
        data.redraw_full_window = 1;
        return;
    }
    if (input_go_back_requested(m, h)) {
        input_box_stop(&main_input);
        window_go_back();
        return;
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
    list_box_update_total_items(&list_box, data.filtered_file_list.num_files);
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
        file_append_extension(data.selected_file, data.file_data->extension, FILE_NAME_MAX);
        if (scroll_index >= 0 && data.filtered_file_list.num_files > scroll_index &&
            platform_file_manager_compare_filename(data.selected_file,
                data.filtered_file_list.files[scroll_index].name) == 0) {
            list_box_select_index(&list_box, scroll_index);
        } else {
            list_box_select_index(&list_box, LIST_BOX_NO_SELECTION);
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
    list_box_update_total_items(&list_box, data.filtered_file_list.num_files);
}

static void confirm_save_file(int accepted, int checked)
{
    if (!accepted) {
        return;
    }
    const char *filename = dir_append_location(data.selected_file, data.file_data->location);
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
        window_editor_scenario_events_show();
    } else if (data.type == FILE_TYPE_CUSTOM_MESSAGES) {
        custom_messages_export_to_xml(filename);
        window_editor_custom_messages_show();
    }
    snprintf(data.file_data->last_loaded_file, FILE_NAME_MAX, "%s", data.selected_file);
}

static void button_ok_cancel(int is_ok, int param2)
{
    if (!is_ok) {
        input_box_stop(&main_input);
        window_go_back();
        return;
    }

    if (!*data.typed_name) {
        return;
    }

    const char *filename;

    if (data.dialog_type == FILE_DIALOG_SAVE) {
        filename = dir_append_location(data.selected_file, data.file_data->location);
    } else {
        filename = dir_get_file_at_location(data.selected_file, data.file_data->location);
        if (!filename) {
            window_plain_message_dialog_show(TR_SAVE_DIALOG_FILE_DOES_NOT_EXIST_TITLE,
                TR_SAVE_DIALOG_FILE_DOES_NOT_EXIST_TEXT, 1);
            return;
        }
    }

    if (data.dialog_type == FILE_DIALOG_LOAD) {
        if (data.type == FILE_TYPE_SAVED_GAME) {
            int result = game_file_load_saved_game(filename);
            if (result == FILE_LOAD_SUCCESS) {
                window_city_show();
            } else if (result == FILE_LOAD_DOES_NOT_EXIST) {
                window_plain_message_dialog_show(TR_SAVE_DIALOG_FILE_DOES_NOT_EXIST_TITLE,
                    TR_SAVE_DIALOG_FILE_DOES_NOT_EXIST_TEXT, 1);
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
                window_editor_map_show();
            } else {
                window_plain_message_dialog_show(TR_SAVE_DIALOG_FILE_DOES_NOT_EXIST_TITLE,
                    TR_SAVE_DIALOG_FILE_DOES_NOT_EXIST_TEXT, 1);
                return;
            }
        } else if (data.type == FILE_TYPE_EMPIRE) {
            int result = empire_xml_parse_file(filename);
            if (result) {
                scenario_editor_set_custom_empire(data.selected_file);
                window_editor_empire_show();
            } else {
                window_plain_message_dialog_show(TR_EDITOR_UNABLE_TO_LOAD_EMPIRE_TITLE,
                    TR_EDITOR_UNABLE_TO_LOAD_EMPIRE_MESSAGE, 1);
                return;
            }
        } else if (data.type == FILE_TYPE_SCENARIO_EVENTS) {
            int result = scenario_events_xml_parse_file(filename);
            if (result) {
                window_editor_scenario_events_show();
            } else {
                window_plain_message_dialog_show(TR_EDITOR_UNABLE_TO_LOAD_EVENTS_TITLE, TR_EDITOR_CHECK_LOG_MESSAGE, 1);
                return;
            }
        } else if (data.type == FILE_TYPE_CUSTOM_MESSAGES) {
            int result = custom_messages_xml_parse_file(filename);
            if (result) {
                window_editor_custom_messages_show();
            } else {
                window_plain_message_dialog_show(TR_EDITOR_UNABLE_TO_LOAD_CUSTOM_MESSAGES_TITLE,
                    TR_EDITOR_CHECK_LOG_MESSAGE, 1);
                return;
            }
        }
        input_box_stop(&main_input);
        snprintf(data.file_data->last_loaded_file, FILE_NAME_MAX, "%s", data.selected_file);
    } else if (data.dialog_type == FILE_DIALOG_SAVE) {
        if (config_get(CONFIG_UI_ASK_CONFIRMATION_ON_FILE_OVERWRITE) &&
            dir_get_file_at_location(data.selected_file, data.file_data->location)) {
            window_popup_dialog_show_confirmation(lang_get_string(CUSTOM_TRANSLATION, TR_SAVE_DIALOG_OVERWRITE_FILE),
                lang_get_string(CUSTOM_TRANSLATION, TR_SAVE_DIALOG_OVERWRITE_FILE_DESC),
                lang_get_string(CUSTOM_TRANSLATION, TR_SAVE_DIALOG_OVERWRITE_FILE_DO_NOT_ASK_AGAIN), confirm_save_file);
        } else {
            confirm_save_file(1, 0);
        }
    } else if (data.dialog_type == FILE_DIALOG_DELETE) {
        if (game_file_delete_saved_game(filename)) {
            data.file_list = dir_find_files_with_extension_at_location(saved_game_data.location, saved_game_data.extension);
            dir_append_files_with_extension(saved_game_data_expanded.extension);

            init_filtered_file_list();
            list_box_update_total_items(&list_box, data.filtered_file_list.num_files);
            select_correct_index();
            snprintf(data.file_data->last_loaded_file, FILE_NAME_MAX, "%s", data.selected_file);
            window_request_refresh();
        }
    }
}

static void button_toggle_sort_type(const generic_button *button)
{
    if (data.sort_type == SORT_BY_NAME) {
        data.sort_type = SORT_BY_DATE;
    } else {
        data.sort_type = SORT_BY_NAME;
    }
    sort_file_list();
    select_correct_index();
    list_box_show_selected_index(&list_box);
    data.redraw_full_window = 1;
}

static void select_file(unsigned int index, int is_double_click)
{
    if (index == LIST_BOX_NO_SELECTION) {
        return;
    }
    if (strcmp(data.selected_file, data.filtered_file_list.files[index].name) != 0) {
        snprintf(data.selected_file, FILE_NAME_MAX, "%s", data.filtered_file_list.files[index].name);
        if (data.dialog_type == FILE_DIALOG_SAVE) {
            file_remove_extension(data.selected_file);
        }
        encoding_from_utf8(data.selected_file, data.typed_name, FILE_NAME_MAX);
        if (data.dialog_type == FILE_DIALOG_SAVE) {
            input_box_refresh_text(&main_input);
            file_append_extension(data.selected_file, data.file_data->extension, FILE_NAME_MAX);
        }
        window_request_refresh();
    }
    if (data.dialog_type != FILE_DIALOG_DELETE && is_double_click) {
        button_ok_cancel(1, 0);
    }
}

static void file_tooltip(const list_box_item *item, tooltip_context *c)
{
    static uint8_t file[FILE_NAME_MAX];
    font_t font = item->is_selected ? FONT_NORMAL_WHITE : FONT_NORMAL_GREEN;
    encoding_from_utf8(data.filtered_file_list.files[item->index].name, file, FILE_NAME_MAX);
    if (text_get_width(file, font) > item->width) {
        c->precomposed_text = file;
        c->type = TOOLTIP_BUTTON;
    }
}

static void handle_tooltip(tooltip_context *c)
{
    list_box_handle_tooltip(&list_box, c);
}

void window_file_dialog_show(file_type type, file_dialog_type dialog_type)
{
    window_type window = {
        WINDOW_FILE_DIALOG,
        draw_background,
        draw_foreground,
        handle_input,
        handle_tooltip
    };
    init(type, dialog_type);
    window_show(&window);
}
