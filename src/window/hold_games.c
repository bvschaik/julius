#include "hold_games.h"

#include "building/count.h"
#include "city/constants.h"
#include "city/data_private.h"
#include "city/festival.h"
#include "city/finance.h"
#include "city/gods.h"
#include "core/image_group.h"
#include "game/resource.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/text.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "input/input.h"
#include "translation/translation.h"
#include "window/advisors.h"
#include "window/message_dialog.h"

static void button_game(int god, int param2);
static void button_size(int size, int param2);
static void button_help(int param1, int param2);
static void button_close(int param1, int param2);
static void button_hold_games(int param1, int param2);

static image_button image_buttons_bottom[] = {
    {58, 350, 27, 27, IB_NORMAL, GROUP_CONTEXT_ICONS, 0, button_help, button_none, 0, 0, 1},
    {558, 350, 24, 24, IB_NORMAL, GROUP_CONTEXT_ICONS, 4, button_close, button_none, 0, 0, 1},
    {500, 350, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 4, button_close, button_none, 0, 0, 1}
};

static image_button action_button[] = {
    {458, 350, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 0, button_hold_games, button_none, 1, 0, 1}
};

static generic_button buttons_gods_size[] = {
    {70, 96, 80, 90, button_game, button_none, 0, 0},
    {170, 96, 80, 90, button_game, button_none, 1, 0},
    {270, 96, 80, 90, button_game, button_none, 2, 0},
    {370, 96, 80, 90, button_game, button_none, 3, 0},
};

typedef struct {
    int header_key;
    int description_key;
    int cost;
    int delay_months;
    int building_id_required;
    int resource_cost[RESOURCE_MAX];
} games_type;

games_type ALL_GAMES[MAX_GAMES] = {
    {TR_WINDOW_GAMES_OPTION_1, TR_WINDOW_GAMES_OPTION_1_DESC, 100, 1, 33, {0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,}},
    {TR_WINDOW_GAMES_OPTION_2, TR_WINDOW_GAMES_OPTION_2_DESC, 0, 1, 33, {0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,}},
    {TR_WINDOW_GAMES_OPTION_3, TR_WINDOW_GAMES_OPTION_3_DESC, 0, 1, 33, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,}},
    {TR_WINDOW_GAMES_OPTION_4, TR_WINDOW_GAMES_OPTION_4_DESC, 100, 1, 32, {0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,}}
};


static int focus_button_id;
static int focus_image_button_id;

static void draw_background(void)
{
    int selected_game = city_data.games.selected_games_id;
    window_advisors_draw_dialog_background();

    graphics_in_dialog();

    outer_panel_draw(48, 48, 34, 22);
    text_draw_centered(translation_for(ALL_GAMES[selected_game].header_key), 48, 60, 544, FONT_LARGE_BLACK, 0);
    for (int game = 0; game < MAX_GAMES; game++) {
        if (game == selected_game) {
            button_border_draw(100 * game + 66, 92, 90, 100, 1);
            image_draw(image_group(GROUP_PANEL_WINDOWS) + game + 21, 100 * game + 70, 96);
        }
        else {
            image_draw(image_group(GROUP_PANEL_WINDOWS) + game + 16, 100 * game + 70, 96);
        }
    }
    text_draw_multiline(translation_for(ALL_GAMES[selected_game].description_key), 70, 222, 500, FONT_NORMAL_BLACK, 0);
    

    int width = text_draw(translation_for(TR_WINDOW_GAMES_COST), 120, 300, FONT_NORMAL_BLACK, 0);
    width += text_draw_money(ALL_GAMES[selected_game].cost, 120 + width, 300, FONT_NORMAL_BLACK);
    text_draw(translation_for(TR_WINDOW_GAMES_PERSONAL_FUNDS), 120 + width, 300, FONT_NORMAL_BLACK, 0);

    width = 0;
    int has_resources = 1;
    for (int i = 0; i < RESOURCE_MAX; ++i) {        
        if (ALL_GAMES[selected_game].resource_cost[i]) {
            width += text_draw_number(ALL_GAMES[selected_game].resource_cost[i], '@', "", 120 + width, 320, FONT_NORMAL_BLACK);
            if (city_resource_get_stored(i) < ALL_GAMES[selected_game].resource_cost[i]) {
                has_resources = 0;
            }
            image_draw(image_group(GROUP_RESOURCE_ICONS) + i, 120 + width, 316);    
            width += 32;
        }
    }

    if (!building_count_active(ALL_GAMES[selected_game].building_id_required)) {
        text_draw(translation_for(TR_WINDOW_GAMES_NO_VENUE), 130, 352, FONT_NORMAL_BLACK, 0);
    } else if (city_emperor_personal_savings() < ALL_GAMES[selected_game].cost) {
        text_draw(translation_for(TR_WINDOW_GAMES_NOT_ENOUGH_FUNDS), 130, 352, FONT_NORMAL_BLACK, 0);
    }
    else if (!has_resources){
        text_draw(translation_for(TR_WINDOW_GAMES_NOT_ENOUGH_RESOURCES), 130, 352, FONT_NORMAL_BLACK, 0);
    }
    else {
        text_draw(translation_for(ALL_GAMES[selected_game].header_key), 130, 352, FONT_NORMAL_BLACK, 0);
        image_buttons_draw(0, 0, action_button, 1);
    }
         
    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    image_buttons_draw(0, 0, image_buttons_bottom, 3);
    graphics_reset_dialog();
}

static void handle_input(const mouse* m, const hotkeys* h)
{
    const mouse* m_dialog = mouse_in_dialog(m);
    int handled = 0;
    handled |= image_buttons_handle_mouse(m_dialog, 0, 0, image_buttons_bottom, 4, &focus_image_button_id);
    handled |= generic_buttons_handle_mouse(m_dialog, 0, 0, buttons_gods_size, 8, &focus_button_id);
    if (focus_image_button_id) {
        focus_button_id = 0;
    }
    if (!handled && input_go_back_requested(m, h)) {
        window_advisors_show();
    }
}

static void button_game(int game, int param2)
{
    city_data.games.selected_games_id = game;
    window_invalidate();
}

static void button_size(int size, int param2)
{
    if (!city_finance_out_of_money()) {
        if (city_festival_select_size(size)) {
            window_invalidate();
        }
    }
}

static void button_help(int param1, int param2)
{
    window_message_dialog_show(MESSAGE_DIALOG_ADVISOR_ENTERTAINMENT, 0);
}

static void button_close(int param1, int param2)
{
    window_advisors_show();
}

static void button_hold_games(int param1, int param2)
{
    city_festival_games_schedule(1);
    window_advisors_show();
}

static void get_tooltip(tooltip_context* c)
{
    if (!focus_image_button_id && (!focus_button_id || focus_button_id > 5)) {
        return;
    }
    c->type = TOOLTIP_BUTTON;
    // image buttons
    switch (focus_image_button_id) {
    case 1: c->text_id = 1; break;
    case 2: c->text_id = 2; break;
    }

}

void window_hold_games_show(void)
{
    window_type window = {
        WINDOW_HOLD_GAMES,
        draw_background,
        draw_foreground,
        handle_input,
        get_tooltip
    };
    window_show(&window);
}
