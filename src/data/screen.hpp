#ifndef DATA_SCREEN_H
#define DATA_SCREEN_H

#define ScreenPixel(x,y) ((color_t*)Data_Screen.drawBuffer)[(y) * Data_Screen.width + (x)]

extern struct _Data_Screen
{
    int width;
    int height;
    struct
    {
        int x, y;
    } offset640x480;
    void *drawBuffer; //[800*600];
} Data_Screen;

#endif
