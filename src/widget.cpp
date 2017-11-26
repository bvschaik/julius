#include "widget.h"

#include "data/constants.hpp"
#include "data/keyboardinput.hpp"

#include "core/calc.h"
#include "graphics.h"
#include "ui/window.h"

#include <ui>
#include <game>
#include <data>

#include "core/lang.h"
#include "core/string.h"
#include "core/time.h"
#include "graphics/image.h"

_InputCursor inputCursor;

const int map_charToFontGraphic[] =
{
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01,
    0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x3F, 0x40, 0x00, 0x00, 0x41, 0x00, 0x4A, 0x43, 0x44, 0x42, 0x46, 0x4E, 0x45, 0x4F, 0x4D,
    0x3E, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x48, 0x49, 0x00, 0x47, 0x00, 0x4B,
    0x00, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x00, 0x63, 0x00, 0x00, 0x50,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x65, 0x61, 0x56, 0x54, 0x51, 0x53, 0x01, 0x67, 0x81, 0x55, 0x57, 0x59, 0x6E, 0x5D, 0x69, 0x1B,
    0x6A, 0x67, 0x6D, 0x60, 0x5D, 0x5F, 0x64, 0x63, 0x19, 0x7B, 0x6B, 0x00, 0x6F, 0x00, 0x00, 0x00,
    0x52, 0x7F, 0x5E, 0x62, 0x66, 0x6C, 0x01, 0x0F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
    0x72, 0x70, 0x71, 0x71, 0x69, 0x83, 0x6D, 0x65, 0x74, 0x6A, 0x73, 0x73, 0x77, 0x75, 0x76, 0x76,
    0x00, 0x6C, 0x7A, 0x78, 0x79, 0x79, 0x7B, 0x00, 0x84, 0x7E, 0x7C, 0x7D, 0x6B, 0x33, 0x00, 0x68,
    0x53, 0x52, 0x54, 0x51, 0x51, 0x85, 0x67, 0x65, 0x57, 0x56, 0x58, 0x55, 0x5B, 0x5A, 0x5C, 0x59,
    0x00, 0x66, 0x5F, 0x5E, 0x60, 0x60, 0x5D, 0x00, 0x86, 0x63, 0x62, 0x64, 0x61, 0x19, 0x00, 0x19,
};

char tmpLine[200];

int Widget_GameText_getWidth(int group, int number, font_t font)
{
    const char *str = lang_get_string(group, number);
    return Widget::Text::getWidth(str, font);
}

static int getSpaceWidth(font_t font)
{
    if (font == FONT_LARGE_PLAIN || font == FONT_LARGE_BLACK || font == FONT_LARGE_BROWN)
    {
        return 10;
    }
    else if (font == FONT_SMALL_PLAIN)
    {
        return 4;
    }
    else
    {
        return 6;
    }
}

int Widget_GameText_getDrawWidth(int group, int number, font_t font)
{
    const char *str = lang_get_string(group, number);
    return Widget::Text::getWidth(str, font) + getSpaceWidth(font);
}

static int getCharacterWidth(unsigned char c, font_t font)
{
    if (!c)
    {
        return 0;
    }
    if (c == ' ')
    {
        return 4;
    }
    int graphicOffset = map_charToFontGraphic[c];
    if (!graphicOffset)
    {
        return 0;
    }
    int graphicId = image_group(GROUP_FONT) + font + graphicOffset - 1;
    return 1 + image_get(graphicId)->width;
}

int getWordWidth(const unsigned char *str, font_t font, int *outNumChars)
{
    int width = 0;
    int guard = 0;
    int wordCharSeen = 0;
    int numChars = 0;
    for (unsigned char c = *str; c; c = *(++str))
    {
        if (++guard >= 200)
        {
            break;
        }
        if (c == ' ')
        {
            if (wordCharSeen)
            {
                break;
            }
            width += 4;
        }
        else if (c == '$')
        {
            if (wordCharSeen)
            {
                break;
            }
        }
        else if (c > ' ')
        {
            // normal char
            width += getCharacterWidth(c, font);
            wordCharSeen = 1;
        }
        numChars++;
    }
    *outNumChars = numChars;
    return width;
}


