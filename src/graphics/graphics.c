#include "graphics.h"

#include "graphics/screen.h"

void graphics_in_dialog()
{
    Graphics_setGlobalTranslation(screen_dialog_offset_x(), screen_dialog_offset_y());
}

void graphics_reset_dialog()
{
    Graphics_setGlobalTranslation(0, 0);
}
