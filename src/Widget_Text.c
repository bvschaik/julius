#include "Widget.h"

#include "Data/Constants.h"
#include "Data/Graphics.h"
#include "Data/KeyboardInput.h"
#include "Data/Mouse.h"

#include "core/calc.h"
#include "Graphics.h"
#include "Language.h"
#include "String.h"
#include "UI/Window.h"

#include "core/time.h"

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

static struct {
	int capture;
	int seen;
	int position;
	int width;
	int visible;
	time_millis updated;
	int xOffset;
	int yOffset;
} inputCursor;

static int drawCharacter(Font font, unsigned int c, int x, int y, int lineHeight, Color color);

void Widget_Text_captureCursor()
{
	inputCursor.capture = 1;
	inputCursor.seen = 0;
	inputCursor.position = 0;
	inputCursor.width = 0;
}

void Widget_Text_drawCursor(int xOffset, int yOffset)
{
	inputCursor.capture = 0;
	time_millis curr = time_get_millis();
	time_millis diff = curr - inputCursor.updated;
	if (!inputCursor.visible && diff >= 200) {
		inputCursor.visible = 1;
		inputCursor.updated = curr;
	} else if (inputCursor.visible && diff >= 400) {
		inputCursor.visible = 0;
		inputCursor.updated = curr;
	}
	if (inputCursor.visible) {
		if (Data_KeyboardInput.isInsert) {
			Graphics_drawLine(
				xOffset + inputCursor.xOffset - 3, yOffset + inputCursor.yOffset - 3,
				xOffset + inputCursor.xOffset + 1, yOffset + inputCursor.yOffset - 3,
				Color_White);
			Graphics_drawLine(
				xOffset + inputCursor.xOffset - 1, yOffset + inputCursor.yOffset - 3,
				xOffset + inputCursor.xOffset - 1, yOffset + inputCursor.yOffset + 13,
				Color_White);
			Graphics_drawLine(
				xOffset + inputCursor.xOffset - 3, yOffset + inputCursor.yOffset + 14,
				xOffset + inputCursor.xOffset + 1, yOffset + inputCursor.yOffset + 14,
				Color_White);
		} else {
			Graphics_fillRect(
				xOffset + inputCursor.xOffset, yOffset + inputCursor.yOffset + 14,
				inputCursor.width, 2, Color_White);
		}
	}
}

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
			int graphicOffset = map_charToFontGraphic[(unsigned char) *str];
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

static int getSpaceWidth(Font font)
{
	if (font == Font_LargePlain || font == Font_LargeBlack || font == Font_LargeBrown) {
		return 10;
	} else if (font == Font_SmallPlain) {
		return 4;
	} else {
		return 6;
	}
}

int Widget_GameText_getDrawWidth(int group, int number, Font font)
{
	const char *str = Language_getString(group, number);
	return Widget_Text_getWidth(str, font) + getSpaceWidth(font);
}

