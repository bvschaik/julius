#include "widget_text.h"

#include "graphics/image.h"
#include "graphics.h"

#include <game>
#include <data>

void Widget::Text::captureCursor(int cursor_position)
{
    inputCursor.capture = 1;
    inputCursor.seen = 0;
    inputCursor.position = 0;
    inputCursor.width = 0;
    inputCursor.cursor_position = cursor_position;
}

int Widget::Text::getWidth(const char *str, font_t font)
{
    int spaceWidth;
    int letterSpacing;

    if (font == FONT_LARGE_PLAIN || font == FONT_LARGE_BLACK || font == FONT_LARGE_BROWN)
    {
        spaceWidth = 10;
        letterSpacing = 1;
    }
    else if (font == FONT_SMALL_PLAIN)
    {
        spaceWidth = 4;
        letterSpacing = 1;
    }
    else if (font == FONT_NORMAL_PLAIN)
    {
        spaceWidth = 6;
        letterSpacing = 1;
    }
    else
    {
        spaceWidth = 6;
        letterSpacing = 0;
    }

    int maxlen = 10000;
    int width = 0;
    int graphicBase = image_group(GROUP_FONT);
    while (*str && maxlen > 0)
    {
        if (*str == ' ')
        {
            width += spaceWidth;
        }
        else
        {
            int graphicOffset = map_charToFontGraphic[*str];
            if (graphicOffset)
            {
                int graphicId = graphicBase + font + graphicOffset - 1;
                width += letterSpacing + image_get(graphicId)->width;
            }
        }
        str++;
        maxlen--;
    }
    return width;
}


int Widget::Text::drawMultiline(const char *str, int xOffset, int yOffset, int boxWidth, font_t font)
{
    int lineHeight;
    switch (font)
    {
    case FONT_LARGE_PLAIN:
    case FONT_LARGE_BLACK:
        lineHeight = 23;
        break;
    case FONT_LARGE_BROWN:
        lineHeight = 24;
        break;
    case FONT_SMALL_PLAIN:
        lineHeight = 9;
        break;
    case FONT_NORMAL_PLAIN:
        lineHeight = 11;
        break;
    default:
        lineHeight = 11;
        break;
    }

    int hasMoreCharacters = 1;
    int guard = 0;
    int y = yOffset;
    while (hasMoreCharacters)
    {
        if (++guard >= 100)
        {
            break;
        }
        // clear line
        for (int i = 0; i < 200; i++)
        {
            tmpLine[i] = 0;
        }
        int currentWidth = 0;
        int lineIndex = 0;
        while (hasMoreCharacters && currentWidth < boxWidth)
        {
            int wordNumChars;
            int wordWidth = getWordWidth((unsigned char*)str, font, &wordNumChars);
            currentWidth += wordWidth;
            if (currentWidth >= boxWidth)
            {
                if (currentWidth == 0)
                {
                    hasMoreCharacters = 0;
                }
            }
            else
            {
                for (int i = 0; i < wordNumChars; i++)
                {
                    tmpLine[lineIndex++] = *(str++);
                }
                if (!*str)
                {
                    hasMoreCharacters = 0;
                }
            }
        }
        Widget::Text::draw(tmpLine, xOffset, y, font, 0);
        y += lineHeight + 5;
    }
    return y - yOffset;
}

void Widget::Text::drawCursor(int xOffset, int yOffset, int isInsert)
{
    inputCursor.capture = 0;
    time_millis curr = time_get_millis();
    time_millis diff = curr - inputCursor.updated;
    if (!inputCursor.visible && diff >= 200)
    {
        inputCursor.visible = 1;
        inputCursor.updated = curr;
    }
    else if (inputCursor.visible && diff >= 400)
    {
        inputCursor.visible = 0;
        inputCursor.updated = curr;
    }
    if (inputCursor.visible)
    {
        if (isInsert)
        {
            Graphics_drawLine(
                xOffset + inputCursor.xOffset - 3, yOffset + inputCursor.yOffset - 3,
                xOffset + inputCursor.xOffset + 1, yOffset + inputCursor.yOffset - 3,
                COLOR_WHITE);
            Graphics_drawLine(
                xOffset + inputCursor.xOffset - 1, yOffset + inputCursor.yOffset - 3,
                xOffset + inputCursor.xOffset - 1, yOffset + inputCursor.yOffset + 13,
                COLOR_WHITE);
            Graphics_drawLine(
                xOffset + inputCursor.xOffset - 3, yOffset + inputCursor.yOffset + 14,
                xOffset + inputCursor.xOffset + 1, yOffset + inputCursor.yOffset + 14,
                COLOR_WHITE);
        }
        else
        {
            Graphics_fillRect(
                xOffset + inputCursor.xOffset, yOffset + inputCursor.yOffset + 14,
                inputCursor.width, 2, COLOR_WHITE);
        }
    }
}

