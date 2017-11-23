#ifndef WIDGET_H
#define WIDGET_H

#include "data/buttons.hpp"

#include "graphics/color.h"
#include "graphics/font.h"
#include "graphics/mouse.h"

#include <stdint.h>

struct _InputCursor
{
    int capture;
    int seen;
    int position;
    int width;
    int visible;
    time_millis updated;
    int xOffset;
    int yOffset;
};

extern _InputCursor inputCursor;
extern const int map_charToFontGraphic[];
extern uint8_t tmpLine[200];

int getWordWidth(const unsigned char *str, font_t font, int *outNumChars);
void numberToString(uint8_t *str, int value, char prefix, const char *postfix);
int drawCharacter(font_t font, unsigned int c, int x, int y, int lineHeight, color_t color);

int Widget_GameText_draw(int group, int number, int xOffset, int yOffset, font_t font);
int Widget_GameText_drawColored(int group, int number, int xOffset, int yOffset, font_t font, color_t color);

void Widget_GameText_drawCentered(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font);
void Widget_GameText_drawCenteredColored(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font, color_t color);

int Widget_GameText_drawNumberWithDescription(int group, int number, int amount, int xOffset, int yOffset, font_t font);

int Widget_GameText_drawYear(int year, int xOffset, int yOffset, font_t font);
int Widget_GameText_drawYearColored(int year, int xOffset, int yOffset, font_t font, color_t color);

int Widget_GameText_drawYearNoSpacing(int year, int xOffset, int yOffset, font_t font);

int Widget_GameText_drawMultiline(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font);

int Widget_GameText_getWidth(int group, int number, font_t font);
int Widget_GameText_getDrawWidth(int group, int number, font_t font);

void Widget_RichText_setFonts(font_t normalFont, font_t linkFont);
// returns total number of lines
int Widget_RichText_draw(const char *text, int xOffset, int yOffset,
                         int boxWidthPixels, int boxHeightLines, int measureOnly);
int Widget_RichText_drawColored(const uint8_t *str, int xOffset, int yOffset,
                                int boxWidth, int heightLines, color_t color);
int Widget_RichText_getClickedLink(const mouse *m);
void Widget_RichText_clearLinks();
void Widget_RichText_scroll(int isDown, int numLines);
void Widget_RichText_reset(int scrollPosition);
void Widget_RichText_save();
void Widget_RichText_restore();
void Widget_RichText_drawScrollbar();
void Widget_RichText_drawScrollbarDot();
int Widget_RichText_getScrollPosition();
int Widget_RichText_handleScrollbar(const mouse *m);
int Widget_RichText_init(const char *str, int xText, int yText, int widthBlocks, int heightBlocks, int adjustWidthOnNoScroll);

#endif
