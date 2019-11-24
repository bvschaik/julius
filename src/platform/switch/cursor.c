#include "graphics/color.h"
#include "game/system.h"
#include "input/cursor.h"
#include "platform/cursor.h"

#include "SDL.h"

static SDL_Texture *cursors[CURSOR_MAX];
SDL_Texture *current_cursor;

extern struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} SDL;

static SDL_Texture *init_cursor(const cursor *c)
{
    SDL_Texture *tex = SDL_CreateTexture(SDL.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 32, 32);

    uint32_t pixels[32 * 32];

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            switch (c->data[i * 32 + j]) {
                case 'X':
                    pixels[i * 32 + j] =  0xFF000000; //black
                    break;
                case '.':
                    pixels[i * 32 + j] = 0xFFFFFFFF; //white
                    break;
                case ' ':
                    pixels[i * 32 + j] = 0x00000000; //transparent
                    break; // Transparent is 0x00000000 which is the default value of the buffer
            }
        }
    }

    SDL_UpdateTexture(tex, NULL, pixels, 32 * 4);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

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
