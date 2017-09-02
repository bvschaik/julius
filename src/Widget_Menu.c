#include "Widget.h"
#include "Graphics.h"
#include "Data/Constants.h"
#include "Data/Mouse.h"

static int getMenuBarItem(MenuBarItem *items, int numItems);
static int getMenuItem(MenuBarItem *menu);

void Widget_Menu_drawMenuBar(MenuBarItem *items, int numItems)
{
	int xOffset = items[0].xStart;
	for (int i = 0; i < numItems; i++) {
		items[i].xStart = xOffset;
		xOffset += Widget_GameText_draw(items[i].textGroup, 0, xOffset, items[i].yStart, FONT_NORMAL_GREEN);
		items[i].xEnd = xOffset;
		xOffset += 32; // spacing
	}
}

void Widget_Menu_drawSubMenu(MenuBarItem *menu, int focusSubMenu)
{
	Widget_Panel_drawUnborderedPanel(menu->xStart, menu->yStart + 18,
		10, (20 + 20 * menu->numItems) / 16);
	for (int i = 0; i < menu->numItems; i++) {
		MenuItem *sub = &menu->items[i];
		int yOffset = 30 + menu->yStart + sub->yStart;
		if (i == focusSubMenu - 1) {
			Graphics_fillRect(menu->xStart, yOffset - 2,
				160, 16, COLOR_BLACK);
			Widget_GameText_drawColored(menu->textGroup, sub->textNumber,
				menu->xStart + 8, yOffset, FONT_NORMAL_PLAIN, COLOR_ORANGE
			);
		} else {
			Widget_GameText_draw(menu->textGroup, sub->textNumber,
				menu->xStart + 8, yOffset, FONT_NORMAL_BLACK
			);
		}
	}
}

int Widget_Menu_handleMenuBar(MenuBarItem *items, int numItems, int *focusMenuId)
{
	int menuId = getMenuBarItem(items, numItems);
	if (focusMenuId) {
		*focusMenuId = menuId;
	}
	if (!menuId) {
		return 0;
	}
	return menuId;
}

static int getMenuBarItem(MenuBarItem *items, int numItems)
{
	int mouseX = Data_Mouse.x;
	int mouseY = Data_Mouse.y;
	for (int i = 0; i < numItems; i++) {
		if (items[i].xStart <= mouseX &&
			items[i].xEnd > mouseX &&
			items[i].yStart <= mouseY &&
			items[i].yStart + 12 > mouseY) {
			return i + 1;
		}
	}
	return 0;
}

int Widget_Menu_handleMenuItem(MenuBarItem *menu, int *focusSubMenuId)
{
	int subMenuId = getMenuItem(menu);
	if (focusSubMenuId) {
		*focusSubMenuId = subMenuId;
	}
	if (!subMenuId) {
		return 0;
	}
	if (Data_Mouse.left.wentDown) {
		MenuItem *item = &menu->items[subMenuId-1];
		item->leftClickHandler(item->parameter);
	}
	return subMenuId;
}

static int getMenuItem(MenuBarItem *menu)
{
	int mouseX = Data_Mouse.x;
	int mouseY = Data_Mouse.y;
	for (int i = 0; i < menu->numItems; i++) {
		if (menu->xStart <= mouseX &&
			menu->xStart + 160 > mouseX &&
			menu->yStart + menu->items[i].yStart + 30 <= mouseY &&
			menu->yStart + menu->items[i].yStart + 45 > mouseY) {
			return i + 1;
		}
	}
	return 0;
}
