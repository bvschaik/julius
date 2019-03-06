#include <vita2d.h>
#include "graphics/color.h"
#include "game/system.h"
#include "input/cursor.h"

#include "SDL.h"

vita2d_texture *cursors[CURSOR_MAX];
vita2d_texture *current_cursor;

static vita2d_texture *init_cursor(const cursor *c)
{
    vita2d_texture *tex = vita2d_create_empty_texture_format(32, 32, SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ARGB);
    color_t *cursor_buf = vita2d_texture_get_datap(tex);

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            switch (c->data[i * 32 + j]) {
                case 'X':
                    cursor_buf[i * 32 + j] = COLOR_BLACK | 0xFFu << 24;
                    break;
                case '.':
                    cursor_buf[i * 32 + j] = COLOR_WHITE | 0xFFu << 24;
                    break;
                case ' ':
                    break; // Transparent is 0x00000000 which is the default value of the buffer
            }
        }
    }

    return tex;
}

void system_init_cursors(void)
{
    for (int i = 0; i < CURSOR_MAX; i++) {
        cursors[i] = init_cursor(input_cursor_data(i));
    }
    system_set_cursor(CURSOR_ARROW);
}

void system_set_cursor(int cursor_id)
{
    current_cursor = cursors[cursor_id];
}
