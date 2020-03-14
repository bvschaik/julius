#ifndef WIDGET_INPUT_BOX_H
#define WIDGET_INPUT_BOX_H

#include "input/mouse.h"

#define INPUT_BOX_BLOCK_SIZE 16

typedef struct {
    int x;
    int y;
    int width_blocks;
    int height_blocks;
} input_box;

int input_box_handle_mouse(const mouse *m, const input_box *box);
void input_box_draw(const input_box *box);

#endif // WIDGET_INPUT_BOX_H
