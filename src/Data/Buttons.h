#ifndef DATA_BUTTONS_H
#define DATA_BUTTONS_H

/*
struct Data_Buttons_ArrowButton {
	int foo;
};
extern struct Data_Buttons_Arrow {
	struct Data_Buttons_ArrowButton difficultyOptions[3];
} Data_Buttons_Arrow;
*/
typedef struct {
	short xOffset;
	short yOffset;
	short width;
	short height;
	short field_8;
	short graphicCollection;
	short graphicIdOffset;
	void (*leftClickHandler)(int param1, int param2);
	void (*rightClickHandler)(int param1, int param2);
	char enabled;
	char hasClickEffect;
	char hasFocus;
	char field_1B;
	int parameter1;
	int parameter2;
} ImageButton;

typedef struct {
	short xStart;
	short yStart;
	short xEnd;
	short yEnd;
	void (*leftClickHandler)(int param1, int param2);
	void (*rightClickHandler)(int param1, int param2);
	//char field_10;
	//char field_11;
	//char field_12;
	//char field_13;
	int type;
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