int drawCharacter(font_t font, unsigned int c, int x, int y, int lineHeight, color_t color)
{
    int graphicOffset = map_charToFontGraphic[c];
    if (!graphicOffset)
    {
        return 0;
    }

    int graphicId = image_group(GROUP_FONT) + font + graphicOffset - 1;
    int height = image_get(graphicId)->height - lineHeight;
    if (height < 0)
    {
        height = 0;
    }
    if (c < 128 || c == 231)   // Some exceptions...
    {
        height = 0;
    }
    Graphics_drawLetter(graphicId, x, y - height, color);
    return image_get(graphicId)->width;
}

void numberToString(char *str, int value, char prefix, const char *postfix)
{
    int offset = 0;
    if (prefix)
    {
        str[offset++] = prefix;
    }
    offset += string_from_int(&str[offset], value, 0);
    while (*postfix)
    {
        str[offset++] = *postfix;
        postfix++;
    }
    str[offset] = 0;
}


int Widget_GameText_draw(int group, int number, int xOffset, int yOffset, font_t font)
{
    const char *str = lang_get_string(group, number);
    return Widget::Text::draw(str, xOffset, yOffset, font, 0);
}

int Widget_GameText_drawColored(int group, int number, int xOffset, int yOffset, font_t font, color_t color)
{
    const char *str = lang_get_string(group, number);
    return Widget::Text::draw(str, xOffset, yOffset, font, color);
}

void Widget_GameText_drawCentered(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font)
{
    const char *str = lang_get_string(group, number);
    Widget::Text::drawCentered(str, xOffset, yOffset, boxWidth, font, 0);
}

void Widget_GameText_drawCenteredColored(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font, color_t color)
{
    const char *str = lang_get_string(group, number);
    Widget::Text::drawCentered(str, xOffset, yOffset, boxWidth, font, color);
}

int Widget_GameText_drawNumberWithDescription(int group, int number, int amount, int xOffset, int yOffset, font_t font)
{
    int amountOffset = 1;
    if (amount == 1 || amount == -1)
    {
        amountOffset = 0;
    }
    int descOffsetX;
    if (amount >= 0)
    {
        descOffsetX = Widget::Text::drawNumber(amount, ' ', " ",
                                               xOffset, yOffset, font);
    }
    else
    {
        descOffsetX = Widget::Text::drawNumber(-amount, '-', " ",
                                               xOffset, yOffset, font);
    }
    return descOffsetX + Widget_GameText_draw(group, number + amountOffset,
            xOffset + descOffsetX, yOffset, font);
}

int Widget_GameText_drawYear(int year, int xOffset, int yOffset, font_t font)
{
    int width = 0;
    if (year >= 0)
    {
        width += Widget_GameText_draw(20, 1, xOffset + width, yOffset, font);
        width += Widget::Text::drawNumber(year, ' ', " ", xOffset + width, yOffset, font);
    }
    else
    {
        width += Widget::Text::drawNumber(-year, ' ', " ", xOffset + width, yOffset, font);
        width += Widget_GameText_draw(20, 0, xOffset + width, yOffset, font);
    }
    return width;
}

int Widget_GameText_drawYearColored(int year, int xOffset, int yOffset, font_t font, color_t color)
{
    int width = 0;
    if (year >= 0)
    {
        width += Widget_GameText_drawColored(20, 1, xOffset + width, yOffset, font, color);
        width += Widget::Text::drawNumberColored(year, ' ', " ", xOffset + width, yOffset, font, color);
    }
    else
    {
        width += Widget::Text::drawNumberColored(-year, ' ', " ", xOffset + width, yOffset, font, color);
        width += Widget_GameText_drawColored(20, 0, xOffset + width, yOffset, font, color);
    }
    return width;
}

