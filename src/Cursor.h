#ifndef CURSOR_H
#define CURSOR_H

enum {
	Cursor_Arrow = 0,
	Cursor_Shovel = 1,
	Cursor_Sword = 2,
};

struct Cursor {
	int hotspotX;
	int hotspotY;
	char data[32*32+1];
};

const struct Cursor *Cursor_getData(int cursorId);

void Cursor_set();

#endif
