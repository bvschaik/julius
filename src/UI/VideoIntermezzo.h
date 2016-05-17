#ifndef VIDEOINTERMEZZO_H
#define VIDEOINTERMEZZO_H

#include "Window.h"

void UI_VideoIntermezzo_show(const char* filename, int width, int height, WindowId nextWindowId);

void UI_VideoIntermezzo_init();
void UI_VideoIntermezzo_drawBackground();
void UI_VideoIntermezzo_drawForeground();
void UI_VideoIntermezzo_handleMouse();

#endif