int Widget_GameText_drawYearNoSpacing(int year, int xOffset, int yOffset, font_t font)
{
    int width = 0;
    if (year >= 0)
    {
        width += Widget_GameText_draw(20, 1, xOffset + width, yOffset, font);
        width += Widget::Text::drawNumber(year, ' ', " ", xOffset + width, yOffset, font);
    }
    else
    {
        width += Widget::Text::drawNumber(-year, ' ', " ", xOffset + width, yOffset, font);
        width += Widget_GameText_draw(20, 0, xOffset + width - 8, yOffset, font);
    }
    return width;
}

int Widget_GameText_drawMultiline(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font)
{
    const char *str = lang_get_string(group, number);
    return Widget::Text::drawMultiline(str, xOffset, yOffset, boxWidth, font);
}

// RICH TEXT

#define MAX_LINKS 50

static void drawRichTextLine(const char *str, int x, int y, color_t color, int measureOnly);
static int getRichTextWordWidth(const char *str, int *outNumChars);
static int drawRichTextCharacter(font_t font, unsigned int c, int x, int y, color_t color, int measureOnly);

static ImageButton imageButtonScrollUp =
{
    0, 0, 39, 26, ImageButton_Scroll, 96, 8, Widget_RichText_scroll, Widget::Button::doNothing, 0, 1, 1
};
static ImageButton imageButtonScrollDown =
{
    0, 0, 39, 26, ImageButton_Scroll, 96, 12, Widget_RichText_scroll, Widget::Button::doNothing, 1, 1, 1
};

static struct RichTextLink
{
    int messageId;
    int xMin;
    int yMin;
    int xMax;
    int yMax;
} links[MAX_LINKS];

static int numLinks;
static font_t richTextNormalFont = FONT_NORMAL_WHITE;
static font_t richTextLinkFont = FONT_NORMAL_RED;

static struct
{
    int xText;
    int yText;
    int textWidthBlocks;
    int textHeightBlocks;
    int textHeightLines;
    int numberOfLines;
    int scrollPosition;
    int maxScrollPosition;
    int isDraggingScroll;
    int scrollPositionDrag;
    struct
    {
        int scrollPosition;
        int numberOfLines;
        int textHeightLines;
    } backup;
} data;

void Widget_RichText_setFonts(font_t normalFont, font_t linkFont)
{
    richTextNormalFont = normalFont;
    richTextLinkFont = linkFont;
}

int Widget_RichText_getClickedLink(const mouse *m)
{
    if (m->left.went_down)
    {
        for (int i = 0; i < numLinks; i++)
        {
            if (m->x >= links[i].xMin && m->x <= links[i].xMax &&
                    m->y >= links[i].yMin && m->y <= links[i].yMax)
            {
                return links[i].messageId;
            }
        }
    }
    return -1;
}

static void addRichTextLink(int messageId, int xStart, int xEnd, int y)
{
    if (numLinks < MAX_LINKS)
    {
        links[numLinks].messageId = messageId;
        links[numLinks].xMin = xStart - 2;
        links[numLinks].xMax = xEnd + 2;
        links[numLinks].yMin = y - 1;
        links[numLinks].yMax = y + 13;
        numLinks++;
    }
}

void Widget_RichText_clearLinks()
{
    for (int i = 0; i < MAX_LINKS; i++)
    {
        links[i].messageId = 0;
        links[i].xMin = 0;
        links[i].xMax = 0;
        links[i].yMin = 0;
        links[i].yMax = 0;
    }
    numLinks = 0;
}

void Widget_RichText_reset(int scrollPosition)
{
    data.scrollPosition = scrollPosition;
    data.numberOfLines = 0;
    data.isDraggingScroll = 0;
}

void Widget_RichText_save()
{
    data.backup.numberOfLines = data.numberOfLines;
    data.backup.scrollPosition = data.scrollPosition;
    data.backup.textHeightLines = data.textHeightLines;
    data.scrollPosition = 0;
    data.textHeightLines = 30;
}

void Widget_RichText_restore()
{
    data.numberOfLines = data.backup.numberOfLines;
    data.scrollPosition = data.backup.scrollPosition;
    data.textHeightLines = data.backup.textHeightLines;
}

