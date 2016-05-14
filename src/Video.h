#ifndef VIDEO_H
#define VIDEO_H

#include "UI/Window.h"

int Video_start(const char* filename, int xOffset, int yOffset, int fullscreen, WindowId nextWindowId);

void Video_cancel();

#endif
