#include "KeyboardInput.h"

#include "Data/KeyboardInput.h"

static void moveLeft(char *start, char *end)
{
	while (start < end) {
		start[0] = start[1];
		start++;
	}
	*start = 0;
}

static void moveRight(char *start, char *end)
{
	end[1] = 0;
	while (end > start) {
		end--;
		end[1] = end[0];
	}
}

static void determineCursorPosition()
{
	// TODO
}

static void removeCurrentCharacter()
{
	struct Data_KeyboardInputLine *current = Data_KeyboardInput.current;
	moveLeft(&current->text[current->cursorPosition], &current->text[current->length]);
	determineCursorPosition();
}

static void addCharacter(char value)
{
	determineCursorPosition();
	// TODO
}

void KeyboardInput_return()
{
	Data_KeyboardInput.accepted = 1;
}

void KeyboardInput_backspace()
{
	if (Data_KeyboardInput.current->cursorPosition > 0) {
		Data_KeyboardInput.current->cursorPosition--;
		removeCurrentCharacter();
	}
}

void KeyboardInput_delete()
{
	removeCurrentCharacter();
}

void KeyboardInput_insert()
{
	Data_KeyboardInput.isInsert ^= 1;
}

void KeyboardInput_left()
{
	if (Data_KeyboardInput.current->cursorPosition > 0) {
		Data_KeyboardInput.current->cursorPosition--;
	}
}

void KeyboardInput_right()
{
	if (Data_KeyboardInput.current->cursorPosition < Data_KeyboardInput.current->length) {
		Data_KeyboardInput.current->cursorPosition++;
	}
}

void KeyboardInput_home()
{
	Data_KeyboardInput.current->cursorPosition = 0;
}

void KeyboardInput_end()
{
	Data_KeyboardInput.current->cursorPosition = Data_KeyboardInput.current->length;
}
