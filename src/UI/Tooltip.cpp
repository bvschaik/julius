#include "Tooltip.h"

#include "Window.h"

#include "../Graphics.h"
#include "../Language.h"
#include "../String.h"
#include "../Time.h"
#include "../Widget.h"

#include "../Data/CityInfo.h"
#include "../Data/Constants.h"
#include "../Data/Mouse.h"
#include "../Data/Screen.h"

#include <string.h>
#include <stdio.h>

static void resetTooltip(struct TooltipContext *c);
static void drawTooltip(struct TooltipContext *c);
static void drawButtonTooltip(struct TooltipContext *c);
static void drawOverlayTooltip(struct TooltipContext *c);
static void drawSenateTooltip(struct TooltipContext *c);

static TimeMillis lastUpdate;
static int infoId;

void UI_Tooltip_resetTimer()
{
	lastUpdate = Time_getMillis();
	infoId = 0;
}

void UI_Tooltip_handle(void (*func)(struct TooltipContext *))
{
	struct TooltipContext tooltipContext = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	if (func) {
		func(&tooltipContext);
	}
	if (tooltipContext.type != TooltipType_None) {
		drawTooltip(&tooltipContext);
		resetTooltip(&tooltipContext);
	}
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
	const char *text = Language_getString(c->textGroup, c->textId);
	Widget_RichText_setFonts(Font_SmallPlain, Font_SmallPlain);

	c->width = 200;
	int lines = Widget_RichText_draw(text, c->x + 5, c->y + 5,
		c->width - 5, 30, 1);
	if (lines > 2) {
		c->width = 300;
		lines = Widget_RichText_draw(text, c->x + 5, c->y + 5,
			c->width - 5, 30, 1);
	}
	c->height = 16 * lines + 10;

	if (Data_Mouse.x < Data_Screen.offset640x480.x + c->width + 100) {
		c->x = Data_Mouse.x + 20 + c->xOffset;
	} else {
		c->x = Data_Mouse.x - c->width - 20;
	}
	if (Data_Mouse.y < Data_Screen.offset640x480.y + 200) {
		c->y = Data_Mouse.y + 50;
	} else {
		c->y = Data_Mouse.y - 72;
	}

	Graphics_drawRect(c->x, c->y, c->width, c->height, Color_Black);
	Graphics_fillRect(c->x + 1, c->y + 1, c->width - 2, c->height - 2, Color_White);
	Widget_RichText_drawColored(text, c->x + 5, c->y + 7,
		c->width - 5, lines, Color_Tooltip);
/*
      switch ( window_id )
      {
        case 7u:
          if ( button_y < screen_640x480_y + 432 )
          {
            switch ( currentAdvisor )
            {
              case 1:
                tooltip_y = button_y - 74;
                break;
              case 5:
                tooltip_y = button_y - 54;
                break;
              case 6:
                tooltip_y = button_y - 58;
                break;
              default:
                tooltip_y = button_y - 64;
                break;
            }
          }
          else
          {
            tooltip_y = screen_640x480_y + 432;
          }
          break;
        case 45u:
          tooltip_y = button_y - 52;
          break;
        case 42u:
          if ( v1 )
            tooltip_y = button_y + 40;
          else
            tooltip_y = button_y - 72;
          tooltip_x = button_x - tooltip_width / 2 - 10;
          break;
        default:
          if ( v1 )
            tooltip_y = button_y + 40;
          else
            tooltip_y = button_y - 62;
          break;
      }
*/
}

static char tmpString[1000];
static void drawOverlayTooltip(struct TooltipContext *c)
{
	const char *text = Language_getString(c->textGroup, c->textId);
	if (c->hasNumericPrefix) {
		int offset = String_intToString(tmpString, c->numericPrefix, 0);
		strcpy(&tmpString[offset], text);
		text = tmpString;
	}
	Widget_RichText_setFonts(Font_SmallPlain, Font_SmallPlain);

	c->width = 200;
	int lines = Widget_RichText_draw(text, c->x + 5, c->y + 5,
		c->width - 5, 30, 1);
	if (lines > 2) {
		c->width = 300;
		lines = Widget_RichText_draw(text, c->x + 5, c->y + 5,
			c->width - 5, 30, 1);
	}
	c->height = 16 * lines + 10;

	if (Data_Mouse.x < c->width + 20) {
		c->x = Data_Mouse.x + 20;
	} else {
		c->x = Data_Mouse.x - c->width - 20;
	}
	if (Data_Mouse.y < 200) {
		c->y = Data_Mouse.y + 50;
	} else {
		c->y = Data_Mouse.y - 72;
	}

	Graphics_drawRect(c->x, c->y, c->width, c->height, Color_Black);
	Graphics_fillRect(c->x + 1, c->y + 1, c->width - 2, c->height - 2, Color_White);
	Widget_RichText_drawColored(text, c->x + 5, c->y + 7,
		c->width - 5, lines, Color_Tooltip);
}

static void drawSenateTooltip(struct TooltipContext *c)
{
	c->width = 180;
	c->height = 80;
	if (Data_Mouse.x < c->width + 20) {
		c->x = Data_Mouse.x + 20;
	} else {
		c->x = Data_Mouse.x - c->width - 20;
	}
	if (Data_Mouse.y < 200) {
		c->y = Data_Mouse.y + 10;
	} else {
		c->y = Data_Mouse.y - 32;
	}
	
	Graphics_drawRect(c->x, c->y, c->width, c->height, Color_Black);
	Graphics_fillRect(c->x + 1, c->y + 1, c->width - 2, c->height - 2, Color_White);
	
	// unemployment
	Widget_GameText_drawColored(68, 148, c->x + 5, c->y + 5, Font_SmallPlain, Color_Tooltip);
	Widget_Text_drawNumberColored(Data_CityInfo.unemploymentPercentage, '@', "%",
		c->x + 140, c->y + 5, Font_SmallPlain, Color_Tooltip);
	
	// ratings
	Widget_GameText_drawColored(68, 149, c->x + 5, c->y + 19, Font_SmallPlain, Color_Tooltip);
	Widget_Text_drawNumberColored(Data_CityInfo.ratingCulture, '@', " ",
		c->x + 140, c->y + 19, Font_SmallPlain, Color_Tooltip);

	Widget_GameText_drawColored(68, 150, c->x + 5, c->y + 33, Font_SmallPlain, Color_Tooltip);
	Widget_Text_drawNumberColored(Data_CityInfo.ratingProsperity, '@', " ",
		c->x + 140, c->y + 33, Font_SmallPlain, Color_Tooltip);

	Widget_GameText_drawColored(68, 151, c->x + 5, c->y + 47, Font_SmallPlain, Color_Tooltip);
	Widget_Text_drawNumberColored(Data_CityInfo.ratingPeace, '@', " ",
		c->x + 140, c->y + 47, Font_SmallPlain, Color_Tooltip);

	Widget_GameText_drawColored(68, 152, c->x + 5, c->y + 61, Font_SmallPlain, Color_Tooltip);
	Widget_Text_drawNumberColored(Data_CityInfo.ratingFavor, '@', " ",
		c->x + 140, c->y + 61, Font_SmallPlain, Color_Tooltip);
}
