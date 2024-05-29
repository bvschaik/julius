#include "new_campaign.h"

#include "campaign/campaign.h"
#include "core/dir.h"
#include "core/direction.h"
#include "core/image_group.h"
#include "core/lang.h"
#include "core/log.h"
#include "core/string.h"
#include "game/settings.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
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
#include "window/mission_selection.h"
#include "window/plain_message_dialog.h"

#define PLAYER_NAME_LENGTH 32
#define CAMPAIGN_LIST_Y_POSITION 96
#define ORIGINAL_CAMPAIGN_ID 0

static void start_mission(int param1, int param2);
static void button_back(int param1, int param2);
static void draw_campaign_item(const list_box_item *item);
static void select_campaign(unsigned int index, int is_double_click);
static void campaign_name_tooltip(const list_box_item *item, tooltip_context *c);

static image_button image_buttons[] = {
    {536, 440, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 0, start_mission, button_none, 0, 0, 1},
    {584, 440, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 4,   button_back, button_none, 1, 0, 1}
};

static struct {
    const dir_listing *campaign_list;
    uint8_t player_name[PLAYER_NAME_LENGTH];
} data;

static campaign_info original_campaign_info = {
    .number_of_missions = 11
};

static list_box_type list_box = {
    .x = 16,
    .y = CAMPAIGN_LIST_Y_POSITION,
    .width_blocks = 20,
    .height_blocks = 23,
    .item_height = 16,
    .draw_inner_panel = 1,
    .extend_to_hidden_scrollbar = 1,
    .decorate_scrollbar = 1,
    .draw_item = draw_campaign_item,
    .on_select = select_campaign,
    .handle_tooltip = campaign_name_tooltip
};

static input_box player_name_input = { 304, 52, 20, 2, FONT_NORMAL_WHITE, 1, data.player_name, PLAYER_NAME_LENGTH };

static void reset_campaign_data(void)
{
    original_campaign_info.name = lang_get_string(CUSTOM_TRANSLATION, TR_WINDOW_ORIGINAL_CAMPAIGN_NAME);
    original_campaign_info.description = lang_get_string(CUSTOM_TRANSLATION, TR_WINDOW_ORIGINAL_CAMPAIGN_DESC);

    campaign_clear();
}

static void calculate_input_box_width(void)
{
    int text_width = lang_text_get_width(31, 0, FONT_NORMAL_BLACK);
    player_name_input.x = ((text_width + 31) / BLOCK_SIZE) * BLOCK_SIZE;
    player_name_input.width_blocks = (624 - player_name_input.x) / BLOCK_SIZE;
}

static void init(void)
{
    reset_campaign_data();
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

static const campaign_info *get_campaign_info(void)
{
    return list_box_get_selected_index(&list_box) == ORIGINAL_CAMPAIGN_ID ?
        &original_campaign_info : campaign_get_info();
}

static void draw_background(void)
{
    image_draw_fullscreen_background(image_group(GROUP_MAIN_MENU_BACKGROUND));

    graphics_in_dialog();
    outer_panel_draw(0, 0, 40, 30);
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_WINDOW_SELECT_CAMPAIGN, 32, 14, 554, FONT_LARGE_BLACK);
    lang_text_draw(31, 0, 16, 61, FONT_NORMAL_BLACK);

    // Proceed? text
    lang_text_draw_right_aligned(43, 5, 362, 447, 164, FONT_NORMAL_BLACK);

    const campaign_info *info = get_campaign_info();
    if (!info) {
        lang_text_draw_centered(CUSTOM_TRANSLATION, TR_SAVE_DIALOG_INVALID_FILE, 362, 241, 246, FONT_LARGE_BLACK);
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
    image_buttons_draw(0, 0, image_buttons, 2);
    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (input_box_is_accepted()) {
        start_mission(0, 0);
        return;
    }

    const mouse *m_dialog = mouse_in_dialog(m);

    if (input_box_handle_mouse(m_dialog, &player_name_input) ||
        image_buttons_handle_mouse(m_dialog, 0, 0, image_buttons, 2, 0) ||
        list_box_handle_input(&list_box, m_dialog, 1)) {
        list_box_request_refresh(&list_box);
        return;
    }
    if (input_go_back_requested(m, h)) {
        button_back(0, 0);
    }
}

static void button_back(int param1, int param2)
{
    input_box_stop(&player_name_input);
    campaign_clear();
    window_go_back();
}

static void select_campaign(unsigned int index, int is_double_click)
{
    if (index == ORIGINAL_CAMPAIGN_ID) {
        campaign_clear();
    } else {
        const char *campaign_file_name = data.campaign_list->files[index - 1].name;
        if (!campaign_file_name) {
            return;
        }
        campaign_load(campaign_file_name);
    }
    window_request_refresh();

    if (is_double_click) {
        start_mission(0, 0);
    }
}

static void start_mission(int param1, int param2)
{
    const campaign_info *info = get_campaign_info();
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

void window_new_campaign_show(void)
{
    window_type window = {
        WINDOW_NEW_CAMPAIGN,
        draw_background,
        draw_foreground,
        handle_input,
        handle_tooltip
    };
    init();
    window_show(&window);
}
