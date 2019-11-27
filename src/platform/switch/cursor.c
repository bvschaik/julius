#include "graphics/color.h"
#include "game/system.h"
#include "input/cursor.h"
#include "platform/cursor.h"

#include "SDL.h"

#define CURSOR_SIZE 32
#define ALPHA_OPAQUE (0xFFu << 24)

static SDL_Texture *cursors[CURSOR_MAX];
SDL_Texture *current_cursor;

extern struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} SDL;

static SDL_Texture *init_cursor(const cursor *c)
{
    SDL_Texture *tex = SDL_CreateTexture(SDL.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, CURSOR_SIZE, CURSOR_SIZE);

    uint32_t pixels[CURSOR_SIZE * CURSOR_SIZE];

    for (int y = 0; y < c->height; y++) {
        for (int x = 0; x < c->width; x++) {
            switch (c->data[y * c->width + x]) {
                case '#':
                    pixels[y * CURSOR_SIZE + x] = COLOR_BLACK | ALPHA_OPAQUE;
                    break;
                case '\'':
                    pixels[y * CURSOR_SIZE + x] = COLOR_WHITE | ALPHA_OPAQUE;
                    break;
                case ' ':
                    pixels[y * CURSOR_SIZE + x] = 0x00000000; // transparent
                    break;
            }
        }
    }

    SDL_UpdateTexture(tex, NULL, pixels, CURSOR_SIZE * 4);
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
