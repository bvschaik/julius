#include "victory_dialog.h"

#include "assets/assets.h"
#include "city/victory.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/property.h"
#include "sound/music.h"
#include "window/city.h"

#define MAX_RANK 10

static void button_accept(int param1, int param2);
static void button_continue_governing(int months, int param2);

static generic_button victory_buttons[] = {
    {32, 112, 480, 20, button_accept, button_none, 0, 0},
    {32, 144, 480, 20, button_continue_governing, button_none, 24, 0},
    {32, 176, 480, 20, button_continue_governing, button_none, 60, 0},
};

static int focus_button_id = 0;

static int get_next_rank(void)
{
    int current_rank = 0;
    if (scenario_is_custom()) {
        current_rank = scenario_property_player_rank();
    } else {
        current_rank = scenario_campaign_rank();
    }
    if (current_rank < MAX_RANK) {
        return current_rank + 1;
    } else {
        return MAX_RANK;
    }
}

static void draw_background(void)
{
    window_draw_underlying_window();    	
    graphics_in_dialog();

    outer_panel_draw(48, 128, 34, 15);
    if (scenario_campaign_rank() < 10 || scenario_is_custom()) {
        lang_text_draw_centered(62, 0, 48, 144, 544, FONT_LARGE_BLACK);
        lang_text_draw_centered(62, 2, 48, 175, 544, FONT_NORMAL_BLACK);
        lang_text_draw_centered(32, get_next_rank(), 48, 194, 544, FONT_LARGE_BLACK);
    } else {
        text_draw_centered(scenario_player_name(), 48, 144, 512, FONT_LARGE_BLACK, 0);
        lang_text_draw_multiline(62, 26, 140, 175, 360, FONT_NORMAL_BLACK);
    }
    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    if (city_victory_state() == VICTORY_STATE_WON) {
        int image_id = assets_get_image_id("UI", "Victory_Banner");
        image_draw(image_id, 88, 137, COLOR_MASK_NONE, SCALE_NONE);
        image_draw(image_id, 512, 137, COLOR_MASK_NONE, SCALE_NONE);

        large_label_draw(80, 240, 30, focus_button_id == 1);
        if (scenario_campaign_rank() < 10 || scenario_is_custom()) {
            lang_text_draw_centered(62, 3, 80, 246, 480, FONT_NORMAL_GREEN);
        } else {
            lang_text_draw_centered(62, 27, 80, 246, 480, FONT_NORMAL_GREEN);
        }
        if (scenario_campaign_rank() >= 2 || scenario_is_custom()) {
            // Continue for 2/5 years
            large_label_draw(80, 272, 30, focus_button_id == 2);
            lang_text_draw_centered(62, 4, 80, 278, 480, FONT_NORMAL_GREEN);

            large_label_draw(80, 304, 30, focus_button_id == 3);
            lang_text_draw_centered(62, 5, 80, 310, 480, FONT_NORMAL_GREEN);
        }
    } else {
        // lost
        large_label_draw(80, 224, 30, focus_button_id == 1);
        lang_text_draw_centered(62, 6, 80, 230, 480, FONT_NORMAL_GREEN);
    }
    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    int num_buttons;
    if (scenario_campaign_rank() >= 2 || scenario_is_custom()) {
        num_buttons = 3;
    } else {
        num_buttons = 1;
    }
    generic_buttons_handle_mouse(mouse_in_dialog(m), 48, 128, victory_buttons, num_buttons, &focus_button_id);
}

static void button_accept(int param1, int param2)
{
    window_city_show();
}

static void button_continue_governing(int months, int param2)
{
    city_victory_continue_governing(months);
    window_city_show();
    city_victory_reset();
    sound_music_update(1);
}

void window_victory_dialog_show(void)
{
    window_type window = {
        WINDOW_VICTORY_DIALOG,
        draw_background,
        draw_foreground,
        handle_input
    };
    window_show(&window);
}
