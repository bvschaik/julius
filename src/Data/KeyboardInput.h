#ifndef DATA_KEYBOARDINPUT_H
#define DATA_KEYBOARDINPUT_H

struct Data_KeyboardInputLine {
	int cursorPosition;
	int length;
	int maxLength;
	char *text;
};

extern struct Data_KeyboardInput {
	struct Data_KeyboardInputLine lines[6];
	int current;
	int isInsert;
	int accepted;
} Data_KeyboardInput;

#endif
