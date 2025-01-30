#include "select_campaign.h"

#include "core/dir.h"
#include "core/direction.h"
#include "core/image_group.h"
#include "core/lang.h"
#include "core/log.h"
#include "core/string.h"
#include "game/campaign.h"
#include "game/settings.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/list_box.h"
#include "graphics/panel.h"
#include "graphics/rich_text.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/property.h"
#include "scenario/scenario.h"
#include "widget/input_box.h"
#include "window/main_menu.h"
#include "window/mission_list.h"
#include "window/mission_selection.h"
#include "window/plain_message_dialog.h"

#define PLAYER_NAME_LENGTH 32
#define CAMPAIGN_LIST_Y_POSITION 96
#define ORIGINAL_CAMPAIGN_ID 0

static void button_start_mission(const generic_button *button);
static void button_mission_list(const generic_button *button);
static void button_back(const generic_button *button);
static void draw_campaign_item(const list_box_item *item);
static void select_campaign(unsigned int index, int is_double_click);
static void campaign_name_tooltip(const list_box_item *item, tooltip_context *c);

static generic_button bottom_buttons[] = {
    {16, 436, 90, 30, button_back, 0, TR_BUTTON_CANCEL },
    {444, 436, 180, 30, button_start_mission, 0, TR_WINDOW_CAMPAIGN_BUTTON_BEGIN_CAMPAIGN },
    {234, 436, 200, 30, button_mission_list, 0, TR_WINDOW_CAMPAIGN_BUTTON_MISSION_LIST }
};

#define NUM_BOTTOM_BUTTONS (sizeof(bottom_buttons) / sizeof(generic_button))

static struct {
    const dir_listing *campaign_list;
    uint8_t player_name[PLAYER_NAME_LENGTH];
    unsigned int available_buttons;
    unsigned int bottom_button_focus_id;
} data;

static list_box_type list_box = {
    .x = 16,
    .y = CAMPAIGN_LIST_Y_POSITION,
    .width_blocks = 20,
    .height_blocks = 21,
    .item_height = 16,
    .draw_inner_panel = 1,
    .extend_to_hidden_scrollbar = 1,
    .decorate_scrollbar = 1,
    .draw_item = draw_campaign_item,
    .on_select = select_campaign,
    .handle_tooltip = campaign_name_tooltip
};

static input_box player_name_input = { 304, 52, 20, 2, FONT_NORMAL_WHITE, 1, data.player_name, PLAYER_NAME_LENGTH };

static void calculate_input_box_width(void)
{
    int text_width = lang_text_get_width(31, 0, FONT_NORMAL_BLACK);
    player_name_input.x = ((text_width + 31) / BLOCK_SIZE) * BLOCK_SIZE;
    player_name_input.width_blocks = (624 - player_name_input.x) / BLOCK_SIZE;
}

static void init(void)
{
    game_campaign_clear();
    setting_clear_personal_savings();
    scenario_settings_init();
    const uint8_t *default_player_name = setting_player_name();
    if (!string_length(default_player_name)) {
        default_player_name = lang_get_string(9, 5);
    }
    player_name_input.placeholder = default_player_name;
    if (string_equals(player_name_input.placeholder, data.player_name)) {
        *data.player_name = 0;
    }
    data.campaign_list = dir_find_all_subdirectories_at_location(PATH_LOCATION_CAMPAIGN);
    data.campaign_list = dir_append_files_with_extension("campaign");
    calculate_input_box_width();
    input_box_start(&player_name_input);
    rich_text_set_fonts(FONT_NORMAL_BLACK, FONT_NORMAL_BLACK, FONT_NORMAL_BLACK, 5);
    list_box_init(&list_box, data.campaign_list->num_files + 1);
    list_box_select_index(&list_box, ORIGINAL_CAMPAIGN_ID);
}

