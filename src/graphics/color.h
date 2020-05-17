#ifndef GRAPHICS_COLOR_H
#define GRAPHICS_COLOR_H

#include <stdint.h>

typedef uint32_t color_t;

#define COLOR_BLACK 0x000000
#define COLOR_RED 0xff0000
#define COLOR_WHITE 0xffffff

#define COLOR_SG2_TRANSPARENT 0xf700ff
#define COLOR_TOOLTIP 0x424242
#define COLOR_SIDEBAR 0xbdb592

#define COLOR_FONT_RED COLOR_RED
#define COLOR_FONT_BLUE 0x0055ff
#define COLOR_FONT_YELLOW 0xe7e75a
#define COLOR_FONT_ORANGE 0xff5a08
#define COLOR_FONT_ORANGE_LIGHT 0xffa500
#define COLOR_FONT_LIGHT_GRAY 0xb3b3b3

#define COLOR_INSET_LIGHT 0xffffff
#define COLOR_INSET_DARK 0x848484

#define COLOR_MASK_NONE 0xffffff
#define COLOR_MASK_RED 0xff0818
#define COLOR_MASK_GREEN 0x18ff18
#define COLOR_MASK_BLUE 0x663377ff
#define COLOR_MASK_GREEN_LEGION 0x4400ff00

#define COLOR_MINIMAP_VIEWPORT 0xe7e75a
#define COLOR_MINIMAP_DARK 0x424242
#define COLOR_MINIMAP_LIGHT 0xc6c6c6
#define COLOR_MINIMAP_SOLDIER 0xf70000
#define COLOR_MINIMAP_ENEMY_CENTRAL 0x7b0000
#define COLOR_MINIMAP_ENEMY_NORTHERN 0x1800ff
#define COLOR_MINIMAP_ENEMY_DESERT 0x08007b
#define COLOR_MINIMAP_WOLF COLOR_BLACK

#define COLOR_MOUSE_DARK_GRAY 0x3f3f3f
#define COLOR_MOUSE_MEDIUM_GRAY 0x737373
#define COLOR_MOUSE_LIGHT_GRAY 0xb3b3b3

#define ALPHA_OPAQUE 0xff000000
#define ALPHA_FONT_SEMI_TRANSPARENT 0x99000000
#define ALPHA_MASK_SEMI_TRANSPARENT 0x48000000
#define ALPHA_TRANSPARENT 0x00000000

#endif // GRAPHICS_COLOR_H
