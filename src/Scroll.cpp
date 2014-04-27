#include "Scroll.h"

#include "Time.h"

#include "Data/Constants.h"
#include "Data/Mouse.h"
#include "Data/Screen.h"
#include "Data/Settings.h"
#include "Data/State.h"

#define SCROLL_BORDER 10

static TimeMillis lastScrollTime = 0;

static int shouldScrollMap()
{
	TimeMillis currentTime = Time_getMillis();
	int diff = currentTime - lastScrollTime;
	if (currentTime < lastScrollTime) {
		diff = 10000;
	}
	int scrollDelay = (100 - Data_Settings.scrollSpeed) / 10;
	if (scrollDelay < 10) { // 0% = 10 = no scroll at all
		if (diff >= 12 * scrollDelay + 2) {
			lastScrollTime = currentTime;
			return 1;
		}
	}
	return 0;
}

int Scroll_getDirection() {
	if (!shouldScrollMap()) {
		return Direction_None;
	}
	Data_State.isScrollingMap = 0;
	int top = 0;
	int bottom = 0;
	int left = 0;
	int right = 0;
	if (Data_Mouse.x < SCROLL_BORDER) {
		left = 1;
		Data_State.isScrollingMap = 1;
	}
	if (Data_Mouse.x >= Data_Screen.width - SCROLL_BORDER) {
		right = 1;
		Data_State.isScrollingMap = 1;
	}
	if (Data_Mouse.y < SCROLL_BORDER) {
		top = 1;
		Data_State.isScrollingMap = 1;
	}
	if (Data_Mouse.y >= Data_Screen.height - SCROLL_BORDER) {
		bottom = 1;
		Data_State.isScrollingMap = 1;
	}
	// TODO keyboard arrow keys
	// two sides
	if (left && top) {
		return Direction_TopLeft;
	} else if (left && bottom) {
		return Direction_BottomLeft;
	} else if (right && top) {
		return Direction_TopRight;
	} else if (right && bottom) {
		return Direction_BottomRight;
	}
	// one side
	if (left) {
		return Direction_Left;
	} else if (right) {
		return Direction_Right;
	} else if (top) {
		return Direction_Top;
	} else if (bottom) {
		return Direction_Bottom;
	}
	// none of them
	return Direction_None;
}
