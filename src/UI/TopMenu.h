#ifndef UI_TOPMENU_H
#define UI_TOPMENU_H

#include "Tooltip.h"

#include "input/mouse.h"

void UI_TopMenu_initFromSettings();
void UI_TopMenu_drawBackground();
void UI_TopMenu_drawBackgroundIfNecessary();
void UI_TopMenu_drawForeground();
void UI_TopMenu_handleMouse(const mouse *m);
int UI_TopMenu_handleMouseWidget(const mouse *m);
int UI_TopMenu_getTooltipText(struct TooltipContext *c);

#endif
