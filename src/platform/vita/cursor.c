#include <vita2d.h>
#include "graphics/color.h"
#include "game/system.h"
#include "input/cursor.h"
#include "platform/cursor.h"
#include "platform/vita/vita.h"

#include "SDL.h"

#define CURSOR_SIZE 32

static vita_cursor cursors[CURSOR_MAX];
vita_cursor *current_cursor;

static vita2d_texture *init_cursor(const cursor *c)
{
    vita2d_texture *tex = vita2d_create_empty_texture_format(CURSOR_SIZE, CURSOR_SIZE, SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ARGB);
    color_t *cursor_buf = vita2d_texture_get_datap(tex);

    for (int y = 0; y < c->height; y++) {
        for (int x = 0; x < c->width; x++) {
            switch (c->data[y * c->width + x]) {
                case '#':
                    cursor_buf[y * CURSOR_SIZE + x] = ALPHA_OPAQUE | COLOR_BLACK;
                    break;
                case '\'':
                    cursor_buf[y * CURSOR_SIZE + x] = ALPHA_OPAQUE | COLOR_WHITE;
                    break;
                case ' ':
                    cursor_buf[y * CURSOR_SIZE + x] = ALPHA_TRANSPARENT;
                    break;
            }
        }
    }

    return tex;
}

void platform_init_cursors(int scale_percentage)
{
    for (int i = 0; i < CURSOR_MAX; i++) {
        const cursor *c = input_cursor_data(i, CURSOR_SCALE_1);
        cursors[i].texture = init_cursor(c);
        cursors[i].hotspot_x = c->hotspot_x;
        cursors[i].hotspot_y = c->hotspot_y;
    }
    system_set_cursor(CURSOR_ARROW);
}

void system_set_cursor(int cursor_id)
{
    current_cursor = &cursors[cursor_id];
}
