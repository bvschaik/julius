#include "graphics.h"

#include "Data/Screen.h"
#include "../Graphics.h"

void graphics_in_dialog()
{
    Graphics_setGlobalTranslation(Data_Screen.offset640x480.x, Data_Screen.offset640x480.y);
    Graphics_setClipRectangle(0, 0, 640, 480);
}

void graphics_reset_dialog()
{
    Graphics_setGlobalTranslation(0, 0);
    Graphics_resetClipRectangle();
}