static void draw_background(void)
{
    image_draw_fullscreen_background(image_group(GROUP_MAIN_MENU_BACKGROUND));

    graphics_in_dialog();
    outer_panel_draw(0, 0, 40, 30);
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_WINDOW_SELECT_CAMPAIGN, 32, 14, 554, FONT_LARGE_BLACK);
    lang_text_draw(31, 0, 16, 61, FONT_NORMAL_BLACK);

    const campaign_info *info = game_campaign_get_info();
    if (!info) {
        lang_text_draw_centered(CUSTOM_TRANSLATION, TR_SAVE_DIALOG_INVALID_FILE, 362, 241, 246, FONT_LARGE_BLACK);
        data.available_buttons = 1;
    } else {
        int y_offset = 40;
        text_draw_centered(info->name, 362, CAMPAIGN_LIST_Y_POSITION, 246, FONT_NORMAL_BLACK, 0);
        if (info->author) {
            int width = lang_text_draw(CUSTOM_TRANSLATION, TR_WINDOW_CAMPAIGN_AUTHOR,
                362, CAMPAIGN_LIST_Y_POSITION + 20, FONT_NORMAL_BLACK);
            text_draw(info->author, 362 + width, CAMPAIGN_LIST_Y_POSITION + 20, FONT_NORMAL_BLACK, 0);
            y_offset += 20;
        }
        if (info->description) {
            rich_text_reset(0);
            int box_width = rich_text_init(info->description, 62, CAMPAIGN_LIST_Y_POSITION + y_offset,
                246 / BLOCK_SIZE, (340 - y_offset) / BLOCK_SIZE, 0);
            rich_text_draw(info->description, 362, CAMPAIGN_LIST_Y_POSITION + y_offset,
                box_width * BLOCK_SIZE, (340 - y_offset) / BLOCK_SIZE, 0);
        } else {
            lang_text_draw_centered(CUSTOM_TRANSLATION, TR_WINDOW_CAMPAIGN_NO_DESC, 362, 246, 246, FONT_NORMAL_BLACK);
        }
        if (info->current_mission >= info->number_of_missions) {
            lang_text_draw_centered(CUSTOM_TRANSLATION, TR_WINDOW_CAMPAIGN_FINISHED,
                362, 414, 246, FONT_NORMAL_BLACK);
        } else if (info->current_mission > 0) {
            int width = lang_text_get_width(CUSTOM_TRANSLATION, TR_WINDOW_CAMPAIGN_CURRENT_MISSION, FONT_NORMAL_BLACK);
            width += text_get_number_width(info->current_mission + 1, '@', "", FONT_NORMAL_BLACK);
            int x_offset = (246 - width) / 2;
            x_offset += lang_text_draw(CUSTOM_TRANSLATION, TR_WINDOW_CAMPAIGN_CURRENT_MISSION,
                362 + x_offset, 414, FONT_NORMAL_BLACK);
            text_draw_number(info->current_mission + 1, '@', "", 362 + x_offset, 414, FONT_NORMAL_BLACK, 0);
        }

        data.available_buttons = info->current_mission > 0 ? 3 : 2;
    }

    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        int disabled = i >= data.available_buttons;
        text_draw_centered(lang_get_string(CUSTOM_TRANSLATION, bottom_buttons[i].parameter1), bottom_buttons[i].x,
            bottom_buttons[i].y + 9, bottom_buttons[i].width,
            disabled ? FONT_NORMAL_PLAIN : FONT_NORMAL_BLACK,
            disabled ? COLOR_FONT_LIGHT_GRAY : 0);
    }

    list_box_request_refresh(&list_box);

    graphics_reset_dialog();
}

