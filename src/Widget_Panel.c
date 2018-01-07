#include "Widget.h"

#include "Graphics.h"

void Widget_Panel_drawOuterPanel(int xOffset, int yOffset, int widthInBlocks, int heightInBlocks)
{
	int graphicBase = image_group(GROUP_DIALOG_BACKGROUND);
	int graphicId;
	int graphicY = 0;
	int yAdd = 0;
	for (int y = 0; y < heightInBlocks; y++) {
		int graphicX = 0;
		for (int x = 0; x < widthInBlocks; x++) {
			if (y == 0) {
				if (x == 0) {
					graphicId = 0;
				} else if (x < widthInBlocks - 1) {
					graphicId = 1 + graphicX++;
				} else {
					graphicId = 11;
				}
				yAdd = 0;
			} else if (y < heightInBlocks - 1) {
				if (x == 0) {
					graphicId = 12 + graphicY;
				} else if (x < widthInBlocks - 1) {
					graphicId = 13 + graphicY + graphicX++;
				} else {
					graphicId = 23 + graphicY;
				}
				yAdd = 12;
			} else {
				if (x == 0) {
					graphicId = 132;
				} else if (x < widthInBlocks - 1) {
					graphicId = 133 + graphicX++;
				} else {
					graphicId = 143;
				}
				yAdd = 0;
			}
			Graphics_drawImage(
				graphicBase + graphicId,
				xOffset + 16 * x,
				yOffset + 16 * y);
			if (graphicX >= 10) {
				graphicX = 0;
			}
		}
		graphicY += yAdd;
		if (graphicY >= 120) {
			graphicY = 0;
		}
	}
}

void Widget_Panel_drawUnborderedPanel(int xOffset, int yOffset, int widthInBlocks, int heightInBlocks)
{
	int graphicBase = image_group(GROUP_DIALOG_BACKGROUND);
	int graphicY = 0;
	for (int y = 0; y < heightInBlocks; y++) {
		int graphicX = 0;
		for (int x = 0; x < widthInBlocks; x++) {
			int graphicId = 13 + graphicY + graphicX++;
			Graphics_drawImage(
				graphicBase + graphicId,
				xOffset + 16 * x,
				yOffset + 16 * y);
			if (graphicX >= 10) {
				graphicX = 0;
			}
		}
		graphicY += 12;
		if (graphicY >= 120) {
			graphicY = 0;
		}
	}
}

void Widget_Panel_drawInnerPanel(int xOffset, int yOffset, int widthInBlocks, int heightInBlocks)
{
	int graphicBase = image_group(GROUP_SUNKEN_TEXTBOX_BACKGROUND);
	int graphicId;
	int graphicY = 0;
	int yAdd = 0;
	for (int y = 0; y < heightInBlocks; y++) {
		int graphicX = 0;
		for (int x = 0; x < widthInBlocks; x++) {
			if (y == 0) {
				if (x == 0) {
					graphicId = 0;
				} else if (x < widthInBlocks - 1) {
					graphicId = 1 + graphicX++;
				} else {
					graphicId = 6;
				}
				yAdd = 0;
			} else if (y < heightInBlocks - 1) {
				if (x == 0) {
					graphicId = 7 + graphicY;
				} else if (x < widthInBlocks - 1) {
					graphicId = 8 + graphicY + graphicX++;
				} else {
					graphicId = 13 + graphicY;
				}
				yAdd = 7;
			} else {
				if (x == 0) {
					graphicId = 42;
				} else if (x < widthInBlocks - 1) {
					graphicId = 43 + graphicX++;
				} else {
					graphicId = 48;
				}
				yAdd = 0;
			}
			Graphics_drawImage(
				graphicBase + graphicId,
				xOffset + 16 * x,
				yOffset + 16 * y);
			if (graphicX >= 5) {
				graphicX = 0;
			}
		}
		graphicY += yAdd;
		if (graphicY >= 35) {
			graphicY = 0;
		}
	}
}

void Widget_Panel_drawLargeLabelButton(int xOffset, int yOffset, int widthInBlocks, int type)
{
	int graphicBase = image_group(GROUP_PANEL_BUTTON);
	for (int i = 0; i < widthInBlocks; i++) {
		int graphicId;
		if (i == 0) {
			graphicId = 3 * type;
		} else if (i < widthInBlocks - 1) {
			graphicId = 3 * type + 1;
		} else {
			graphicId = 3 * type + 2;
		}
		Graphics_drawImage(graphicBase + graphicId, xOffset + 16 * i, yOffset);
	}
}

void Widget_Panel_drawSmallLabelButton(int xOffset, int yOffset, int widthInBlocks, int type)
{
	int graphicBase = image_group(GROUP_PANEL_BUTTON);
	for (int i = 0; i < widthInBlocks; i++) {
		int graphicId;
		if (i == 0) {
			graphicId = 3 * type + 40;
		} else if (i < widthInBlocks - 1) {
			graphicId = 3 * type + 41;
		} else {
			graphicId = 3 * type + 42;
		}
		Graphics_drawImage(graphicBase + graphicId, xOffset + 16 * i, yOffset);
	}
}
