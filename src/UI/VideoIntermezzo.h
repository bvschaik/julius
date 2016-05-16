#ifndef VIDEOINTERMEZZO_H
#define VIDEOINTERMEZZO_H

#include "Window.h"

void VideoIntermezzo_show(const char* filename, int width, int height, WindowId nextWindowId);

void VideoIntermezzo_init();
void VideoIntermezzo_drawBackground();
void VideoIntermezzo_drawForeground();
void VideoIntermezzo_handleMouse();

#endif