static int drawRichText(const char *str, int xOffset, int yOffset,
                        int boxWidth, int heightLines, color_t color, int measureOnly)
{
    int graphicHeightLines = 0;
    int graphicId = 0;
    int linesBeforeGraphic = 0;
    int paragraph = 0;
    int hasMoreCharacters = 1;
    int y = yOffset;
    int guard = 0;
    int line = 0;
    int numLines = 0;
    while (hasMoreCharacters || graphicHeightLines)
    {
        if (++guard >= 1000)
        {
            break;
        }
        // clear line
        for (int i = 0; i < 200; i++)
        {
            tmpLine[i] = 0;
        }
        int currentWidth;
        int xLineOffset;
        int lineIndex = 0;
        if (paragraph)
        {
            currentWidth = xLineOffset = 50;
        }
        else
        {
            currentWidth = xLineOffset = 0;
        }
        paragraph = 0;
        while ((hasMoreCharacters || graphicHeightLines) && currentWidth < boxWidth)
        {
            if (graphicHeightLines)
            {
                graphicHeightLines--;
                break;
            }
            int wordNumChars;
            currentWidth += getRichTextWordWidth(str, &wordNumChars);
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
                    char c = *(str++);
                    if (c == '@')
                    {
                        if (*str == 'P')
                        {
                            paragraph = 1;
                            str++;
                            currentWidth = boxWidth;
                            break;
                        }
                        else if (*str == 'L')
                        {
                            str++;
                            currentWidth = boxWidth;
                            break;
                        }
                        else if (*str == 'G')
                        {
                            if (lineIndex)
                            {
                                numLines++;
                            }
                            str++; // skip 'G'
                            currentWidth = boxWidth;
                            graphicId = string_to_int(str);
                            c = *(str++);
                            while (c >= '0' && c <= '9')
                            {
                                c = *(str++);
                            }
                            graphicId += image_group(GROUP_MESSAGE_IMAGES) - 1;
                            graphicHeightLines = image_get(graphicId)->height / 16 + 2;
                            if (line > 0)
                            {
                                linesBeforeGraphic = 1;
                            }
                            break;
                        }
                    }
                    if (lineIndex || c != ' ')   // no space at start of line
                    {
                        tmpLine[lineIndex++] = c;
                    }
                }
                if (!*str)
                {
                    hasMoreCharacters = 0;
                }
            }
        }

        int outsideViewport = 0;
        if (!measureOnly)
        {
            if (line < data.scrollPosition || line >= data.scrollPosition + heightLines)
            {
                outsideViewport = 1;
            }
        }
        if (!outsideViewport)
        {
            drawRichTextLine(tmpLine, xLineOffset + xOffset, y, color, measureOnly);
        }
        if (!measureOnly)
        {
            if (graphicId)
            {
                if (linesBeforeGraphic)
                {
                    linesBeforeGraphic--;
                }
                else
                {
                    const image *img = image_get(graphicId);
                    graphicHeightLines = img->height / 16 + 2;
                    int xOffsetGraphic = xOffset + (boxWidth - img->width) / 2 - 4;
                    if (line < heightLines + data.scrollPosition)
                    {
                        if (line >= data.scrollPosition)
                        {
                            Graphics_drawImage(graphicId, xOffsetGraphic, y + 8);
                        }
                        else
                        {
                            Graphics_drawImage(graphicId, xOffsetGraphic, y + 8 - 16 * (data.scrollPosition - line));
                        }
                    }
                    graphicId = 0;
                }
            }
        }
        line++;
        numLines++;
        if (!outsideViewport)
        {
            y += 16;
        }
    }
    return numLines;
}

int Widget_RichText_draw(const char *str, int xOffset, int yOffset,
                         int boxWidth, int heightLines, int measureOnly)
{
    return drawRichText(str, xOffset, yOffset, boxWidth, heightLines, 0, measureOnly);
}

