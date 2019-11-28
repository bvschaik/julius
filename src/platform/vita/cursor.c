#include <vita2d.h>
#include "graphics/color.h"
#include "game/system.h"
#include "input/cursor.h"
#include "platform/cursor.h"

#include "SDL.h"

#define CURSOR_SIZE 32
#define ALPHA_OPAQUE (0xFFu << 24)

vita2d_texture *cursors[CURSOR_MAX];
vita2d_texture *current_cursor;

static vita2d_texture *init_cursor(const cursor *c)
{
    vita2d_texture *tex = vita2d_create_empty_texture_format(CURSOR_SIZE, CURSOR_SIZE, SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ARGB);
    color_t *cursor_buf = vita2d_texture_get_datap(tex);

    for (int y = 0; y < c->height; y++) {
        for (int x = 0; x < c->width; x++) {
            switch (c->data[y * c->width + x]) {
                case '#':
                    cursor_buf[y * CURSOR_SIZE + x] = COLOR_BLACK | ALPHA_OPAQUE;
                    break;
                case '\'':
                    cursor_buf[y * CURSOR_SIZE + x] = COLOR_WHITE | ALPHA_OPAQUE;
                    break;
                case ' ':
                    break; // Transparent is 0x00000000 which is the default value of the buffer
            }
        }
    }

    return tex;
}

void platform_init_cursors(int scale_percentage)
{
    for (int i = 0; i < CURSOR_MAX; i++) {
        cursors[i] = init_cursor(input_cursor_data(i, CURSOR_SCALE_1));
    }
    system_set_cursor(CURSOR_ARROW);
}

void system_set_cursor(int cursor_id)
{
    current_cursor = cursors[cursor_id];
}
