#ifndef WIDGET_H
#define WIDGET_H

#include "Data/Types.h"
#include "Data/Buttons.h"

int Widget_Text_draw(const char *str, int xOffset, int yOffset, Font font, Color color);

void Widget_Text_drawCentered(const char *str, int xOffset, int yOffset, int boxWidth, Font font, Color color);

int Widget_GameText_draw(int group, int number, int xOffset, int yOffset, Font font, Color color);

void Widget_GameText_drawCentered(int group, int number, int xOffset, int yOffset, int boxWidth, Font font, Color color);

int Widget_Text_drawNumber(int value, char prefix, const char *postfix, int xOffset, int yOffset, Font font, Color color);

void Widget_Text_drawNumberCentered(int value, char prefix, const char *postfix, int xOffset, int yOffset, int boxWidth, Font font, Color color);

int Widget_GameText_drawNumberWithDescription(int group, int number, int amount, int xOffset, int yOffset, Font font, Color color);

int Widget_GameText_drawYear(int year, int xOffset, int yOffset, Font font, Color color);

int Widget_Text_drawMultiline(const char *str, int xOffset, int yOffset, int boxWidth, Font font, Color color);

int Widget_GameText_drawMultiline(int group, int number, int xOffset, int yOffset, int boxWidth, Font font, Color color);

int Widget_GameText_getWidth(int group, int number, Font font);


void Widget_Panel_drawOuterPanel(int xOffset, int yOffset, int widthInBlocks, int heightInBlocks);

void Widget_Panel_drawUnborderedPanel(int xOffset, int yOffset, int widthInBlocks, int heightInBlocks);

void Widget_Panel_drawInnerPanel(int xOffset, int yOffset, int widthInBlocks, int heightInBlocks);

void Widget_Panel_drawInnerPanelBottom(int xOffset, int yOffset, int widthInBlocks);

void Widget_Panel_drawButtonBorder(int xOffset, int yOffset, int widthInPixels, int heightInPixels, int hasFocus);

void Widget_Panel_drawSmallLabelButton(int dummy, int xOffset, int yOffset, int width, int widthIsBlocks, int type);

void Widget_Panel_drawLargeLabelButton(int dummy, int xOffset, int yOffset, int widthInBlocks, int type);



void Widget_Button_drawArrowButtons(int xOffset, int yOffset, ArrowButton *buttons, int numButtons);

int Widget_Button_handleArrowButtons(int xOffset, int yOffset, ArrowButton *buttons, int numButtons);

//void Widget_Button_drawImageButtons();

int Widget_Button_handleCustomButtons(int xOffset, int yOffset, CustomButton *buttons, int numButtons, int *focusButtonId);


void Widget_Button_doNothing(int param1, int param2);

#endif
