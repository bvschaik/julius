#ifndef UI_TOOLTIP_H
#define UI_TOOLTIP_H

#include "graphics/mouse.hpp"

enum {
	TooltipType_None = 0,
	TooltipType_Button = 1,
	TooltipType_Overlay = 2,
	TooltipType_Senate = 3
};

enum {
	TooltipPriority_Low = 1,
	TooltipPriority_High = 2
};

struct TooltipContext {
    // read-only
    const int mouse_x;
    const int mouse_y;
	int type;
	int priority;
	int textGroup;
	int textId;
	int hasNumericPrefix;
	int numericPrefix;
};


void UI_Tooltip_resetTimer();

void UI_Tooltip_handle(const mouse *m, void (*func)(struct TooltipContext *));

#endif
