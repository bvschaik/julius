#include "Widget.h"

#include "core/calc.h"
#include "Graphics.h"
#include "UI/Window.h"

#include "core/lang.h"
#include "core/string.h"
#include "graphics/image_button.h"

static uint8_t tmpLine[200];

static struct {
	int capture;
	int seen;
	int position;
    int cursor_position;
	int width;
	int visible;
	time_millis updated;
	int xOffset;
	int yOffset;
} inputCursor;

void Widget_Text_captureCursor(int cursor_position)
{
	inputCursor.capture = 1;
	inputCursor.seen = 0;
	inputCursor.position = 0;
	inputCursor.width = 0;
    inputCursor.cursor_position = cursor_position;
}

void Widget_Text_drawCursor(int xOffset, int yOffset, int isInsert)
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
		if (isInsert) {
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
		} else {
			Graphics_fillRect(
				xOffset + inputCursor.xOffset, yOffset + inputCursor.yOffset + 14,
				inputCursor.width, 2, COLOR_WHITE);
		}
	}
}

int Widget_Text_getWidth(const uint8_t *str, font_t font)
{
	const font_definition *def = font_definition_for(font);
	int maxlen = 10000;
	int width = 0;
	int graphicBase = image_group(GROUP_FONT);
	while (*str && maxlen > 0) {
		if (*str == ' ') {
			width += def->space_width;
		} else {
			int graphicOffset = font_image_for(*str);
			if (graphicOffset) {
				int graphicId = graphicBase + def->image_offset + graphicOffset - 1;
				width += def->letter_spacing + image_get(graphicId)->width;
			}
		}
		str++;
		maxlen--;
	}
	return width;
}

int Widget_GameText_getWidth(int group, int number, font_t font)
{
	const uint8_t *str = lang_get_string(group, number);
	return Widget_Text_getWidth(str, font);
}

int Widget_GameText_getDrawWidth(int group, int number, font_t font)
{
	const uint8_t *str = lang_get_string(group, number);
	return Widget_Text_getWidth(str, font) + font_definition_for(font)->space_width;
}

static int getCharacterWidth(uint8_t c, const font_definition *def)
{
	int graphicOffset = font_image_for(c);
	if (!graphicOffset) {
		return 0;
	}
	int graphicId = image_group(GROUP_FONT) + def->image_offset + graphicOffset - 1;
	return 1 + image_get(graphicId)->width;
}

static int getWordWidth(const uint8_t *str, font_t font, int *outNumChars)
{
    const font_definition *def = font_definition_for(font);
	int width = 0;
	int guard = 0;
	int wordCharSeen = 0;
	int numChars = 0;
	for (uint8_t c = *str; c; c = *(++str)) {
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
			width += getCharacterWidth(c, def);
			wordCharSeen = 1;
		}
		numChars++;
	}
	*outNumChars = numChars;
	return width;
}

void Widget_Text_drawCentered(const uint8_t *str, int x, int y, int boxWidth, font_t font, color_t color)
{
	int offset = (boxWidth - Widget_Text_getWidth(str, font)) / 2;
	if (offset < 0) {
		offset = 0;
	}
	Widget_Text_draw(str, offset + x, y, font, color);
}

static int drawCharacter(const font_definition *def, unsigned int c, int x, int y, color_t color)
{
    int graphicOffset = font_image_for(c);
    int graphicId = image_group(GROUP_FONT) + def->image_offset + graphicOffset - 1;
    int height = image_get(graphicId)->height - def->line_height;
    if (height < 0) {
        height = 0;
    }
    if (c < 128 || c == 231) { // Some exceptions...
        height = 0;
    }
    Graphics_drawLetter(graphicId, x, y - height, color);
    return image_get(graphicId)->width;
}

