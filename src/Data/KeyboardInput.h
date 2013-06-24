#ifndef DATA_KEYBOARDINPUT_H
#define DATA_KEYBOARDINPUT_H

struct Data_KeyboardInputLine {
	int cursorPosition;
	int length;
	int maxLength;
	char text[65];
};

struct Data_KeyboardInput {
	struct Data_KeyboardInputLine lines;
	struct Data_KeyboardInputLine *current;
	int isInsert;
	int accepted;
} Data_KeyboardInput;

#endif
