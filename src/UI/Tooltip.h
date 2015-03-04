#ifndef UI_TOOLTIP_H
#define UI_TOOLTIP_H

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
	int type;
	int priority;
	int x;
	int y;
	int width;
	int height;
	int textGroup;
	int textId;
	int hasNumericPrefix;
	int numericPrefix;
};


void UI_Tooltip_resetTimer();

void UI_Tooltip_handle(void (*func)(struct TooltipContext *));

#endif