int Widget_Text_draw(const uint8_t *str, int x, int y, font_t font, color_t color)
{
    const font_definition *def = font_definition_for(font);

	int currentX = x;
	while (*str) {
		uint8_t c = *str;

		if (c == '_') {
			c = ' ';
		}

		if (c >= ' ') {
			int width = 0;
			int graphic = font_image_for(c);
			if (graphic == 0) {
				width = def->space_width_draw;
			} else {
				width = def->letter_spacing_draw + drawCharacter(def, c, currentX, y, color);
			}
			if (inputCursor.capture && inputCursor.position == inputCursor.cursor_position) {
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
	currentX += def->space_width_draw;
	return currentX - x;
}

static void numberToString(uint8_t *str, int value, char prefix, const char *postfix)
{
	int offset = 0;
	if (prefix) {
		str[offset++] = prefix;
	}
	offset += string_from_int(&str[offset], value, 0);
	while (*postfix) {
		str[offset++] = *postfix;
		postfix++;
	}
	str[offset] = 0;
}

int Widget_Text_drawNumber(int value, char prefix, const char *postfix, int xOffset, int yOffset, font_t font)
{
	uint8_t str[100];
	numberToString(str, value, prefix, postfix);
	return Widget_Text_draw(str, xOffset, yOffset, font, 0);
}

int Widget_Text_drawNumberColored(int value, char prefix, const char *postfix, int xOffset, int yOffset, font_t font, color_t color)
{
	uint8_t str[100];
	numberToString(str, value, prefix, postfix);
	return Widget_Text_draw(str, xOffset, yOffset, font, color);
}

int Widget_Text_drawMoney(int value, int xOffset, int yOffset, font_t font)
{
    uint8_t str[100];
    numberToString(str, value, '@', " Dn");
    return Widget_Text_draw(str, xOffset, yOffset, font, 0);
}

int Widget_Text_drawPercentage(int value, int xOffset, int yOffset, font_t font)
{
    uint8_t str[100];
    numberToString(str, value, '@', "%");
    return Widget_Text_draw(str, xOffset, yOffset, font, 0);
}

void Widget_Text_drawNumberCentered(int value, int xOffset, int yOffset, int boxWidth, font_t font)
{
	uint8_t str[100];
	numberToString(str, value, '@', " ");
	Widget_Text_drawCentered(str, xOffset, yOffset, boxWidth, font, 0);
}

void Widget_Text_drawNumberCenteredColored(int value, int xOffset, int yOffset, int boxWidth, font_t font, color_t color)
{
	uint8_t str[100];
	numberToString(str, value, '@', " ");
	Widget_Text_drawCentered(str, xOffset, yOffset, boxWidth, font, color);
}


int Widget_GameText_draw(int group, int number, int xOffset, int yOffset, font_t font)
{
	const uint8_t *str = lang_get_string(group, number);
	return Widget_Text_draw(str, xOffset, yOffset, font, 0);
}

int Widget_GameText_drawColored(int group, int number, int xOffset, int yOffset, font_t font, color_t color)
{
    const uint8_t *str = lang_get_string(group, number);
	return Widget_Text_draw(str, xOffset, yOffset, font, color);
}

void Widget_GameText_drawCentered(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font)
{
    const uint8_t *str = lang_get_string(group, number);
	Widget_Text_drawCentered(str, xOffset, yOffset, boxWidth, font, 0);
}

void Widget_GameText_drawCenteredColored(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font, color_t color)
{
    const uint8_t *str = lang_get_string(group, number);
	Widget_Text_drawCentered(str, xOffset, yOffset, boxWidth, font, color);
}

int Widget_GameText_drawNumberWithDescription(int group, int number, int amount, int xOffset, int yOffset, font_t font)
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

int Widget_GameText_drawYear(int year, int xOffset, int yOffset, font_t font)
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

int Widget_GameText_drawYearColored(int year, int xOffset, int yOffset, font_t font, color_t color)
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

int Widget_GameText_drawYearNoSpacing(int year, int xOffset, int yOffset, font_t font)
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

int Widget_Text_drawMultiline(const uint8_t *str, int xOffset, int yOffset, int boxWidth, font_t font)
{
	int lineHeight;
	switch (font) {
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
			int wordWidth = getWordWidth(str, font, &wordNumChars);
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

int Widget_GameText_drawMultiline(int group, int number, int xOffset, int yOffset, int boxWidth, font_t font)
{
    const uint8_t *str = lang_get_string(group, number);
	return Widget_Text_drawMultiline(str, xOffset, yOffset, boxWidth, font);
}


// RICH TEXT

#define MAX_LINKS 50

static void drawRichTextLine(const uint8_t *str, int x, int y, color_t color, int measureOnly);
static int getRichTextWordWidth(const font_definition *def, const uint8_t *str, int *outNumChars);
static int drawRichTextCharacter(const font_definition *def, unsigned int c, int x, int y, color_t color, int measureOnly);

static image_button imageButtonScrollUp = {
	0, 0, 39, 26, IB_SCROLL, 96, 8, Widget_RichText_scroll, button_none, 0, 1, 1
};
static image_button imageButtonScrollDown = {
	0, 0, 39, 26, IB_SCROLL, 96, 12, Widget_RichText_scroll, button_none, 1, 1, 1
};

static struct RichTextLink {
	int messageId;
	int xMin;
	int yMin;
	int xMax;
	int yMax;
} links[MAX_LINKS];

static int numLinks;
static font_t richTextNormalFont = FONT_NORMAL_WHITE;
static font_t richTextLinkFont = FONT_NORMAL_RED;

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

void Widget_RichText_setFonts(font_t normalFont, font_t linkFont)
{
	richTextNormalFont = normalFont;
	richTextLinkFont = linkFont;
}

int Widget_RichText_getClickedLink(const mouse *m)
{
	if (m->left.went_down) {
		for (int i = 0; i < numLinks; i++) {
			if (m->x >= links[i].xMin && m->x <= links[i].xMax &&
				m->y >= links[i].yMin && m->y <= links[i].yMax) {
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

static int drawRichText(const uint8_t *str, int xOffset, int yOffset,
						int boxWidth, int heightLines, color_t color, int measureOnly)
{
    const font_definition *def = font_definition_for(richTextNormalFont);
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
			currentWidth += getRichTextWordWidth(def, str, &wordNumChars);
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
							graphicId = string_to_int(str);
							c = *(str++);
							while (c >= '0' && c <= '9') {
								c = *(str++);
							}
							graphicId += image_group(GROUP_MESSAGE_IMAGES) - 1;
							graphicHeightLines = image_get(graphicId)->height / 16 + 2;
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
			drawRichTextLine(tmpLine, xLineOffset + xOffset, y, color, measureOnly);
		}
		if (!measureOnly) {
			if (graphicId) {
				if (linesBeforeGraphic) {
					linesBeforeGraphic--;
				} else {
				    const image *img = image_get(graphicId);
					graphicHeightLines = img->height / 16 + 2;
					int xOffsetGraphic = xOffset + (boxWidth - img->width) / 2 - 4;
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

int Widget_RichText_draw(const uint8_t *str, int xOffset, int yOffset,
						 int boxWidth, int heightLines, int measureOnly)
{
	return drawRichText(str, xOffset, yOffset, boxWidth, heightLines, 0, measureOnly);
}

int Widget_RichText_drawColored(const uint8_t *str, int xOffset, int yOffset, int boxWidth, int heightLines, color_t color)
{
	return drawRichText(str, xOffset, yOffset, boxWidth, heightLines, color, 0);
}

static void drawRichTextLine(const uint8_t *str, int x, int y, color_t color, int measureOnly)
{
    const font_definition *normal_def = font_definition_for(richTextNormalFont);
    const font_definition *link_def = font_definition_for(richTextLinkFont);
	int numLinkChars = 0;
	for (uint8_t c = *str; c; c = *(++str)) {
		if (c == '@') {
			int messageId = string_to_int(++str);
			while (*str >= '0' && *str <= '9') {
				str++;
			}
			int width = getRichTextWordWidth(normal_def, str, &numLinkChars);
			addRichTextLink(messageId, x, x + width, y);
			c = *str;
		}
		if (c >= ' ') {
			const font_definition *def = normal_def;
			if (numLinkChars > 0) {
				def = link_def;
				numLinkChars--;
			}
			if (font_image_for(c) <= 0) {
				x += 6;
			} else {
				x += drawRichTextCharacter(def, c, x, y, color, measureOnly);
			}
		}
	}
}

static int getRichTextWordWidth(const font_definition *def, const uint8_t *str, int *outNumChars)
{
	int width = 0;
	int guard = 0;
	int wordCharSeen = 0;
	int numChars = 0;
	for (uint8_t c = *str; c; c = *(++str)) {
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
			width += getCharacterWidth(c, def);
			wordCharSeen = 1;
		}
		numChars++;
	}
	*outNumChars = numChars;
	return width;
}

static int drawRichTextCharacter(const font_definition *def, unsigned int c, int x, int y, color_t color, int measureOnly)
{
	int graphicOffset = font_image_for(c);
	if (!graphicOffset) {
		return 0;
	}

	int graphicId = image_group(GROUP_FONT) + def->image_offset + graphicOffset - 1;
	int height = image_get(graphicId)->height - 11;
	if (height < 0) {
		height = 0;
	}
	if (c < 128 || c == 231) { // Some exceptions...
		height = 0;
	}
	if (!measureOnly) {
		Graphics_drawLetter(graphicId, x, y - height, color);
	}
	return image_get(graphicId)->width;
}

void Widget_RichText_drawScrollbar()
{
	if (data.maxScrollPosition) {
		image_buttons_draw(
			data.xText + 16 * data.textWidthBlocks - 1,
			data.yText,
			&imageButtonScrollUp, 1);
		image_buttons_draw(
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
		Graphics_drawImage(image_group(GROUP_PANEL_BUTTON) + 39,
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

static int handleScrollbarDot(const mouse *m)
{
	if (data.maxScrollPosition <= 0 || !m->left.is_down) {
		return 0;
	}
	int totalHeight = 16 * data.textHeightBlocks - 52;
	if (m->x < data.xText + 16 * data.textWidthBlocks + 1 ||
		m->x > data.xText + 16 * data.textWidthBlocks + 41) {
		return 0;
	}
	if (m->y < data.yText + 26 || m->y > data.yText + 26 + totalHeight) {
		return 0;
	}
	int dotHeight = m->y - data.yText - 11;
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

int Widget_RichText_handleScrollbar(const mouse *m)
{
	if (m->scrolled == SCROLL_DOWN) {
		Widget_RichText_scroll(1, 3);
	} else if (m->scrolled == SCROLL_UP) {
		Widget_RichText_scroll(0, 3);
	}

	if (image_buttons_handle_mouse(
		m, data.xText + 16 * data.textWidthBlocks - 1, data.yText,
		&imageButtonScrollUp, 1, 0)) {
			return 1;
	}
	if (image_buttons_handle_mouse(m,
		data.xText + 16 * data.textWidthBlocks - 1,
		data.yText + 16 * data.textHeightBlocks - 26,
		&imageButtonScrollDown, 1, 0)) {
			return 1;
	}
	return handleScrollbarDot(m);
}

int Widget_RichText_getScrollPosition()
{
	return data.scrollPosition;
}

int Widget_RichText_init(const uint8_t *str, int xText, int yText, int widthBlocks, int heightBlocks, int adjustWidthOnNoScroll)
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
