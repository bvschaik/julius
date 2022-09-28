#include "assets/assets.h"
#include "city/data_private.h"
#include "city/race_bet.h"
#include "core/calc.h"
#include "graphics/arrow_button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "race_bet.h"
#include "translation/translation.h"

static void arrow_button_bet(int is_down, int param2);
static void button_horse_selection(int option, int param2);
static void button_confirm(int option, int param2);
static void button_close(int param1, int param2);

static generic_button buttons[] = {
        {34, 145, 81, 91, button_horse_selection, button_none, BLUE_HORSE, 0},
        {144, 145, 81, 91, button_horse_selection, button_none, RED_HORSE, 0},
        {254, 145, 81, 91, button_horse_selection, button_none, WHITE_HORSE, 0},
        {364, 145, 81, 91, button_horse_selection, button_none, GREEN_HORSE, 0}
};

static arrow_button amount_buttons[] = {
        {106, 306, 17, 24, arrow_button_bet, 1, 0},
        {130, 306, 15, 24, arrow_button_bet, 0, 0}
};
static generic_button bet_buttons[] = {
        {90, 354, 300, 20, button_confirm, button_none, 1, 0},
};

static image_button image_button_close[] = {
        {424, 354, 24, 24, IB_NORMAL, GROUP_CONTEXT_ICONS, 4, button_close, button_none, 0, 0, 1}
};


static struct {
    int chosen_horse;
    int bet_amount;
    int in_progress_bet;
    int focus_button_id;
    int focus_button_id2;
    int focus_button_id3;
    int focus_image_button_id;
    int width_blocks;
    int height_blocks;
} data;

static int init(void)
{
    if (window_is(WINDOW_RACE_BET)) {
        // don't show popup over popup
        return 0;
    }
    data.in_progress_bet = city_data.games.chosen_horse ? 1 : 0;
    data.chosen_horse = city_data.games.chosen_horse ? city_data.games.chosen_horse : NO_BET;
    data.bet_amount = city_data.games.bet_amount ? city_data.games.bet_amount: 0;
    data.width_blocks = 30;
    data.height_blocks = 25;

    return 1;
}