void Widget::Text::drawCentered(const char *str, int x, int y, int boxWidth, font_t font, color_t color)
{
    int offset = (boxWidth - Widget::Text::getWidth(str, font)) / 2;
    if (offset < 0)
    {
        offset = 0;
    }
    Widget::Text::draw(str, offset + x, y, font, color);
}

int Widget::Text::draw(const char *str, int x, int y, font_t font, color_t color)
{
    int letterSpacing;
    int lineHeight;
    int spaceWidth;
    switch (font)
    {
    case FONT_LARGE_PLAIN:
        spaceWidth = 8;
        lineHeight = 23;
        letterSpacing = 1;
        break;
    case FONT_LARGE_BLACK:
        spaceWidth = 8;
        lineHeight = 23;
        letterSpacing = 0;
        break;
    case FONT_LARGE_BROWN:
        spaceWidth = 8;
        lineHeight = 24;
        letterSpacing = 0;
        break;
    case FONT_SMALL_PLAIN:
        spaceWidth = 4;
        lineHeight = 9;
        letterSpacing = 1;
        break;
    case FONT_NORMAL_PLAIN:
        spaceWidth = 6;
        lineHeight = 11;
        letterSpacing = 1;
        break;
    default:
        spaceWidth = 6;
        lineHeight = 11;
        letterSpacing = 0;
        break;
    }

    int currentX = x;
    while (*str)
    {
        uint8_t c = *str;

        if (c == '_')
        {
            c = ' ';
        }

        if (c >= ' ')
        {
            int width = 0;
            int graphic = map_charToFontGraphic[c];
            if (graphic == 0)
            {
                width = spaceWidth;
            }
            else
            {
                width = letterSpacing + drawCharacter(font, c, currentX, y, lineHeight, color);
            }
            if (inputCursor.capture && inputCursor.position == inputCursor.cursor_position)
            {
                if (!inputCursor.seen)
                {
                    inputCursor.width = width;
                    inputCursor.xOffset = currentX - x;
                    inputCursor.seen = 1;
                }
            }
            currentX += width;
        }

        str++;
        inputCursor.position++;
    }
    if (inputCursor.capture && !inputCursor.seen)
    {
        inputCursor.width = 4;
        inputCursor.xOffset = currentX - x;
        inputCursor.seen = 1;
    }
    currentX += spaceWidth;
    return currentX - x;
}


int Widget::Text::drawNumber(int value, char prefix, const char *postfix, int xOffset, int yOffset, font_t font)
{
    char str[100];
    numberToString(str, value, prefix, postfix);
    return Widget::Text::draw(str, xOffset, yOffset, font, 0);
}

int Widget::Text::drawNumberColored(int value, char prefix, const char *postfix, int xOffset, int yOffset, font_t font, color_t color)
{
    char str[100];
    numberToString(str, value, prefix, postfix);
    return Widget::Text::draw(str, xOffset, yOffset, font, color);
}

int Widget::Text::drawMoney(int value, int xOffset, int yOffset, font_t font)
{
    char str[100];
    numberToString(str, value, '@', " Dn");
    return Widget::Text::draw(str, xOffset, yOffset, font, 0);
}

int Widget::Text::drawPercentage(int value, int xOffset, int yOffset, font_t font)
{
    char str[100];
    numberToString(str, value, '@', "%");
    return Widget::Text::draw(str, xOffset, yOffset, font, 0);
}

void Widget::Text::drawNumberCentered(int value, int xOffset, int yOffset, int boxWidth, font_t font)
{
    char str[100];
    numberToString(str, value, '@', " ");
    Widget::Text::drawCentered(str, xOffset, yOffset, boxWidth, font, 0);
}

void Widget::Text::drawNumberCenteredColored(int value, int xOffset, int yOffset, int boxWidth, font_t font, color_t color)
{
    char str[100];
    numberToString(str, value, '@', " ");
    Widget::Text::drawCentered(str, xOffset, yOffset, boxWidth, font, color);
}
