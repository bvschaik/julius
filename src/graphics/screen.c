#include "screen.h"

#include "../Graphics.h"
#include "Data/Screen.h"

#include "city/view.h"
#include "city/warning.h"

#include <stdlib.h>
#include <string.h>

static struct {
    struct {
        int x;
        int y;
    } dialog_offset;
} data;

static void set_size(int width, int height)
{
    Data_Screen.width = width;
    Data_Screen.height = height;
    data.dialog_offset.x = (Data_Screen.width - 640) / 2;
    data.dialog_offset.y = (Data_Screen.height - 480) / 2;
    Graphics_setClipRectangle(0, 0, width, height);
    city_view_set_viewport(width, height);
    city_warning_clear_all();
}

void screen_set_resolution(int width, int height)
{
    if (Data_Screen.drawBuffer) {
        free(Data_Screen.drawBuffer);
    }
    Data_Screen.drawBuffer = malloc(width * height * sizeof(color_t));
    memset(Data_Screen.drawBuffer, 0, width * height * sizeof(color_t));
    set_size(width, height);
}

int screen_width()
{
    return Data_Screen.width;
}

int screen_height()
{
    return Data_Screen.height;
}

int screen_dialog_offset_x()
{
    return data.dialog_offset.x;
}

int screen_dialog_offset_y()
{
    return data.dialog_offset.y;
}
