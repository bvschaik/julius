#ifndef DATA_MOUSE_H
#define DATA_MOUSE_H

struct Data_Mouse_Button {
	int isDown;
	int wentDown;
	int wentUp;
};

extern struct _Data_Mouse {
	// filled from outside game
	int x;
	int y;
	int leftDown;
	int rightDown;
	int scrollUp;
	int scrollDown;
	int isInsideWindow;
	// filled by game
	struct Data_Mouse_Button left;
	struct Data_Mouse_Button right;
} Data_Mouse;

#endif
