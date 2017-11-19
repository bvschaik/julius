#include "keyboardinput.h"

#include <ui>

#include "data/keyboardinput.hpp"

enum
{
    CursorPosition_InString = 0,
    CursorPosition_AtEnd = 1,
    CursorPosition_AtMax = 2,
};

static void moveLeft(uint8_t *start, uint8_t *end)
{
    while (start < end)
    {
        start[0] = start[1];
        start++;
    }
    *start = 0;
}

static void moveRight(uint8_t *start, uint8_t *end)
{
    end[1] = 0;
    while (end > start)
    {
        end--;
        end[1] = end[0];
    }
}

static void determineCursorPosition()
{
    Data_KeyboardInput.positionType = CursorPosition_InString;
    if (Data_KeyboardInput.current > 0)
    {
        struct Data_KeyboardInputLine *current =
                &Data_KeyboardInput.lines[Data_KeyboardInput.current];
        int width = Widget::Text::getWidth(current->text, current->font);
        if (current->cursorPosition >= current->length)
        {
            current->cursorPosition = current->length;
            Data_KeyboardInput.positionType = CursorPosition_AtEnd;
        }
        if (current->cursorPosition >= current->maxLength)
        {
            current->cursorPosition = current->maxLength;
            Data_KeyboardInput.positionType = CursorPosition_AtMax;
        }
        if (width >= current->boxWidth)
        {
            Data_KeyboardInput.positionType = CursorPosition_AtMax;
        }
        if (current->cursorPosition < 0)
        {
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
    if (Data_KeyboardInput.current > 0)
    {
        struct Data_KeyboardInputLine *current =
                &Data_KeyboardInput.lines[Data_KeyboardInput.current];

        determineCursorPosition();
        if (Data_KeyboardInput.positionType != CursorPosition_AtMax)
        {
            if (!Data_KeyboardInput.isInsert)
            {
                current->text[current->cursorPosition++] = value;
                if (Data_KeyboardInput.positionType == CursorPosition_AtEnd)
                {
                    current->length++;
                    current->text[current->cursorPosition] = 0;
                }
            }
            else if (current->length < current->maxLength)
            {
                moveRight(
                    &current->text[current->cursorPosition],
                    &current->text[current->length + 1]);
                current->text[current->cursorPosition++] = value;
                current->length++;
            }
        }
    }
}

void KeyboardInput_initTextField(int inputId, uint8_t *text, int maxLength,
                                 int textboxWidth, int allowPunctuation, font_t font)
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
    if (inputId)
    {
        struct Data_KeyboardInputLine *current =
                &Data_KeyboardInput.lines[Data_KeyboardInput.current];
        current->length = 0;
        for (int i = 0; i <= current->maxLength && current->text[i]; i++)
        {
            ++current->length;
        }
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
    if (current->cursorPosition > 0)
    {
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
    if (current->cursorPosition > 0)
    {
        current->cursorPosition--;
    }
}

void KeyboardInput_right()
{
    struct Data_KeyboardInputLine *current =
            &Data_KeyboardInput.lines[Data_KeyboardInput.current];
    if (current->cursorPosition < current->length)
    {
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

static unsigned char unicodeToCp1252(int c)
{
    if (c == 0x152)   // OE
    {
        return 140;
    }
    else if (c == 0x153)     // oe
    {
        return 156;
    }
    else if (c <= 0xff)
    {
        // ascii + ISO-8859-1
        return (unsigned char) c;
    }
    return 0;
}

void KeyboardInput_character(int unicode)
{
    if (Data_KeyboardInput.current <= 0)
    {
        return;
    }
    struct Data_KeyboardInputLine *current =
            &Data_KeyboardInput.lines[Data_KeyboardInput.current];

    int add = 0;
    unsigned char c = unicodeToCp1252(unicode);
    // TODO correct ranges (original ones are from CP437 / dos extended ascii)
    if (c == ' ' || c == '-')
    {
        add = 1;
    }
    else if (c >= '0' && c <= '9')
    {
        add = 1;
    }
    else if (c >= 'a' && c <= 'z')
    {
        add = 1;
    }
    else if (c >= 'A' && c <= 'Z')
    {
        add = 1;
    }
    else if (c >= 128 && c <= 154)
    {
        add = 1;
    }
    else if (c >= 160 && c <= 167)
    {
        add = 1;
    }
    else if (c == 225)
    {
        add = 1;
    }
    else if (current->allowPunctuation)
    {
        if (c == ',' || c == '.' || c == '?' || c == '!')
        {
            add = 1;
        }
    }
    if (add)
    {
        addCharacter((char) c);
        determineCursorPosition();
    }
}
