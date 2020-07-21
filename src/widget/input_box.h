#ifndef WIDGET_INPUT_BOX_H
#define WIDGET_INPUT_BOX_H

#include "graphics/font.h"
#include "input/mouse.h"

#define INPUT_BOX_BLOCK_SIZE 16

typedef struct {
    int x;
    int y;
    int width_blocks;
    int height_blocks;
    font_t font;
    int max_length;
    // private vars
    uint8_t *text;
} input_box;

void input_box_start(input_box *box, uint8_t *text, int length, int allow_punctuation);
void input_box_pause(input_box *box);
void input_box_resume(input_box *box);
void input_box_stop(input_box *box);

void input_box_refresh_text(input_box *box);
int input_box_is_accepted(input_box *box);

int input_box_handle_mouse(const mouse *m, const input_box *box);
void input_box_draw(const input_box *box);

#endif // WIDGET_INPUT_BOX_H
