#ifndef WINDOW_INPUT_BOX_H
#define WINDOW_INPUT_BOX_H

#define INPUT_BOX_BLOCK_SIZE 16

typedef struct
{
    int x;
    int y;
    int w;
    int h;
} input_box;

void input_box_draw(const input_box *box);

#endif // WINDOW_INPUT_BOX_H
