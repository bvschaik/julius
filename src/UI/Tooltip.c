#include "Tooltip.h"

#include "Advisors.h"
#include "Window.h"

#include "../Graphics.h"

#include "../Data/CityInfo.h"
#include "../Data/Screen.h"

#include "core/lang.h"
#include "core/string.h"
#include "core/time.h"
#include "game/settings.h"
#include "graphics/lang_text.h"
#include "graphics/rich_text.h"
#include "graphics/text.h"

static int shouldDrawTooltip(struct TooltipContext *c);
static void resetTooltip(struct TooltipContext *c);
static void drawTooltip(struct TooltipContext *c);
static void drawButtonTooltip(struct TooltipContext *c);
static void drawOverlayTooltip(struct TooltipContext *c);
static void drawSenateTooltip(struct TooltipContext *c);

static time_millis lastUpdate = 0;

void UI_Tooltip_resetTimer()
{
	lastUpdate = time_get_millis();
}

void UI_Tooltip_handle(const mouse *m, void (*func)(struct TooltipContext *))
{
	struct TooltipContext tooltipContext = {m->x, m->y, 0, 0, 0, 0, 0, 0};
	tooltipContext.textGroup = 68; // default group
	tooltipContext.priority = TooltipPriority_Low;
	if (setting_tooltips() && func) {
		func(&tooltipContext);
	}
	if (shouldDrawTooltip(&tooltipContext)) {
		rich_text_save();
		drawTooltip(&tooltipContext);
		resetTooltip(&tooltipContext);
		rich_text_restore();
	}
}

static int shouldDrawTooltip(struct TooltipContext* c)
{
	if (c->type == TooltipType_None) {
		UI_Tooltip_resetTimer();
		return 0;
	}
	if (c->priority == TooltipPriority_Low && setting_tooltips() != TOOLTIPS_FULL) {
		UI_Tooltip_resetTimer();
		return 0;
	}
	if (time_get_millis() - lastUpdate < 150) { // delay drawing by 150 ms
		return 0;
	}
	return 1;
}

static void resetTooltip(struct TooltipContext *c)
{
	if (c->type != TooltipType_None) {
		c->type = TooltipType_None;
		UI_Window_requestRefresh();
	}
}

static void drawTooltip(struct TooltipContext *c)
{
	if (c->type == TooltipType_Overlay) {
		drawOverlayTooltip(c);
	} else if (c->type == TooltipType_Senate) {
		drawSenateTooltip(c);
	} else if (c->type == TooltipType_Button) {
		drawButtonTooltip(c);
	}
}

static void drawButtonTooltip(struct TooltipContext *c)
{
	const uint8_t *text = lang_get_string(c->textGroup, c->textId);
	rich_text_set_fonts(FONT_SMALL_PLAIN, FONT_SMALL_PLAIN);

	int width = 200;
	int lines = rich_text_draw(text, 0, 0, width - 5, 30, 1);
	if (lines > 2) {
		width = 300;
		lines = rich_text_draw(text, 0, 0, width - 5, 30, 1);
	}
	int height = 16 * lines + 10;

	int x, y;
	if (c->mouse_x < Data_Screen.offset640x480.x + width + 100) {
		if (UI_Window_getId() == Window_Advisors) {
			x = c->mouse_x + 50;
		} else {
			x = c->mouse_x + 20;
		}
	} else {
		x = c->mouse_x - width - 20;
	}
	
	switch (UI_Window_getId()) {
		case Window_Advisors:
			if (c->mouse_y < Data_Screen.offset640x480.y + 432) {
				y = c->mouse_y;
				switch (UI_Advisors_getId()) {
					case ADVISOR_LABOR: y -= 74; break;
					case ADVISOR_TRADE: y -= 54; break;
					case ADVISOR_POPULATION: y -= 58; break;
					default: y -= 64; break;
				}
			} else {
				y = Data_Screen.offset640x480.y + 432;
			}
			break;
		case Window_TradePricesDialog: // FIXED used to cause ghosting
			y = c->mouse_y - 42;
			break;
		case Window_DonateToCityDialog:
			y = c->mouse_y - 52;
			break;
		case Window_LaborPriorityDialog:
			x = c->mouse_x - width / 2 - 10;
			if (c->mouse_y < Data_Screen.offset640x480.y + 200) {
				y = c->mouse_y + 40;
			} else {
				y = c->mouse_y - 72;
			}
			break;
		default:
			if (c->mouse_y < Data_Screen.offset640x480.y + 200) {
				y = c->mouse_y + 40;
			} else {
				y = c->mouse_y - 62;
			}
			break;
	}

	Graphics_drawRect(x, y, width, height, COLOR_BLACK);
	Graphics_fillRect(x + 1, y + 1, width - 2, height - 2, COLOR_WHITE);
	rich_text_draw_colored(text, x + 5, y + 7,
		width - 5, lines, COLOR_TOOLTIP);
}

