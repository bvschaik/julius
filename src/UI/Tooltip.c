#include "Tooltip.h"

#include "Advisors.h"
#include "Window.h"

#include "../Graphics.h"
#include "../Widget.h"

#include "../Data/CityInfo.h"
#include "../Data/Constants.h"
#include "../Data/Screen.h"
#include "../Data/Settings.h"

#include "core/lang.h"
#include "core/string.h"
#include "core/time.h"

#include <string.h>

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

void UI_Tooltip_handle(void (*func)(struct TooltipContext *))
{
	struct TooltipContext tooltipContext = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	tooltipContext.textGroup = 68; // default group
	tooltipContext.priority = TooltipPriority_Low;
	if (Data_Settings.mouseTooltips && func) {
		func(&tooltipContext);
	}
	if (shouldDrawTooltip(&tooltipContext)) {
		Widget_RichText_save();
		drawTooltip(&tooltipContext);
		resetTooltip(&tooltipContext);
		Widget_RichText_restore();
	}
}

static int shouldDrawTooltip(struct TooltipContext* c)
{
	if (c->type == TooltipType_None) {
		UI_Tooltip_resetTimer();
		return 0;
	}
	if (c->priority == TooltipPriority_Low && Data_Settings.mouseTooltips < 2) {
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
	Widget_RichText_setFonts(FONT_SMALL_PLAIN, FONT_SMALL_PLAIN);

	c->width = 200;
	int lines = Widget_RichText_draw(text, c->x + 5, c->y + 5,
		c->width - 5, 30, 1);
	if (lines > 2) {
		c->width = 300;
		lines = Widget_RichText_draw(text, c->x + 5, c->y + 5,
			c->width - 5, 30, 1);
	}
	c->height = 16 * lines + 10;

    const mouse *m = mouse_get();
	if (m->x < Data_Screen.offset640x480.x + c->width + 100) {
		if (UI_Window_getId() == Window_Advisors) {
			c->x = m->x + 50;
		} else {
			c->x = m->x + 20;
		}
	} else {
		c->x = m->x - c->width - 20;
	}
	
	switch (UI_Window_getId()) {
		case Window_Advisors:
			if (m->y < Data_Screen.offset640x480.y + 432) {
				c->y = m->y;
				switch (UI_Advisors_getId()) {
					case Advisor_Labor: c->y -= 74; break;
					case Advisor_Trade: c->y -= 54; break;
					case Advisor_Population: c->y -= 58; break;
					default: c->y -= 64; break;
				}
			} else {
				c->y = Data_Screen.offset640x480.y + 432;
			}
			break;
		case Window_TradePricesDialog: // FIXED used to cause ghosting
			c->y = m->y - 42;
			break;
		case Window_DonateToCityDialog:
			c->y = m->y - 52;
			break;
		case Window_LaborPriorityDialog:
			c->x = m->x - c->width / 2 - 10;
			if (m->y < Data_Screen.offset640x480.y + 200) {
				c->y = m->y + 40;
			} else {
				c->y = m->y - 72;
			}
			break;
		default:
			if (m->y < Data_Screen.offset640x480.y + 200) {
				c->y = m->y + 40;
			} else {
				c->y = m->y - 62;
			}
			break;
	}

	Graphics_drawRect(c->x, c->y, c->width, c->height, COLOR_BLACK);
	Graphics_fillRect(c->x + 1, c->y + 1, c->width - 2, c->height - 2, COLOR_WHITE);
	Widget_RichText_drawColored(text, c->x + 5, c->y + 7,
		c->width - 5, lines, COLOR_TOOLTIP);
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
	Widget_RichText_setFonts(FONT_SMALL_PLAIN, FONT_SMALL_PLAIN);

	c->width = 200;
	int lines = Widget_RichText_draw(text, c->x + 5, c->y + 5,
		c->width - 5, 30, 1);
	if (lines > 2) {
		c->width = 300;
		lines = Widget_RichText_draw(text, c->x + 5, c->y + 5,
			c->width - 5, 30, 1);
	}
	c->height = 16 * lines + 10;

    const mouse *m = mouse_get();
	if (m->x < c->width + 20) {
		c->x = m->x + 20;
	} else {
		c->x = m->x - c->width - 20;
	}
	if (m->y < 200) {
		c->y = m->y + 50;
	} else {
		c->y = m->y - 72;
	}

	Graphics_drawRect(c->x, c->y, c->width, c->height, COLOR_BLACK);
	Graphics_fillRect(c->x + 1, c->y + 1, c->width - 2, c->height - 2, COLOR_WHITE);
	Widget_RichText_drawColored(text, c->x + 5, c->y + 7,
		c->width - 5, lines, COLOR_TOOLTIP);
}

static void drawSenateTooltip(struct TooltipContext *c)
{
    const mouse *m = mouse_get();
	c->width = 180;
	c->height = 80;
	if (m->x < c->width + 20) {
		c->x = m->x + 20;
	} else {
		c->x = m->x - c->width - 20;
	}
	if (m->y < 200) {
		c->y = m->y + 10;
	} else {
		c->y = m->y - 32;
	}
	
	Graphics_drawRect(c->x, c->y, c->width, c->height, COLOR_BLACK);
	Graphics_fillRect(c->x + 1, c->y + 1, c->width - 2, c->height - 2, COLOR_WHITE);
	
	// unemployment
	Widget_GameText_drawColored(68, 148, c->x + 5, c->y + 5, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
	Widget_Text_drawNumberColored(Data_CityInfo.unemploymentPercentage, '@', "%",
		c->x + 140, c->y + 5, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
	
	// ratings
	Widget_GameText_drawColored(68, 149, c->x + 5, c->y + 19, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
	Widget_Text_drawNumberColored(Data_CityInfo.ratingCulture, '@', " ",
		c->x + 140, c->y + 19, FONT_SMALL_PLAIN, COLOR_TOOLTIP);

	Widget_GameText_drawColored(68, 150, c->x + 5, c->y + 33, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
	Widget_Text_drawNumberColored(Data_CityInfo.ratingProsperity, '@', " ",
		c->x + 140, c->y + 33, FONT_SMALL_PLAIN, COLOR_TOOLTIP);

	Widget_GameText_drawColored(68, 151, c->x + 5, c->y + 47, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
	Widget_Text_drawNumberColored(Data_CityInfo.ratingPeace, '@', " ",
		c->x + 140, c->y + 47, FONT_SMALL_PLAIN, COLOR_TOOLTIP);

	Widget_GameText_drawColored(68, 152, c->x + 5, c->y + 61, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
	Widget_Text_drawNumberColored(Data_CityInfo.ratingFavor, '@', " ",
		c->x + 140, c->y + 61, FONT_SMALL_PLAIN, COLOR_TOOLTIP);
}