static int getCharacterWidth(unsigned char c, Font font)
{
	if (!c) {
		return 0;
	}
	if (c == ' ') {
		return 4;
	}
	int graphicOffset = map_charToFontGraphic[c];
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
			if (inputCursor.capture &&
				inputCursor.position == Data_KeyboardInput.lines[Data_KeyboardInput.current].cursorPosition) {
				if (!inputCursor.seen) {
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
	if (inputCursor.capture && !inputCursor.seen) {
		inputCursor.width = 4;
		inputCursor.xOffset = currentX - x;
		inputCursor.seen = 1;
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
	offset += String_fromInt(&str[offset], value, 0);
	while (*postfix) {
		str[offset++] = *postfix;
		postfix++;
	}
	str[offset] = 0;
}

int Widget_Text_drawNumber(int value, char prefix, const char *postfix, int xOffset, int yOffset, Font font)
{
	char str[100];
	numberToString(str, value, prefix, postfix);
	return Widget_Text_draw(str, xOffset, yOffset, font, 0);
}

int Widget_Text_drawNumberColored(int value, char prefix, const char *postfix, int xOffset, int yOffset, Font font, Color color)
{
	char str[100];
	numberToString(str, value, prefix, postfix);
	return Widget_Text_draw(str, xOffset, yOffset, font, color);
}

void Widget_Text_drawNumberCentered(int value, char prefix, const char *postfix, int xOffset, int yOffset, int boxWidth, Font font)
{
	char str[100];
	numberToString(str, value, prefix, postfix);
	Widget_Text_drawCentered(str, xOffset, yOffset, boxWidth, font, 0);
}

void Widget_Text_drawNumberCenteredColored(int value, char prefix, const char *postfix, int xOffset, int yOffset, int boxWidth, Font font, Color color)
{
	char str[100];
	numberToString(str, value, prefix, postfix);
	Widget_Text_drawCentered(str, xOffset, yOffset, boxWidth, font, color);
}


int Widget_GameText_draw(int group, int number, int xOffset, int yOffset, Font font)
{
	const char *str = Language_getString(group, number);
	return Widget_Text_draw(str, xOffset, yOffset, font, 0);
}

int Widget_GameText_drawColored(int group, int number, int xOffset, int yOffset, Font font, Color color)
{
	const char *str = Language_getString(group, number);
	return Widget_Text_draw(str, xOffset, yOffset, font, color);
}

void Widget_GameText_drawCentered(int group, int number, int xOffset, int yOffset, int boxWidth, Font font)
{
	const char *str = Language_getString(group, number);
	Widget_Text_drawCentered(str, xOffset, yOffset, boxWidth, font, 0);
}

void Widget_GameText_drawCenteredColored(int group, int number, int xOffset, int yOffset, int boxWidth, Font font, Color color)
{
	const char *str = Language_getString(group, number);
	Widget_Text_drawCentered(str, xOffset, yOffset, boxWidth, font, color);
}

int Widget_GameText_drawNumberWithDescription(int group, int number, int amount, int xOffset, int yOffset, Font font)
{
	int amountOffset = 1;
	if (amount == 1 || amount == -1) {
		amountOffset = 0;
	}
	int descOffsetX;
	if (amount >= 0) {
		descOffsetX = Widget_Text_drawNumber(amount, ' ', " ",
			xOffset, yOffset, font);
	} else {
		descOffsetX = Widget_Text_drawNumber(-amount, '-', " ",
			xOffset, yOffset, font);
	}
	return descOffsetX + Widget_GameText_draw(group, number + amountOffset,
		xOffset + descOffsetX, yOffset, font);
}

int Widget_GameText_drawYear(int year, int xOffset, int yOffset, Font font)
{
	int width = 0;
	if (year >= 0) {
		width += Widget_GameText_draw(20, 1, xOffset + width, yOffset, font);
		width += Widget_Text_drawNumber(year, ' ', " ", xOffset + width, yOffset, font);
	} else {
		width += Widget_Text_drawNumber(-year, ' ', " ", xOffset + width, yOffset, font);
		width += Widget_GameText_draw(20, 0, xOffset + width, yOffset, font);
	}
	return width;
}

int Widget_GameText_drawYearColored(int year, int xOffset, int yOffset, Font font, Color color)
{
	int width = 0;
	if (year >= 0) {
		width += Widget_GameText_drawColored(20, 1, xOffset + width, yOffset, font, color);
		width += Widget_Text_drawNumberColored(year, ' ', " ", xOffset + width, yOffset, font, color);
	} else {
		width += Widget_Text_drawNumberColored(-year, ' ', " ", xOffset + width, yOffset, font, color);
		width += Widget_GameText_drawColored(20, 0, xOffset + width, yOffset, font, color);
	}
	return width;
}

int Widget_GameText_drawYearNoSpacing(int year, int xOffset, int yOffset, Font font)
{
	int width = 0;
	if (year >= 0) {
		width += Widget_GameText_draw(20, 1, xOffset + width, yOffset, font);
		width += Widget_Text_drawNumber(year, ' ', " ", xOffset + width, yOffset, font);
	} else {
		width += Widget_Text_drawNumber(-year, ' ', " ", xOffset + width, yOffset, font);
		width += Widget_GameText_draw(20, 0, xOffset + width - 8, yOffset, font);
	}
	return width;
}

int Widget_Text_drawMultiline(const char *str, int xOffset, int yOffset, int boxWidth, Font font)
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
		Widget_Text_draw(tmpLine, xOffset, y, font, 0);
		y += lineHeight + 5;
	}
	return y - yOffset;
}

int Widget_GameText_drawMultiline(int group, int number, int xOffset, int yOffset, int boxWidth, Font font)
{
	const char *str = Language_getString(group, number);
	return Widget_Text_drawMultiline(str, xOffset, yOffset, boxWidth, font);
}


// RICH TEXT

#define MAX_LINKS 50

static void drawRichTextLine(const unsigned char *str, int x, int y, Color color, int measureOnly);
static int getRichTextWordWidth(const unsigned char *str, int *outNumChars);
static int drawRichTextCharacter(Font font, unsigned int c, int x, int y, Color color, int measureOnly);

static ImageButton imageButtonScrollUp = {
	0, 0, 39, 26, ImageButton_Scroll, 96, 8, Widget_RichText_scroll, Widget_Button_doNothing, 0, 1, 1
};
static ImageButton imageButtonScrollDown = {
	0, 0, 39, 26, ImageButton_Scroll, 96, 12, Widget_RichText_scroll, Widget_Button_doNothing, 1, 1, 1
};

static struct RichTextLink {
	int messageId;
	int xMin;
	int yMin;
	int xMax;
	int yMax;
} links[MAX_LINKS];

static int numLinks;
static Font richTextNormalFont = Font_NormalWhite;
static Font richTextLinkFont = Font_NormalRed;

static struct {
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
	struct {
		int scrollPosition;
		int numberOfLines;
		int textHeightLines;
	} backup;
} data;

void Widget_RichText_setFonts(Font normalFont, Font linkFont)
{
	richTextNormalFont = normalFont;
	richTextLinkFont = linkFont;
}

int Widget_RichText_getClickedLink()
{
	if (Data_Mouse.left.wentDown) {
		for (int i = 0; i < numLinks; i++) {
			if (Data_Mouse.x >= links[i].xMin && Data_Mouse.x <= links[i].xMax &&
				Data_Mouse.y >= links[i].yMin && Data_Mouse.y <= links[i].yMax) {
				return links[i].messageId;
			}
		}
	}
	return -1;
}

static void addRichTextLink(int messageId, int xStart, int xEnd, int y)
{
	if (numLinks < MAX_LINKS) {
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
	for (int i = 0; i < MAX_LINKS; i++) {
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
						int boxWidth, int heightLines, Color color, int measureOnly)
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
	while (hasMoreCharacters || graphicHeightLines) {
		if (++guard >= 1000) {
			break;
		}
		// clear line
		for (int i = 0; i < 200; i++) {
			tmpLine[i] = 0;
		}
		int currentWidth;
		int xLineOffset;
		int lineIndex = 0;
		if (paragraph) {
			currentWidth = xLineOffset = 50;
		} else {
			currentWidth = xLineOffset = 0;
		}
		paragraph = 0;
		while ((hasMoreCharacters || graphicHeightLines) && currentWidth < boxWidth) {
			if (graphicHeightLines) {
				graphicHeightLines--;
				break;
			}
			int wordNumChars;
			currentWidth += getRichTextWordWidth((const unsigned char*)str, &wordNumChars);
			if (currentWidth >= boxWidth) {
				if (currentWidth == 0) {
					hasMoreCharacters = 0;
				}
			} else {
				for (int i = 0; i < wordNumChars; i++) {
					char c = *(str++);
					if (c == '@') {
						if (*str == 'P') {
							paragraph = 1;
							str++;
							currentWidth = boxWidth;
							break;
						} else if (*str == 'L') {
							str++;
							currentWidth = boxWidth;
							break;
						} else if (*str == 'G') {
							if (lineIndex) {
								numLines++;
							}
							str++; // skip 'G'
							currentWidth = boxWidth;
							graphicId = String_toInt(str);
							c = *(str++);
							while (c >= '0' && c <= '9') {
								c = *(str++);
							}
							graphicId += GraphicId(ID_Graphic_MessageImages) - 1;
							graphicHeightLines = GraphicHeight(graphicId) / 16 + 2;
							if (line > 0) {
								linesBeforeGraphic = 1;
							}
							break;
						}
					}
					if (lineIndex || c != ' ') { // no space at start of line
						tmpLine[lineIndex++] = c;
					}
				}
				if (!*str) {
					hasMoreCharacters = 0;
				}
			}
		}

		int outsideViewport = 0;
		if (!measureOnly) {
			if (line < data.scrollPosition || line >= data.scrollPosition + heightLines) {
				outsideViewport = 1;
			}
		}
		if (!outsideViewport) {
			drawRichTextLine((unsigned char*)tmpLine, xLineOffset + xOffset, y, color, measureOnly);
		}
		if (!measureOnly) {
			if (graphicId) {
				if (linesBeforeGraphic) {
					linesBeforeGraphic--;
				} else {
					graphicHeightLines = GraphicHeight(graphicId) / 16 + 2;
					int xOffsetGraphic = xOffset + (boxWidth - Data_Graphics_Main.index[graphicId].width) / 2 - 4;
					if (line < heightLines + data.scrollPosition) {
						if (line >= data.scrollPosition) {
							Graphics_drawImage(graphicId, xOffsetGraphic, y + 8);
						} else {
							Graphics_drawImage(graphicId, xOffsetGraphic, y + 8 - 16 * (data.scrollPosition - line));
						}
					}
					graphicId = 0;
				}
			}
		}
		line++;
		numLines++;
		if (!outsideViewport) {
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

int Widget_RichText_drawColored(const char *str, int xOffset, int yOffset,
								int boxWidth, int heightLines, Color color)
{
	return drawRichText(str, xOffset, yOffset, boxWidth, heightLines, color, 0);
}

static void drawRichTextLine(const unsigned char *str, int x, int y, Color color, int measureOnly)
{
	int numLinkChars = 0;
	for (unsigned char c = *str; c; c = *(++str)) {
		if (c == '@') {
			int messageId = String_toInt((char*)++str);
			while (*str >= '0' && *str <= '9') {
				str++;
			}
			int width = getRichTextWordWidth(str, &numLinkChars);
			addRichTextLink(messageId, x, x + width, y);
			c = *str;
		}
		if (c >= ' ') {
			Font font = richTextNormalFont;
			if (numLinkChars > 0) {
				font = richTextLinkFont;
				numLinkChars--;
			}
			if (map_charToFontGraphic[c] <= 0) {
				x += 6;
			} else {
				x += drawRichTextCharacter(font, c, x, y, color, measureOnly);
			}
		}
	}
}

static int getRichTextWordWidth(const unsigned char *str, int *outNumChars)
{
	int width = 0;
	int guard = 0;
	int wordCharSeen = 0;
	int numChars = 0;
	for (unsigned char c = *str; c; c = *(++str)) {
		if (++guard >= 2000) {
			break;
		}
		if (c == '@') {
			c = *(++str);
			if (!wordCharSeen) {
				if (c == 'P' || c == 'L') {
					numChars += 2;
					width = 0;
					break;
				} else if (c == 'G') {
					// skip graphic
					numChars += 2;
					while (c >= '0' && c <= '9') {
						c = *(++str);
						numChars++;
					}
					width = 0;
					break;
				} else {
					numChars++;
					while (c >= '0' && c <= '9') {
						c = *(++str);
						numChars++;
					}
				}
			}
		}
		if (c == ' ') {
			if (wordCharSeen) {
				break;
			}
			width += 4;
		} else if (c > ' ') {
			// normal char
			width += getCharacterWidth(c, richTextNormalFont);
			wordCharSeen = 1;
		}
		numChars++;
	}
	*outNumChars = numChars;
	return width;
}

static int drawRichTextCharacter(Font font, unsigned int c, int x, int y, Color color, int measureOnly)
{
	int graphicOffset = map_charToFontGraphic[c];
	if (!graphicOffset) {
		return 0;
	}

	int graphicId = GraphicId(ID_Graphic_Font) + font + graphicOffset - 1;
	int height = Data_Graphics_Main.index[graphicId].height - 11;
	if (height < 0) {
		height = 0;
	}
	if (c < 128 || c == 231) { // Some exceptions...
		height = 0;
	}
	if (!measureOnly) {
		Graphics_drawLetter(graphicId, x, y - height, color);
	}
	return Data_Graphics_Main.index[graphicId].width;
}

void Widget_RichText_drawScrollbar()
{
	if (data.maxScrollPosition) {
		Widget_Button_drawImageButtons(
			data.xText + 16 * data.textWidthBlocks - 1,
			data.yText,
			&imageButtonScrollUp, 1);
		Widget_Button_drawImageButtons(
			data.xText + 16 * data.textWidthBlocks - 1,
			data.yText + 16 * data.textHeightBlocks - 26,
			&imageButtonScrollDown, 1);
		Widget_RichText_drawScrollbarDot();
	}
}

void Widget_RichText_drawScrollbarDot()
{
	if (data.maxScrollPosition) {
		int pct;
		if (data.scrollPosition <= 0) {
			pct = 0;
		} else if (data.scrollPosition >= data.maxScrollPosition) {
			pct = 100;
		} else {
			pct = calc_percentage(data.scrollPosition, data.maxScrollPosition);
		}
		int offset = calc_adjust_with_percentage(16 * data.textHeightBlocks - 77, pct);
		if (data.isDraggingScroll) {
			offset = data.scrollPositionDrag;
		}
		Graphics_drawImage(GraphicId(ID_Graphic_PanelButton) + 39,
			data.xText + 16 * data.textWidthBlocks + 6, data.yText + offset + 26);
	}
}

void Widget_RichText_scroll(int isDown, int numLines)
{
	if (isDown) {
		data.scrollPosition += numLines;
		if (data.scrollPosition > data.maxScrollPosition) {
			data.scrollPosition = data.maxScrollPosition;
		}
	} else {
		data.scrollPosition -= numLines;
		if (data.scrollPosition < 0) {
			data.scrollPosition = 0;
		}
	}
	Widget_RichText_clearLinks();
	data.isDraggingScroll = 0;
	UI_Window_requestRefresh();
}

static int handleScrollbarDot()
{
	if (data.maxScrollPosition <= 0 || !Data_Mouse.left.isDown) {
		return 0;
	}
	int totalHeight = 16 * data.textHeightBlocks - 52;
	if (Data_Mouse.x < data.xText + 16 * data.textWidthBlocks + 1 ||
		Data_Mouse.x > data.xText + 16 * data.textWidthBlocks + 41) {
		return 0;
	}
	if (Data_Mouse.y < data.yText + 26 || Data_Mouse.y > data.yText + 26 + totalHeight) {
		return 0;
	}
	int dotHeight = Data_Mouse.y - data.yText - 11;
	if (dotHeight > totalHeight) {
		dotHeight = totalHeight;
	}
	int pctScrolled = calc_percentage(dotHeight, totalHeight);
	data.scrollPosition = calc_adjust_with_percentage(data.maxScrollPosition, pctScrolled);
	data.isDraggingScroll = 1;
	data.scrollPositionDrag = dotHeight - 25;
	Widget_RichText_clearLinks();
	if (data.scrollPositionDrag < 0) {
		data.scrollPositionDrag = 0;
	}
	UI_Window_requestRefresh();
	return 1;
}

int Widget_RichText_handleScrollbar()
{
	if (Data_Mouse.scrollDown) {
		Widget_RichText_scroll(1, 3);
	} else if (Data_Mouse.scrollUp) {
		Widget_RichText_scroll(0, 3);
	}

	if (Widget_Button_handleImageButtons(
		data.xText + 16 * data.textWidthBlocks - 1,
		data.yText,
		&imageButtonScrollUp, 1, 0)) {
			return 1;
	}
	if (Widget_Button_handleImageButtons(
		data.xText + 16 * data.textWidthBlocks - 1,
		data.yText + 16 * data.textHeightBlocks - 26,
		&imageButtonScrollDown, 1, 0)) {
			return 1;
	}
	return handleScrollbarDot();
}

int Widget_RichText_getScrollPosition()
{
	return data.scrollPosition;
}

int Widget_RichText_init(const char *str, int xText, int yText, int widthBlocks, int heightBlocks, int adjustWidthOnNoScroll)
{
	data.xText = xText;
	data.yText = yText;
	if (!data.numberOfLines) {
		data.textHeightBlocks = heightBlocks;
		data.textHeightLines = heightBlocks - 1;
		data.textWidthBlocks = widthBlocks;
		
		imageButtonScrollUp.enabled = 1;
		imageButtonScrollDown.enabled = 1;

		data.numberOfLines = Widget_RichText_draw(str,
			data.xText + 8, data.yText + 6,
			16 * data.textWidthBlocks - 16, data.textHeightLines, 1);
		if (data.numberOfLines <= data.textHeightLines) {
			if (adjustWidthOnNoScroll) {
				data.textWidthBlocks += 2;
			}
			data.maxScrollPosition = 0;
			imageButtonScrollUp.enabled = 0;
			imageButtonScrollDown.enabled = 0;
		} else {
			data.maxScrollPosition = data.numberOfLines - data.textHeightLines;
		}
		UI_Window_requestRefresh();
	}
	return data.textWidthBlocks;
}