static uint8_t tmpString[1000];
static void drawOverlayTooltip(struct TooltipContext *c)
{
	const uint8_t *text = lang_get_string(c->textGroup, c->textId);
	if (c->hasNumericPrefix) {
		int offset = string_from_int(tmpString, c->numericPrefix, 0);
		string_copy(text, &tmpString[offset], 1000);
		text = tmpString;
	}
	rich_text_set_fonts(FONT_SMALL_PLAIN, FONT_SMALL_PLAIN);

	int width = 200;
	int lines = rich_text_draw(text, 0, 0, width - 5, 30, 1);
	if (lines > 2) {
		width = 300;
		lines = rich_text_draw(text, 0, 0, width - 5, 30, 1);
	}
	int height = 16 * lines + 10;

	int x, y;
	if (c->mouse_x < width + 20) {
		x = c->mouse_x + 20;
	} else {
		x = c->mouse_x - width - 20;
	}
	if (c->mouse_y < 200) {
		y = c->mouse_y + 50;
	} else {
		y = c->mouse_y - 72;
	}

	Graphics_drawRect(x, y, width, height, COLOR_BLACK);
	Graphics_fillRect(x + 1, y + 1, width - 2, height - 2, COLOR_WHITE);
	rich_text_draw_colored(text, x + 5, y + 7,
		width - 5, lines, COLOR_TOOLTIP);
}

static void drawSenateTooltip(struct TooltipContext *c)
{
    int x, y;
	int width = 180;
	int height = 80;
	if (c->mouse_x < width + 20) {
		x = c->mouse_x + 20;
	} else {
		x = c->mouse_x - width - 20;
	}
	if (c->mouse_y < 200) {
		y = c->mouse_y + 10;
	} else {
		y = c->mouse_y - 32;
	}
	
	Graphics_drawRect(x, y, width, height, COLOR_BLACK);
	Graphics_fillRect(x + 1, y + 1, width - 2, height - 2, COLOR_WHITE);
	
	// unemployment
	lang_text_draw_colored(68, 148, x + 5, y + 5, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
	Widget_Text_drawNumberColored(Data_CityInfo.unemploymentPercentage, '@', "%",
		x + 140, y + 5, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
	
	// ratings
	lang_text_draw_colored(68, 149, x + 5, y + 19, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
	Widget_Text_drawNumberColored(Data_CityInfo.ratingCulture, '@', " ",
		x + 140, y + 19, FONT_SMALL_PLAIN, COLOR_TOOLTIP);

	lang_text_draw_colored(68, 150, x + 5, y + 33, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
	Widget_Text_drawNumberColored(Data_CityInfo.ratingProsperity, '@', " ",
		x + 140, y + 33, FONT_SMALL_PLAIN, COLOR_TOOLTIP);

	lang_text_draw_colored(68, 151, x + 5, y + 47, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
	Widget_Text_drawNumberColored(Data_CityInfo.ratingPeace, '@', " ",
		x + 140, y + 47, FONT_SMALL_PLAIN, COLOR_TOOLTIP);

	lang_text_draw_colored(68, 152, x + 5, y + 61, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
	Widget_Text_drawNumberColored(Data_CityInfo.ratingFavor, '@', " ",
		x + 140, y + 61, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
}