static void draw_campaign_item(const list_box_item *item)
{
    uint8_t file[FILE_NAME_MAX];
    if (item->index == ORIGINAL_CAMPAIGN_ID) {
        string_copy(lang_get_string(CUSTOM_TRANSLATION, TR_WINDOW_ORIGINAL_CAMPAIGN_NAME), file, FILE_NAME_MAX);
    } else {
        encoding_from_utf8(data.campaign_list->files[item->index - 1].name, file, FILE_NAME_MAX);
        file_remove_extension((char *) file);
    }
    font_t font = item->is_selected ? FONT_NORMAL_WHITE : FONT_NORMAL_GREEN;
    text_ellipsize(file, font, item->width);
    text_draw(file, item->x, item->y, font, 0);
    if (item->is_focused) {
        button_border_draw(item->x - 4, item->y - 4, item->width + 6, item->height + 4, 1);
    }
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    list_box_draw(&list_box);
    input_box_draw(&player_name_input);
    for (int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        button_border_draw(bottom_buttons[i].x, bottom_buttons[i].y, bottom_buttons[i].width, bottom_buttons[i].height,
            data.bottom_button_focus_id == i + 1);
    }
    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (input_box_is_accepted()) {
        button_start_mission(0);
        return;
    }

    const mouse *m_dialog = mouse_in_dialog(m);

    if (input_box_handle_mouse(m_dialog, &player_name_input) ||
        generic_buttons_handle_mouse(m_dialog, 0, 0, bottom_buttons,
            data.available_buttons, &data.bottom_button_focus_id) ||
        list_box_handle_input(&list_box, m_dialog, 1)) {
        list_box_request_refresh(&list_box);
        return;
    }
    if (input_go_back_requested(m, h)) {
        button_back(0);
    }
}

static void button_back(const generic_button *button)
{
    input_box_stop(&player_name_input);
    game_campaign_clear();
    window_main_menu_show(0);
}

static void select_campaign(unsigned int index, int is_double_click)
{
    const char *campaign_file_name;
    if (index == ORIGINAL_CAMPAIGN_ID) {
        campaign_file_name = CAMPAIGN_ORIGINAL_NAME;
    } else {
        campaign_file_name = data.campaign_list->files[index - 1].name;
        if (!campaign_file_name) {
            return;
        }
    }
    game_campaign_load(campaign_file_name);
    window_request_refresh();

    if (is_double_click) {
        button_start_mission(0);
    }
}

static void button_start_mission(const generic_button *button)
{
    const campaign_info *info = game_campaign_get_info();
    if (!info) {
        window_plain_message_dialog_show(TR_WINDOW_INVALID_CAMPAIGN_TITLE,
            TR_WINDOW_INVALID_CAMPAIGN_TEXT, 1);
        return;
    }
    if (!*data.player_name) {
        string_copy(player_name_input.placeholder, data.player_name, PLAYER_NAME_LENGTH);
    }
    scenario_set_campaign_rank(info->starting_rank);
    scenario_set_player_name(data.player_name);
    scenario_save_campaign_player_name();
    setting_set_personal_savings_for_mission(0, 0);
    input_box_stop(&player_name_input);
    window_mission_selection_show();
}

static void button_mission_list(const generic_button *button)
{
    const campaign_info *info = game_campaign_get_info();
    if (!info || !info->current_mission) {
        return;
    }
    if (!*data.player_name) {
        string_copy(player_name_input.placeholder, data.player_name, PLAYER_NAME_LENGTH);
    }
    scenario_set_player_name(data.player_name);
    scenario_save_campaign_player_name();
    setting_set_personal_savings_for_mission(0, 0);
    input_box_stop(&player_name_input);
    window_mission_list_show();
}

static void campaign_name_tooltip(const list_box_item *item, tooltip_context *c)
{
    static uint8_t file[FILE_NAME_MAX];
    if (item->index == ORIGINAL_CAMPAIGN_ID) {
        string_copy(lang_get_string(CUSTOM_TRANSLATION, TR_WINDOW_ORIGINAL_CAMPAIGN_NAME), file, FILE_NAME_MAX);
    } else {
        encoding_from_utf8(data.campaign_list->files[item->index - 1].name, file, FILE_NAME_MAX);
        file_remove_extension((char *) file);
    }
    font_t font = item->is_selected ? FONT_NORMAL_WHITE : FONT_NORMAL_GREEN;
    if (text_get_width(file, font) > item->width) {
        c->precomposed_text = file;
        c->type = TOOLTIP_BUTTON;
    }
}

static void handle_tooltip(tooltip_context *c)
{
    list_box_handle_tooltip(&list_box, c);
}

void window_select_campaign_show(void)
{
    window_type window = {
        WINDOW_SELECT_CAMPAIGN,
        draw_background,
        draw_foreground,
        handle_input,
        handle_tooltip
    };
    init();
    window_show(&window);
}
