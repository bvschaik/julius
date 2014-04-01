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
	struct Data_KeyboardInputLine *current =
		&Data_KeyboardInput.lines[Data_KeyboardInput.current];
	moveLeft(&current->text[current->cursorPosition], &current->text[current->length]);
	determineCursorPosition();
}

static void addCharacter(char value)
{
	determineCursorPosition();
	// TODO
}

void KeyboardInput_initTextField(int inputId, char *text, int maxLength,
	int textboxWidth, int allowPunctuation)
{
	Data_KeyboardInput.lines[inputId].text = text;
	Data_KeyboardInput.lines[inputId].length = 0;
	Data_KeyboardInput.lines[inputId].maxLength = maxLength;
	//Data_KeyboardInput.lines[inputId].font = font;
	// TODO
}

void KeyboardInput_initInput(int inputId)
{
	Data_KeyboardInput.current = inputId;
	
	struct Data_KeyboardInputLine *current =
		&Data_KeyboardInput.lines[Data_KeyboardInput.current];
	current->length = 0;
	while (current->length <= current->maxLength && current->text[current->length]) {
		++current->length;
	}
}

void KeyboardInput_return()
{
	Data_KeyboardInput.accepted = 1;
}

void KeyboardInput_backspace()
{
	struct Data_KeyboardInputLine *current =
		&Data_KeyboardInput.lines[Data_KeyboardInput.current];
	if (current->cursorPosition > 0) {
		current->cursorPosition--;
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
	struct Data_KeyboardInputLine *current =
		&Data_KeyboardInput.lines[Data_KeyboardInput.current];
	if (current->cursorPosition > 0) {
		current->cursorPosition--;
	}
}

void KeyboardInput_right()
{
	struct Data_KeyboardInputLine *current =
		&Data_KeyboardInput.lines[Data_KeyboardInput.current];
	if (current->cursorPosition < current->length) {
		current->cursorPosition++;
	}
}

void KeyboardInput_home()
{
	Data_KeyboardInput.lines[Data_KeyboardInput.current].cursorPosition = 0;
}

void KeyboardInput_end()
{
	struct Data_KeyboardInputLine *current =
		&Data_KeyboardInput.lines[Data_KeyboardInput.current];
	current->cursorPosition = current->length;
}
