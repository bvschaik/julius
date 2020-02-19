#include "replay_confirm.h"

#include "game/settings.h"
#include "game/system.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/window.h"

static void button_confirm(int param1, int param2);
static void button_cancel(int param1, int param2);

static generic_button buttons[] = {
	{128, 140, 224, 20, button_confirm, button_none, 1, 0},
	{128, 208, 224, 20, button_cancel, button_none, 1, 0},
};

static struct {
	int focus_button_id;
	void(*close_callback)(void);
} data;

static void init(void(*close_callback)(void))
{
	data.focus_button_id = 0;
	data.close_callback = close_callback;
}

static void draw_foreground(void)
{
	graphics_in_dialog();

	outer_panel_draw(96, 80, 18, 12);

	lang_text_draw_centered(0, 2, 128, 94, 224, FONT_LARGE_BLACK);

	label_draw(128, 140, 14, data.focus_button_id == 3 ? 1 : 2);
	label_draw(128, 208, 14, data.focus_button_id == 4 ? 1 : 2);

	lang_text_draw_centered(0, 2, 128, 144, 224, FONT_NORMAL_GREEN);
	lang_text_draw_centered(42, 6, 128, 212, 224, FONT_NORMAL_GREEN);

	graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
	if (m->right.went_up) {
		data.close_callback();
	}
	else {
		generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, 2, &data.focus_button_id);
	}
}

static void button_confirm(int param1, int param2)
{
	if (scenario_is_custom()) {
		game_file_start_scenario_by_name(scenario_name());
		window_city_show();
	}
	else {
		scenario_save_campaign_player_name();
		window_mission_briefing_show();
	}
}

static void button_cancel(int param1, int param2)
{
	data.close_callback();
}

void window_confirm_replay_show(void(*close_callback)(void))
{
	window_type window = {
		WINDOW_DISPLAY_OPTIONS,
		window_draw_underlying_window,
		draw_foreground,
		handle_mouse
	};
	init(close_callback);
	window_show(&window);
}