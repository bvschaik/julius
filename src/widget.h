#ifndef WIDGET_H
#define WIDGET_H

#include "data/buttons.hpp"

#include "graphics/color.h"
#include "graphics/font.h"
#include "graphics/mouse.h"

#include <stdint.h>

int Widget_GameText_draw(int group, int number, int xOffset, int yOffset, font_t font);
int Widget_GameText_drawColored(int group, int number, int xOffset, int yOffset, font_t font, color_t color);

void Widget_GameText_drawCentered(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font);
void Widget_GameText_drawCenteredColored(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font, color_t color);

int Widget_GameText_drawNumberWithDescription(int group, int number, int amount, int xOffset, int yOffset, font_t font);

int Widget_GameText_drawYear(int year, int xOffset, int yOffset, font_t font);
int Widget_GameText_drawYearColored(int year, int xOffset, int yOffset, font_t font, color_t color);

int Widget_GameText_drawYearNoSpacing(int year, int xOffset, int yOffset, font_t font);

int Widget_Text_drawMultiline(const uint8_t *str, int xOffset, int yOffset, int boxWidth, font_t font);

int Widget_GameText_drawMultiline(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font);

int Widget_Text_getWidth(const uint8_t *str, font_t font);
int Widget_GameText_getWidth(int group, int number, font_t font);
int Widget_GameText_getDrawWidth(int group, int number, font_t font);

void Widget_RichText_setFonts(font_t normalFont, font_t linkFont);
// returns total number of lines
int Widget_RichText_draw(const uint8_t *text, int xOffset, int yOffset,
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
int Widget_RichText_init(const uint8_t *str, int xText, int yText, int widthBlocks, int heightBlocks, int adjustWidthOnNoScroll);

void Widget_Panel_drawOuterPanel(int xOffset, int yOffset, int widthInBlocks, int heightInBlocks);

void Widget_Panel_drawUnborderedPanel(int xOffset, int yOffset, int widthInBlocks, int heightInBlocks);

void Widget_Panel_drawInnerPanel(int xOffset, int yOffset, int widthInBlocks, int heightInBlocks);

void Widget_Panel_drawInnerPanelBottom(int xOffset, int yOffset, int widthInBlocks);

void Widget_Panel_drawButtonBorder(int xOffset, int yOffset, int widthInPixels, int heightInPixels, int hasFocus);

void Widget_Panel_drawSmallLabelButton(int xOffset, int yOffset, int widthInBlocks, int type);

void Widget_Panel_drawLargeLabelButton(int xOffset, int yOffset, int widthInBlocks, int type);



void Widget_Button_drawArrowButtons(int xOffset, int yOffset, ArrowButton *buttons, int numButtons);

int Widget_Button_handleArrowButtons(int xOffset, int yOffset, ArrowButton *buttons, int numButtons);

void Widget_Button_drawImageButtons(int xOffset, int yOffset, ImageButton *buttons, int numButtons);

int Widget_Button_handleImageButtons(int xOffset, int yOffset, ImageButton *buttons, int numButtons, int *focusButtonId);

int Widget_Button_handleCustomButtons(int xOffset, int yOffset, CustomButton *buttons, int numButtons, int *focusButtonId);


void Widget_Button_doNothing(int param1, int param2);

void Widget_Menu_drawMenuBar(MenuBarItem *items, int numItems);
void Widget_Menu_drawSubMenu(MenuBarItem *menu, int focusSubMenu);

int Widget_Menu_handleMenuBar(const mouse *m, MenuBarItem *items, int numItems, int *focusMenuId);
int Widget_Menu_handleMenuItem(const mouse *m, MenuBarItem *menu, int *focusSubMenuId);

#endif