int Widget_RichText_drawColored(const char *str, int xOffset, int yOffset, int boxWidth, int heightLines, color_t color)
{
    return drawRichText(str, xOffset, yOffset, boxWidth, heightLines, color, 0);
}

static void drawRichTextLine(const char *str, int x, int y, color_t color, int measureOnly)
{
    int numLinkChars = 0;
    for (unsigned char c = *str; c; c = *(++str))
    {
        if (c == '@')
        {
            int messageId = string_to_int(++str);
            while (*str >= '0' && *str <= '9')
            {
                str++;
            }
            int width = getRichTextWordWidth(str, &numLinkChars);
            addRichTextLink(messageId, x, x + width, y);
            c = *str;
        }
        if (c >= ' ')
        {
            font_t font = richTextNormalFont;
            if (numLinkChars > 0)
            {
                font = richTextLinkFont;
                numLinkChars--;
            }
            if (map_charToFontGraphic[c] <= 0)
            {
                x += 6;
            }
            else
            {
                x += drawRichTextCharacter(font, c, x, y, color, measureOnly);
            }
        }
    }
}

static int getRichTextWordWidth(const char *str, int *outNumChars)
{
    int width = 0;
    int guard = 0;
    int wordCharSeen = 0;
    int numChars = 0;
    for (unsigned char c = *str; c; c = *(++str))
    {
        if (++guard >= 2000)
        {
            break;
        }
        if (c == '@')
        {
            c = *(++str);
            if (!wordCharSeen)
            {
                if (c == 'P' || c == 'L')
                {
                    numChars += 2;
                    width = 0;
                    break;
                }
                else if (c == 'G')
                {
                    // skip graphic
                    numChars += 2;
                    while (c >= '0' && c <= '9')
                    {
                        c = *(++str);
                        numChars++;
                    }
                    width = 0;
                    break;
                }
                else
                {
                    numChars++;
                    while (c >= '0' && c <= '9')
                    {
                        c = *(++str);
                        numChars++;
                    }
                }
            }
        }
        if (c == ' ')
        {
            if (wordCharSeen)
            {
                break;
            }
            width += 4;
        }
        else if (c > ' ')
        {
            // normal char
            width += getCharacterWidth(c, richTextNormalFont);
            wordCharSeen = 1;
        }
        numChars++;
    }
    *outNumChars = numChars;
    return width;
}

static int drawRichTextCharacter(font_t font, unsigned int c, int x, int y, color_t color, int measureOnly)
{
    int graphicOffset = map_charToFontGraphic[c];
    if (!graphicOffset)
    {
        return 0;
    }

    int graphicId = image_group(GROUP_FONT) + font + graphicOffset - 1;
    int height = image_get(graphicId)->height - 11;
    if (height < 0)
    {
        height = 0;
    }
    if (c < 128 || c == 231)   // Some exceptions...
    {
        height = 0;
    }
    if (!measureOnly)
    {
        Graphics_drawLetter(graphicId, x, y - height, color);
    }
    return image_get(graphicId)->width;
}

void Widget_RichText_drawScrollbar()
{
    if (data.maxScrollPosition)
    {
        Widget::Button::drawImageButtons(
            data.xText + 16 * data.textWidthBlocks - 1,
            data.yText,
            &imageButtonScrollUp, 1);
        Widget::Button::drawImageButtons(
            data.xText + 16 * data.textWidthBlocks - 1,
            data.yText + 16 * data.textHeightBlocks - 26,
            &imageButtonScrollDown, 1);
        Widget_RichText_drawScrollbarDot();
    }
}

void Widget_RichText_drawScrollbarDot()
{
    if (data.maxScrollPosition)
    {
        int pct;
        if (data.scrollPosition <= 0)
        {
            pct = 0;
        }
        else if (data.scrollPosition >= data.maxScrollPosition)
        {
            pct = 100;
        }
        else
        {
            pct = calc_percentage(data.scrollPosition, data.maxScrollPosition);
        }
        int offset = calc_adjust_with_percentage(16 * data.textHeightBlocks - 77, pct);
        if (data.isDraggingScroll)
        {
            offset = data.scrollPositionDrag;
        }
        Graphics_drawImage(image_group(GROUP_PANEL_BUTTON) + 39,
                           data.xText + 16 * data.textWidthBlocks + 6, data.yText + offset + 26);
    }
}

