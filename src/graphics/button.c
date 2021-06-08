#include "button.h"

#include "graphics/image.h"
#include "graphics/panel.h"

void button_none(int param1, int param2)
{
}

void button_border_draw(int x, int y, int width_pixels, int height_pixels, int has_focus)
{
    int width_blocks = width_pixels / BLOCK_SIZE;
    if (width_pixels % BLOCK_SIZE) {
        width_blocks++;
    }
    int height_blocks = height_pixels / BLOCK_SIZE;
    if (height_pixels % BLOCK_SIZE) {
        height_blocks++;
    }
    int last_block_offset_x = BLOCK_SIZE * width_blocks - width_pixels;
    int last_block_offset_y = BLOCK_SIZE * height_blocks - height_pixels;

    int image_base = image_group(GROUP_BORDERED_BUTTON);
    if (has_focus) {
        image_base += 8;
    }

    for (int yy = 0; yy < height_blocks; yy++) {
        int draw_offset_y = y + BLOCK_SIZE * yy;
        for (int xx = 0; xx < width_blocks; xx++) {
            int draw_offset_x = x + BLOCK_SIZE * xx;
            if (yy == 0) {
                if (xx == 0) {
                    image_draw(image_base, draw_offset_x, draw_offset_y);
                } else if (xx < width_blocks - 1) {
                    image_draw(image_base + 1, draw_offset_x, draw_offset_y);
                } else {
                    image_draw(image_base + 2, draw_offset_x - last_block_offset_x, draw_offset_y);
                }
            } else if (yy < height_blocks - 1) {
                if (xx == 0) {
                    image_draw(image_base + 7, draw_offset_x, draw_offset_y);
                } else if (xx >= width_blocks - 1) {
                    image_draw(image_base + 3, draw_offset_x - last_block_offset_x, draw_offset_y);
                }
            } else {
                if (xx == 0) {
                    image_draw(image_base + 6, draw_offset_x, draw_offset_y - last_block_offset_y);
                } else if (xx < width_blocks - 1) {
                    image_draw(image_base + 5, draw_offset_x, draw_offset_y - last_block_offset_y);
                } else {
                    image_draw(image_base + 4,
                        draw_offset_x - last_block_offset_x, draw_offset_y - last_block_offset_y);
                }
            }
        }
    }
}
