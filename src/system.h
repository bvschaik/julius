#ifndef SYSTEM_H
#define SYSTEM_H

void System_resize(int width, int height);

void System_toggleFullscreen();

void System_initCursors();
void System_setCursor(int cursorId);

void System_exit();

#endif
