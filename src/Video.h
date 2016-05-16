#ifndef VIDEO_H
#define VIDEO_H

int Video_start(const char* filename);

void Video_stop();

void Video_shutdown();

int Video_isFinished();

void Video_init();
void Video_draw(int xOffset, int yOffset);

#endif
