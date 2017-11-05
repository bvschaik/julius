#ifndef SCREEN_H
#define SCREEN_H

#ifdef __cplusplus
extern "C" {
#endif


void Screen_setResolution(int width, int height);
void Screen_setResolutionWithPixels(int width, int height, void *pixels);


#ifdef __cplusplus
}
#endif

#endif
