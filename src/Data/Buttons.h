#ifndef DATA_BUTTONS_H
#define DATA_BUTTONS_H

#include "Types.h"

enum {
	ImageButton_Normal = 4,
	ImageButton_Scroll = 6,
	ImageButton_Build = 2
};

typedef struct {
	short xOffset;
	short yOffset;
	short width;
	short height;
	short buttonType;
	short graphicCollection;
	short graphicIdOffset;
	void (*leftClickHandler)(int param1, int param2);
	void (*rightClickHandler)(int param1, int param2);
	int parameter1;
	int parameter2;
	char enabled;
	// state
	char pressed;
	char focused;
	TimeMillis pressedSince;
} ImageButton;

enum {
	CustomButton_Immediate = 1,
	CustomButton_OnMouseUp = 3,
};

typedef struct {
	short xStart;
	short yStart;
	short xEnd;
	short yEnd;
	int type;
	void (*leftClickHandler)(int param1, int param2);
	void (*rightClickHandler)(int param1, int param2);
	int parameter1;
	int parameter2;
} CustomButton;

typedef struct {
	short xOffset;
	short yOffset;
	short graphicId;
	short size;
	void (*leftClickHandler)(int param1, int param2);
	int parameter1;
	int parameter2;
	char field_C;
	char field_D;
	char field_E;
	char field_F;//always 4 in game
} ArrowButton;

typedef struct {
	short yStart;
	short textNumber;
	void (*leftClickHandler)(int param);
	int parameter;
} MenuItem;

typedef struct {
	short xStart;
	short xEnd;
	short yStart;
	short textGroup;
	MenuItem *items;
	int numItems;
} MenuBarItem;

#endif
