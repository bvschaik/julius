#include "game/system.h"
#include "input/cursor.h"

#include "SDL.h"

static SDL_Cursor *cursors[CURSOR_MAX];

static SDL_Cursor *init_cursor(const cursor *c)
{
    Uint8 data[4*32];
    Uint8 mask[4*32];
    int b = -1;
    for (int i = 0; i < 32 * 32; i++) {
        if (i % 8 ) {
            data[b] <<= 1;
            mask[b] <<= 1;
        } else {
            b++;
            data[b] = mask[b] = 0;
        }
        switch (c->data[i]) {
            case 'X':
                data[b] |= 0x01;
                // fallthrough
            case '.':
                mask[b] |= 0x01;
                break;
        }
    }
    return SDL_CreateCursor(data, mask, 32, 32, c->hotspot_x, c->hotspot_y);
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
    SDL_SetCursor(cursors[cursor_id]);
}
