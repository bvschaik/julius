#ifndef WIDGET_INPUT_BOX_H
#define WIDGET_INPUT_BOX_H

#include "graphics/font.h"
#include "input/mouse.h"

typedef struct {
    int x;
    int y;
    int width_blocks;
    int height_blocks;
    font_t font;
    int allow_punctuation;
    uint8_t *text;
    int text_length;
    int put_clear_button_outside_box;
    const uint8_t *placeholder;
    void (*on_change)(int is_addition_at_end);
    uint8_t *old_text;
} input_box;

/**
 * This will start text input. The `text` variable of the box will be used to capture
 * input until @link input_box_stop @endlink is called
 * @param box Input box
 */
void input_box_start(input_box *box);
void input_box_pause(void);
void input_box_resume(void);
void input_box_stop(input_box *box);

void input_box_refresh_text(input_box *box);
int input_box_is_accepted(void);

int input_box_handle_mouse(const mouse *m, const input_box *box);
void input_box_draw(const input_box *box);

#endif // WIDGET_INPUT_BOX_H
