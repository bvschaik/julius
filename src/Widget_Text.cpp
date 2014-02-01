#include "Widget.h"

#include "Data/Constants.h"
#include "Data/Graphics.h"
#include "Data/KeyboardInput.h"

#include "Graphics.h"
#include "Language.h"
#include "String.h"

#include <stdio.h>

static const int map_charToFontGraphic[] = {
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

static char tmpLine[200];

static int drawCharacter(Font font, unsigned int c, int x, int y, int lineHeight, Color color);

int Widget_Text_getWidth(const char *str, Font font)
{
	int spaceWidth;
	int letterSpacing;
	
	if (font == Font_LargePlain || font == Font_LargeBlack || font == Font_LargeBrown) {
		spaceWidth = 10;
		letterSpacing = 1;
	} else if (font == Font_SmallPlain) {
		spaceWidth = 4;
		letterSpacing = 1;
	} else if (font == Font_NormalPlain) {
		spaceWidth = 6;
		letterSpacing = 1;
	} else {
		spaceWidth = 6;
		letterSpacing = 0;
	}
	
	int maxlen = 10000;
	int width = 0;
	int graphicBase = GraphicId(ID_Graphic_Font);
	while (*str && maxlen > 0) {
		if (*str == ' ') {
			width += spaceWidth;
		} else {
			int graphicOffset = map_charToFontGraphic[(int) *str];
			if (graphicOffset) {
				int graphicId = graphicBase + font + graphicOffset - 1;
				width += letterSpacing + Data_Graphics_Main.index[graphicId].width;
			}
		}
		str++;
		maxlen--;
	}
	return width;
}

int Widget_GameText_getWidth(int group, int number, Font font)
{
	const char *str = Language_getString(group, number);
	return Widget_Text_getWidth(str, font);
}

static int getCharacterWidth(unsigned char c, Font font)
{
	if (!c) {
		return 0;
	}
	if (c == ' ') {
		return 4;
	}
	int graphicOffset = map_charToFontGraphic[(int)c];
	if (!graphicOffset) {
		return 0;
	}
	int graphicId = GraphicId(ID_Graphic_Font) + font + graphicOffset - 1;
	return 1 + Data_Graphics_Main.index[graphicId].width;
}

static int getWordWidth(const unsigned char *str, Font font, int *outNumChars)
{
	int width = 0;
	int guard = 0;
	int wordCharSeen = 0;
	int numChars = 0;
	for (unsigned char c = *str; c; c = *(++str)) {
		if (++guard >= 200) {
			break;
		}
		if (c == ' ') {
			if (wordCharSeen) {
				break;
			}
			width += 4;
		} else if (c == '$') {
			if (wordCharSeen) {
				break;
			}
		} else if (c > ' ') {
			// normal char
			width += getCharacterWidth(c, font);
			wordCharSeen = 1;
		}
		numChars++;
	}
	*outNumChars = numChars;
	return width;
}

void Widget_Text_drawCentered(const char *str, int x, int y, int boxWidth, Font font, Color color)
{
	int offset = (boxWidth - Widget_Text_getWidth(str, font)) / 2;
	if (offset < 0) {
		offset = 0;
	}
	Widget_Text_draw(str, offset + x, y, font, color);
}

int Widget_Text_draw(const char *str, int x, int y, Font font, Color color)
{
	int letterSpacing;
	int lineHeight;
	int spaceWidth;
	switch (font) {
		case Font_LargePlain:
			spaceWidth = 8;
			lineHeight = 23;
			letterSpacing = 1;
			break;
		case Font_LargeBlack:
			spaceWidth = 8;
			lineHeight = 23;
			letterSpacing = 0;
			break;
		case Font_LargeBrown:
			spaceWidth = 8;
			lineHeight = 24;
			letterSpacing = 0;
			break;
		case Font_SmallPlain:
			spaceWidth = 4;
			lineHeight = 9;
			letterSpacing = 1;
			break;
		case Font_NormalPlain:
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
	while (*str) {
		unsigned char c = (unsigned char) *str;

		if (c == '_') {
			c = ' ';
		}

		if (c >= ' ') {
			int width = 0;
			int graphic = map_charToFontGraphic[c];
			if (graphic == 0) {
				width = spaceWidth;
			} else {
				width = letterSpacing + drawCharacter(font, c, currentX, y, lineHeight, color);
			}
			currentX += width;
		}

		str++;
		// TODO: stuff related to cursor
	}
	currentX += spaceWidth;
	return currentX - x;
}

static int drawCharacter(Font font, unsigned int c, int x, int y, int lineHeight, Color color)
{
	int graphicOffset = map_charToFontGraphic[c];
	if (!graphicOffset) {
		return 0;
	}

	int graphicId = GraphicId(ID_Graphic_Font) + font + graphicOffset - 1;
	int height = Data_Graphics_Main.index[graphicId].height - lineHeight;
	if (height < 0) {
		height = 0;
	}
	if (c < 128 || c == 231) { // Some exceptions...
		height = 0;
	}
	Graphics_drawLetter(graphicId, x, y - height, color);
	return Data_Graphics_Main.index[graphicId].width;
}

static void numberToString(char *str, int value, char prefix, const char *postfix)
{
	int offset = 0;
	if (prefix) {
		str[offset++] = prefix;
	}
	offset += String_intToString(&str[offset], value, 0);
	while (*postfix) {
		str[offset++] = *postfix;
		postfix++;
	}
	str[offset] = 0;
}

int Widget_Text_drawNumber(int value, char prefix, const char *postfix, int xOffset, int yOffset, Font font, Color color)
{
	char str[100];
	numberToString(str, value, prefix, postfix);
	return Widget_Text_draw(str, xOffset, yOffset, font, color);
}

void Widget_Text_drawNumberCentered(int value, char prefix, const char *postfix, int xOffset, int yOffset, int boxWidth, Font font, Color color)
{
	char str[100];
	numberToString(str, value, prefix, postfix);
	Widget_Text_drawCentered(str, xOffset, yOffset, boxWidth, font, color);
}


int Widget_GameText_draw(int group, int number, int xOffset, int yOffset, Font font, Color color)
{
	const char *str = Language_getString(group, number);
	return Widget_Text_draw(str, xOffset, yOffset, font, color);
}

void Widget_GameText_drawCentered(int group, int number, int xOffset, int yOffset, int boxWidth, Font font, Color color)
{
	const char *str = Language_getString(group, number);
	Widget_Text_drawCentered(str, xOffset, yOffset, boxWidth, font, color);
}

int Widget_GameText_drawNumberWithDescription(int group, int number, int amount, int xOffset, int yOffset, Font font, Color color)
{
	int amountOffset = 1;
	if (amount == 1 || amount == -1) {
		amountOffset = 0;
	}
	int descOffsetX;
	if (amount >= 0) {
		descOffsetX = Widget_Text_drawNumber(amount, ' ', " ",
			xOffset, yOffset, font, color);
	} else {
		descOffsetX = Widget_Text_drawNumber(-amount, '-', " ",
			xOffset, yOffset, font, color);
	}
	return descOffsetX + Widget_GameText_draw(group, number + amountOffset,
		xOffset + descOffsetX, yOffset, font, color);
}

int Widget_GameText_drawYear(int year, int xOffset, int yOffset, Font font, Color color)
{
	int width = 0;
	if (year >= 0) {
		width += Widget_GameText_draw(20, 1, xOffset + width, yOffset, font, color);
		width += Widget_Text_drawNumber(year, ' ', " ", xOffset + width, yOffset, font, color);
	} else {
		width += Widget_Text_drawNumber(-year, ' ', " ", xOffset + width, yOffset, font, color);
		width += Widget_GameText_draw(20, 0, xOffset + width, yOffset, font, color);
	}
	return width;
}

int Widget_GameText_drawYearNoSpacing(int year, int xOffset, int yOffset, Font font, Color color)
{
	int width = 0;
	if (year >= 0) {
		width += Widget_GameText_draw(20, 1, xOffset + width, yOffset, font, color);
		width += Widget_Text_drawNumber(year, ' ', " ", xOffset + width, yOffset, font, color);
	} else {
		width += Widget_Text_drawNumber(-year, ' ', " ", xOffset + width, yOffset, font, color);
		width += Widget_GameText_draw(20, 0, xOffset + width - 8, yOffset, font, color);
	}
	return width;
}

int Widget_Text_drawMultiline(const char *str, int xOffset, int yOffset, int boxWidth, Font font, Color color)
{
	int lineHeight;
	switch (font) {
		case Font_LargePlain:
		case Font_LargeBlack:
			lineHeight = 23;
			break;
		case Font_LargeBrown:
			lineHeight = 24;
			break;
		case Font_SmallPlain:
			lineHeight = 9;
			break;
		case Font_NormalPlain:
			lineHeight = 11;
			break;
		default:
			lineHeight = 11;
			break;
	}

	int hasMoreCharacters = 1;
	int guard = 0;
	int y = yOffset;
	while (hasMoreCharacters) {
		if (++guard >= 100) {
			break;
		}
		// clear line
		for (int i = 0; i < 200; i++) {
			tmpLine[i] = 0;
		}
		int currentWidth = 0;
		int lineIndex = 0;
		while (hasMoreCharacters && currentWidth < boxWidth) {
			int wordNumChars;
			int wordWidth = getWordWidth((unsigned char*)str, font, &wordNumChars);
			currentWidth += wordWidth;
			if (currentWidth >= boxWidth) {
				if (currentWidth == 0) {
					hasMoreCharacters = 0;
				}
			} else {
				for (int i = 0; i < wordNumChars; i++) {
					tmpLine[lineIndex++] = *(str++);
				}
				if (!*str) {
					hasMoreCharacters = 0;
				}
			}
		}
		Widget_Text_draw(tmpLine, xOffset, y, font, color);
		y += lineHeight + 5;
	}
	return y - yOffset;
}

int Widget_GameText_drawMultiline(int group, int number, int xOffset, int yOffset, int boxWidth, Font font, Color color)
{
	const char *str = Language_getString(group, number);
	return Widget_Text_drawMultiline(str, xOffset, yOffset, boxWidth, font, color);
}

