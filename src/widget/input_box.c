#include "input_box.h"

#include "graphics/panel.h"

void input_box_draw(const input_box *box)
{
    inner_panel_draw(box->x, box->y, box->w, box->h);
}
