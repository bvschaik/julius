#include "Advisors_private.h"

#include "Tooltip.h"

#include "city/finance.h"
#include "city/labor.h"
#include "core/calc.h"
#include "graphics/arrow_button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"

static void buttonSetPriority(int newPriority, int param2);

// labor priority stuff
static int prioritySelectedCategory;
static int priorityMaxItems;
static int priorityFocusButtonId;

static generic_button priorityButtons[] = {
    {180, 256, 460, 281, GB_IMMEDIATE, buttonSetPriority, button_none, 0, 0}, // no prio
    {178, 221, 205, 248, GB_IMMEDIATE, buttonSetPriority, button_none, 1, 0},
    {210, 221, 237, 248, GB_IMMEDIATE, buttonSetPriority, button_none, 2, 0},
    {242, 221, 269, 248, GB_IMMEDIATE, buttonSetPriority, button_none, 3, 0},
    {274, 221, 301, 248, GB_IMMEDIATE, buttonSetPriority, button_none, 4, 0},
    {306, 221, 333, 248, GB_IMMEDIATE, buttonSetPriority, button_none, 5, 0},
    {338, 221, 365, 248, GB_IMMEDIATE, buttonSetPriority, button_none, 6, 0},
    {370, 221, 397, 248, GB_IMMEDIATE, buttonSetPriority, button_none, 7, 0},
    {402, 221, 429, 248, GB_IMMEDIATE, buttonSetPriority, button_none, 8, 0},
    {434, 221, 461, 248, GB_IMMEDIATE, buttonSetPriority, button_none, 9, 0},
};

void UI_LaborPriorityDialog_drawBackground()
{
	int dummy;

	UI_Advisor_Labor_drawBackground(&dummy);
	UI_Advisor_Labor_drawForeground();

    graphics_in_dialog();
	outer_panel_draw(160, 176, 20, 9);
	lang_text_draw_centered(50, 25, 160, 185, 320, FONT_LARGE_BLACK);
	for (int i = 0; i < 9; i++) {
		graphics_draw_rect(178 + 32 * i, 221, 27, 27, COLOR_BLACK);
		lang_text_draw_centered(50, 27 + i, 178 + 32 * i, 224, 27, FONT_LARGE_BLACK);
		if (i >= priorityMaxItems) {
			graphics_shade_rect(179 + 32 * i, 222, 25, 25, 1);
		}
	}

	graphics_draw_rect(180, 256, 280, 25, COLOR_BLACK);
	lang_text_draw_centered(50, 26, 180, 263, 280, FONT_NORMAL_BLACK);
	lang_text_draw_centered(13, 3, 160, 296, 320, FONT_NORMAL_BLACK);
    graphics_reset_dialog();
}

void UI_LaborPriorityDialog_drawForeground()
{
    graphics_in_dialog();
	
	color_t color;
	for (int i = 0; i < 9; i++) {
		color = COLOR_BLACK;
		if (i == priorityFocusButtonId - 2) {
			color = COLOR_RED;
		}
		graphics_draw_rect(178 + 32 * i, 221, 27, 27, color);
	}
	color = COLOR_BLACK;
	if (priorityFocusButtonId == 1) {
		color = COLOR_RED;
	}
	graphics_draw_rect(180, 256, 280, 25, color);

    graphics_reset_dialog();
}

void UI_LaborPriorityDialog_handleMouse(const mouse *m)
{
	if (m->right.went_up) {
		window_advisors_show();
	} else {
		generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, priorityButtons, 1 + priorityMaxItems, &priorityFocusButtonId);
	}
}

static void buttonSetPriority(int newPriority, int param2)
{
    city_labor_set_priority(prioritySelectedCategory, newPriority);
	window_advisors_show();
}

void UI_LaborPriorityDialog_getTooltip(struct TooltipContext *c)
{
	if (!priorityFocusButtonId) {
		return;
	}
	c->type = TooltipType_Button;
	if (priorityFocusButtonId == 1) {
		c->textId = 92;
	} else {
		c->textId = 93;
	}
}
