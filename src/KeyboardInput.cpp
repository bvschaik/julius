#include "KeyboardInput.h"

#include "Widget.h"
#include "Data/KeyboardInput.h"

enum {
	CursorPosition_InString = 0,
	CursorPosition_AtEnd = 1,
	CursorPosition_AtMax = 2,
};

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
	Data_KeyboardInput.positionType = CursorPosition_InString;
	if (Data_KeyboardInput.current > 0) {
		struct Data_KeyboardInputLine *current =
			&Data_KeyboardInput.lines[Data_KeyboardInput.current];
		int width = Widget_Text_getWidth(current->text, current->font);
		if (current->cursorPosition >= current->length) {
			current->cursorPosition = current->length;
			Data_KeyboardInput.positionType = CursorPosition_AtEnd;
		}
		if (current->cursorPosition >= current->maxLength) {
			current->cursorPosition = current->maxLength;
			Data_KeyboardInput.positionType = CursorPosition_AtMax;
		}
		if (width >= current->boxWidth) {
			Data_KeyboardInput.positionType = CursorPosition_AtMax;
		}
		if (current->cursorPosition < 0) {
			current->cursorPosition = 0;
		}
	}
}

static void removeCurrentCharacter()
{
	struct Data_KeyboardInputLine *current =
		&Data_KeyboardInput.lines[Data_KeyboardInput.current];
	moveLeft(&current->text[current->cursorPosition], &current->text[current->length]);
	current->length--;
	determineCursorPosition();
}

static void addCharacter(char value)
{
	if (Data_KeyboardInput.current > 0) {
		struct Data_KeyboardInputLine *current =
			&Data_KeyboardInput.lines[Data_KeyboardInput.current];
		
		determineCursorPosition();
		if (Data_KeyboardInput.positionType != CursorPosition_AtMax) {
			if (!Data_KeyboardInput.isInsert) {
				current->text[current->cursorPosition++] = value;
				if (Data_KeyboardInput.positionType == CursorPosition_AtEnd) {
					current->length++;
				}
			} else if (current->length < current->maxLength) {
				moveRight(
					&current->text[current->cursorPosition],
					&current->text[current->length + 1]);
				current->text[current->cursorPosition++] = value;
				current->length++;
			}
		}
	}
}

void KeyboardInput_initTextField(int inputId, char *text, int maxLength,
	int textboxWidth, int allowPunctuation, Font font)
{
	Data_KeyboardInput.lines[inputId].font = font;
	Data_KeyboardInput.lines[inputId].maxLength = maxLength;
	Data_KeyboardInput.lines[inputId].length = 0;
	Data_KeyboardInput.lines[inputId].boxWidth = textboxWidth;
	Data_KeyboardInput.lines[inputId].allowPunctuation = allowPunctuation;
	Data_KeyboardInput.lines[inputId].cursorPosition = 0;
	Data_KeyboardInput.lines[inputId].text = text;
	
	Data_KeyboardInput.positionType = CursorPosition_InString;
}

void KeyboardInput_initInput(int inputId)
{
	Data_KeyboardInput.current = inputId;
	
	struct Data_KeyboardInputLine *current =
		&Data_KeyboardInput.lines[Data_KeyboardInput.current];
	current->length = 0;
	for (int i = 0; i <= current->maxLength && current->text[i]; i++) {
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

unsigned char unicodeToCp437(int c)
{
	if (c < 0x80) {
		// ascii
		return (unsigned char) c;
	}
	switch (c) {
		case 0xc7: return 128; // C-cedilla
		case 0xfc: return 129; // u-umlaut
		case 0xe9: return 130; // e-acute
		case 0xe2: return 131; // a-hat
		case 0xe4: return 132; // a-umlaut
		case 0xe0: return 133; // a-grave
		case 0xe5: return 134; // a-ring
		case 0xe7: return 135; // c-cedilla
		case 0xea: return 136; // e-hat
		case 0xeb: return 137; // e-umlaut
		case 0xe8: return 138; // e-grave
		case 0xef: return 139; // i-umlaut
		case 0xee: return 140; // i-hat
		case 0xec: return 141; // i-grave
		case 0xc4: return 142; // A-umlaut
		case 0xc5: return 143; // A-ring
		case 0xc9: return 144; // E-acute
		case 0xe6: return 145; // ae
		case 0xc6: return 146; // AE
		case 0xf4: return 147; // o-hat
		case 0xf6: return 148; // o-umlaut
		case 0xf2: return 149; // o-grave
		case 0xfb: return 150; // u-hat
		case 0xf9: return 151; // u-grave
		case 0xff: return 152; // y-umlaut
		case 0xd6: return 153; // O-umlaut
		case 0xdc: return 154; // U-umlaut

		case 0xe1: return 160; // a-acute
		case 0xed: return 161; // i-acute
		case 0xf3: return 162; // o-acute
		case 0xfa: return 163; // u-acute
		case 0xf1: return 164; // n-tilde
		case 0xd1: return 165; // N-tilde
		case 0xaa: return 166; // ^a
		case 0xb0: return 167; // ^ring

		case 0xdf: return 225; // ss
	}
	return 0;
}

void KeyboardInput_character(int unicode)
{
	if (Data_KeyboardInput.current <= 0) {
		return;
	}
	struct Data_KeyboardInputLine *current =
		&Data_KeyboardInput.lines[Data_KeyboardInput.current];

	int add = 0;
	unsigned char c = unicodeToCp437(unicode);
	if (c == ' ' || c == '-') {
		add = 1;
	} else if (c >= '0' && c <= '9') {
		add = 1;
	} else if (c >= 'a' && c <= 'z') {
		add = 1;
	} else if (c >= 'A' && c <= 'Z') {
		add = 1;
	} else if (c >= 128 && c <= 154) {
		add = 1;
	} else if (c >= 160 && c <= 167) {
		add = 1;
	} else if (current->allowPunctuation) {
		if (c == ',' || c == '.' || c == '?' || c == '!') {
			add = 1;
		}
	}
	if (add) {
		addCharacter((char) c);
		determineCursorPosition();
	}
}