static void draw_background(void)
{
    window_draw_underlying_window();

    graphics_in_dialog_with_size(BLOCK_SIZE * data.width_blocks, BLOCK_SIZE * data.height_blocks);

    outer_panel_draw(0, 0, data.width_blocks, data.height_blocks);

    image_draw(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_DENARII, 20, 20, COLOR_MASK_NONE, SCALE_NONE);

    text_draw_centered(translation_for(TR_WINDOW_RACE_BET_TITLE), 0, 20, BLOCK_SIZE * data.width_blocks, FONT_LARGE_BLACK, 0);

    text_draw_multiline(translation_for(TR_WINDOW_RACE_BET_DESCRIPTION), 25, 65, 438, FONT_NORMAL_BLACK, 0);

    inner_panel_draw(18, 300, 28, 2);
    text_draw_centered(translation_for(TR_WINDOW_RACE_BET_AMOUNT), 18, 310, 80, FONT_NORMAL_WHITE, 0);
    int width = text_draw_number(data.bet_amount, '@', " ", 165, 310, FONT_NORMAL_WHITE, 0);
    width += lang_text_draw(50, 15, 165 + width, 310, FONT_NORMAL_WHITE);
    text_draw_with_money(translation_for(TR_PERSONAL_SAVINGS), city_emperor_personal_savings(), " ", "", 284, 310, 175,  FONT_NORMAL_WHITE, 0);

    translation_key horse_description = 0;
    if (data.focus_button_id) {
        horse_description = TR_WINDOW_RACE_BLUE_HORSE_DESCRIPTION + data.focus_button_id - 1;
    } else if (data.chosen_horse) {
        horse_description = TR_WINDOW_RACE_BLUE_HORSE_DESCRIPTION + data.chosen_horse - 1;
    }
    if (horse_description) {
        text_draw_multiline(translation_for(horse_description), 25, 250, 438, FONT_NORMAL_BLACK, 0);
    }

    int button_enabled = data.bet_amount > 0 && data.chosen_horse != 0 && !data.in_progress_bet;

    text_draw_centered(translation_for(data.in_progress_bet ? TR_WINDOW_IN_PROGRESS_BET_BUTTON :
        TR_WINDOW_RACE_BET_BUTTON), 90, 358, 300, button_enabled ? FONT_NORMAL_BLACK : FONT_NORMAL_PLAIN,
        button_enabled ? 0 : COLOR_FONT_LIGHT_GRAY);

    int image_id = assets_get_image_id("UI", "Hipp_Team_Blue");

    for (int i = 0; i < 4; i++) {
        image_draw(image_id + i, 39 + i * 110, 150, COLOR_MASK_NONE, SCALE_NONE);
    }

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog_with_size(BLOCK_SIZE * data.width_blocks, BLOCK_SIZE * data.height_blocks);

    int border_id = assets_get_image_id("UI", "Image Border Small");

    for (int i = 0; i < 4; i++) {
        color_t color = data.focus_button_id == (i + 1) || data.chosen_horse == (i + 1) ?
            COLOR_BORDER_RED : COLOR_BORDER_GREEN;
        image_draw_border(border_id, 34 + i * 110, 145, color);
    }

    arrow_buttons_draw(0, 0, amount_buttons, 2);

    int button_enabled = data.bet_amount > 0 && data.chosen_horse != 0 && !data.in_progress_bet;

    button_border_draw(90, 354, 300, 20, button_enabled && data.focus_button_id3 == 1);
    image_buttons_draw(0, 0, image_button_close, 1);

    graphics_reset_dialog();
    window_request_refresh();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog_with_size(m, data.width_blocks * BLOCK_SIZE, data.height_blocks * BLOCK_SIZE);

    if (image_buttons_handle_mouse(m_dialog, 0, 0, image_button_close, 1, &data.focus_image_button_id)) {
        return;
    }

    if (input_go_back_requested(m, h)) {
        window_go_back();
    }

    if (generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, 4, &data.focus_button_id) ||
        arrow_buttons_handle_mouse(m_dialog, 0, 0, amount_buttons, 2, &data.focus_button_id2) ||
        generic_buttons_handle_mouse(m_dialog, 0, 0, bet_buttons, 1, &data.focus_button_id3)) {
        return;
    }
}

static void arrow_button_bet(int is_down, int param2)
{
    if (!data.in_progress_bet) {
        int amount = data.bet_amount;
        amount += is_down ? -10 : 10;
        data.bet_amount = calc_bound(amount, 0, city_data.emperor.personal_savings);

        window_request_refresh();
    }
}

static void button_horse_selection(int option, int param2)
{
    if (!data.in_progress_bet) {
        data.chosen_horse = option;
        window_request_refresh();
    }
}

static void button_confirm(int option, int param2)
{
    // save bet and go back
    if (!city_data.games.chosen_horse && data.chosen_horse && data.bet_amount) {
        city_data.games.chosen_horse = data.chosen_horse;
        city_data.games.bet_amount = data.bet_amount;
        window_go_back();
    }
}

static void handle_tooltip(tooltip_context *c)
{
    if (data.focus_image_button_id) { // "Exit this panel"
        c->type = TOOLTIP_BUTTON;
        c->text_group = 68;
        c->text_id = 2;
    } else if (data.focus_button_id) {
        c->type = TOOLTIP_BUTTON;
        c->translation_key = TR_WINDOW_RACE_BET_BLUE_HORSE + data.focus_button_id - 1;
    }
}

static void button_close(int param1, int param2)
{
    window_go_back();
}

void window_race_bet_show(void)
{
    if (init()) {
        window_type window = {
                WINDOW_RACE_BET,
                draw_background,
                draw_foreground,
                handle_input,
                handle_tooltip
        };
        window_show(&window);
    }
}
