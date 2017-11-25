#include "Scroll.h"

#include "core/time.h"

#include "Data/Screen.h"
#include "Data/State.h"

#include "core/direction.h"
#include "game/settings.h"

#define SCROLL_BORDER 5

static time_millis lastScrollTime = 0;

static int shouldScrollMap(const mouse *m)
{
	if (!m->is_inside_window) {
		return 0;
	}
	time_millis currentTime = time_get_millis();
	int diff = currentTime - lastScrollTime;
	if (currentTime < lastScrollTime) {
		diff = 10000;
	}
	int scrollDelay = (100 - setting_scroll_speed()) / 10;
	if (scrollDelay < 10) { // 0% = 10 = no scroll at all
		if (diff >= 12 * scrollDelay + 2) {
			lastScrollTime = currentTime;
			return 1;
		}
	}
	return 0;
}

int Scroll_getDirection(const mouse *m) {
	if (!shouldScrollMap(m)) {
		return DIR_8_NONE;
	}
	Data_State.isScrollingMap = 0;
	int top = 0;
	int bottom = 0;
	int left = 0;
	int right = 0;
	// mouse near map edge
	if (m->x < SCROLL_BORDER) {
		left = 1;
		Data_State.isScrollingMap = 1;
	}
	if (m->x >= Data_Screen.width - SCROLL_BORDER) {
		right = 1;
		Data_State.isScrollingMap = 1;
	}
	if (m->y < SCROLL_BORDER) {
		top = 1;
		Data_State.isScrollingMap = 1;
	}
	if (m->y >= Data_Screen.height - SCROLL_BORDER) {
		bottom = 1;
		Data_State.isScrollingMap = 1;
	}
	// keyboard arrow keys
	if (Data_State.arrowKey.left) {
		left = 1;
		Data_State.isScrollingMap = 1;
	}
	if (Data_State.arrowKey.right) {
		right = 1;
		Data_State.isScrollingMap = 1;
	}
	if (Data_State.arrowKey.up) {
		top = 1;
		Data_State.isScrollingMap = 1;
	}
	if (Data_State.arrowKey.down) {
		bottom = 1;
		Data_State.isScrollingMap = 1;
	}
	Data_State.arrowKey.left = 0;
	Data_State.arrowKey.right = 0;
	Data_State.arrowKey.up = 0;
	Data_State.arrowKey.down = 0;

	// two sides
	if (left && top) {
		return DIR_7_TOP_LEFT;
	} else if (left && bottom) {
		return DIR_5_BOTTOM_LEFT;
	} else if (right && top) {
		return DIR_1_TOP_RIGHT;
	} else if (right && bottom) {
		return DIR_3_BOTTOM_RIGHT;
	}
	// one side
	if (left) {
		return DIR_6_LEFT;
	} else if (right) {
		return DIR_2_RIGHT;
	} else if (top) {
		return DIR_0_TOP;
	} else if (bottom) {
		return DIR_4_BOTTOM;
	}
	// none of them
	return DIR_8_NONE;
}