void Widget_RichText_scroll(int isDown, int numLines)
{
    if (isDown)
    {
        data.scrollPosition += numLines;
        if (data.scrollPosition > data.maxScrollPosition)
        {
            data.scrollPosition = data.maxScrollPosition;
        }
    }
    else
    {
        data.scrollPosition -= numLines;
        if (data.scrollPosition < 0)
        {
            data.scrollPosition = 0;
        }
    }
    Widget_RichText_clearLinks();
    data.isDraggingScroll = 0;
    UI_Window_requestRefresh();
}

static int handleScrollbarDot(const mouse *m)
{
    if (data.maxScrollPosition <= 0 || !m->left.is_down)
    {
        return 0;
    }
    int totalHeight = 16 * data.textHeightBlocks - 52;
    if (m->x < data.xText + 16 * data.textWidthBlocks + 1 ||
            m->x > data.xText + 16 * data.textWidthBlocks + 41)
    {
        return 0;
    }
    if (m->y < data.yText + 26 || m->y > data.yText + 26 + totalHeight)
    {
        return 0;
    }
    int dotHeight = m->y - data.yText - 11;
    if (dotHeight > totalHeight)
    {
        dotHeight = totalHeight;
    }
    int pctScrolled = calc_percentage(dotHeight, totalHeight);
    data.scrollPosition = calc_adjust_with_percentage(data.maxScrollPosition, pctScrolled);
    data.isDraggingScroll = 1;
    data.scrollPositionDrag = dotHeight - 25;
    Widget_RichText_clearLinks();
    if (data.scrollPositionDrag < 0)
    {
        data.scrollPositionDrag = 0;
    }
    UI_Window_requestRefresh();
    return 1;
}

int Widget_RichText_handleScrollbar(const mouse *m)
{
    if (m->scrolled == SCROLL_DOWN)
    {
        Widget_RichText_scroll(1, 3);
    }
    else if (m->scrolled == SCROLL_UP)
    {
        Widget_RichText_scroll(0, 3);
    }

    if (Widget::Button::handleImageButtons(
                data.xText + 16 * data.textWidthBlocks - 1,
                data.yText,
                &imageButtonScrollUp, 1, 0))
    {
        return 1;
    }
    if (Widget::Button::handleImageButtons(
                data.xText + 16 * data.textWidthBlocks - 1,
                data.yText + 16 * data.textHeightBlocks - 26,
                &imageButtonScrollDown, 1, 0))
    {
        return 1;
    }
    return handleScrollbarDot(m);
}

int Widget_RichText_getScrollPosition()
{
    return data.scrollPosition;
}

int Widget_RichText_init(const char *str, int xText, int yText, int widthBlocks, int heightBlocks, int adjustWidthOnNoScroll)
{
    data.xText = xText;
    data.yText = yText;
    if (!data.numberOfLines)
    {
        data.textHeightBlocks = heightBlocks;
        data.textHeightLines = heightBlocks - 1;
        data.textWidthBlocks = widthBlocks;

        imageButtonScrollUp.enabled = 1;
        imageButtonScrollDown.enabled = 1;

        data.numberOfLines = Widget_RichText_draw(str,
                             data.xText + 8, data.yText + 6,
                             16 * data.textWidthBlocks - 16, data.textHeightLines, 1);
        if (data.numberOfLines <= data.textHeightLines)
        {
            if (adjustWidthOnNoScroll)
            {
                data.textWidthBlocks += 2;
            }
            data.maxScrollPosition = 0;
            imageButtonScrollUp.enabled = 0;
            imageButtonScrollDown.enabled = 0;
        }
        else
        {
            data.maxScrollPosition = data.numberOfLines - data.textHeightLines;
        }
        UI_Window_requestRefresh();
    }
    return data.textWidthBlocks;
}
