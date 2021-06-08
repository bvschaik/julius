#include "panel.h"

#include "graphics/image.h"

void outer_panel_draw(int x, int y, int width_blocks, int height_blocks)
{
    int image_base = image_group(GROUP_DIALOG_BACKGROUND);
    int image_id;
    int image_y = 0;
    int y_add = 0;
    for (int yy = 0; yy < height_blocks; yy++) {
        int image_x = 0;
        for (int xx = 0; xx < width_blocks; xx++) {
            if (yy == 0) {
                if (xx == 0) {
                    image_id = 0;
                } else if (xx < width_blocks - 1) {
                    image_id = 1 + image_x++;
                } else {
                    image_id = 11;
                }
                y_add = 0;
            } else if (yy < height_blocks - 1) {
                if (xx == 0) {
                    image_id = 12 + image_y;
                } else if (xx < width_blocks - 1) {
                    image_id = 13 + image_y + image_x++;
                } else {
                    image_id = 23 + image_y;
                }
                y_add = 12;
            } else {
                if (xx == 0) {
                    image_id = 132;
                } else if (xx < width_blocks - 1) {
                    image_id = 133 + image_x++;
                } else {
                    image_id = 143;
                }
                y_add = 0;
            }
            image_draw(image_base + image_id, x + BLOCK_SIZE * xx, y + BLOCK_SIZE * yy);
            if (image_x >= 10) {
                image_x = 0;
            }
        }
        image_y += y_add;
        if (image_y >= 120) {
            image_y = 0;
        }
    }
}

void unbordered_panel_draw(int x, int y, int width_blocks, int height_blocks)
{
    int image_base = image_group(GROUP_DIALOG_BACKGROUND);
    int image_y = 0;
    for (int yy = 0; yy < height_blocks; yy++) {
        int image_x = 0;
        for (int xx = 0; xx < width_blocks; xx++) {
            int image_id = 13 + image_y + image_x++;
            image_draw(image_base + image_id, x + BLOCK_SIZE * xx, y + BLOCK_SIZE * yy);
            if (image_x >= 10) {
                image_x = 0;
            }
        }
        image_y += 12;
        if (image_y >= 120) {
            image_y = 0;
        }
    }
}

void inner_panel_draw(int x, int y, int width_blocks, int height_blocks)
{
    int image_base = image_group(GROUP_SUNKEN_TEXTBOX_BACKGROUND);
    int image_y = 0;
    int y_add = 0;
    for (int yy = 0; yy < height_blocks; yy++) {
        int image_x = 0;
        for (int xx = 0; xx < width_blocks; xx++) {
            int image_id;
            if (yy == 0) {
                if (xx == 0) {
                    image_id = 0;
                } else if (xx < width_blocks - 1) {
                    image_id = 1 + image_x++;
                } else {
                    image_id = 6;
                }
                y_add = 0;
            } else if (yy < height_blocks - 1) {
                if (xx == 0) {
                    image_id = 7 + image_y;
                } else if (xx < width_blocks - 1) {
                    image_id = 8 + image_y + image_x++;
                } else {
                    image_id = 13 + image_y;
                }
                y_add = 7;
            } else {
                if (xx == 0) {
                    image_id = 42;
                } else if (xx < width_blocks - 1) {
                    image_id = 43 + image_x++;
                } else {
                    image_id = 48;
                }
                y_add = 0;
            }
            image_draw(image_base + image_id, x + BLOCK_SIZE * xx, y + BLOCK_SIZE * yy);
            if (image_x >= 5) {
                image_x = 0;
            }
        }
        image_y += y_add;
        if (image_y >= 35) {
            image_y = 0;
        }
    }
}

void label_draw(int x, int y, int width_blocks, int type)
{
    int image_base = image_group(GROUP_PANEL_BUTTON);
    for (int i = 0; i < width_blocks; i++) {
        int image_id;
        if (i == 0) {
            image_id = 3 * type + 40;
        } else if (i < width_blocks - 1) {
            image_id = 3 * type + 41;
        } else {
            image_id = 3 * type + 42;
        }
        image_draw(image_base + image_id, x + BLOCK_SIZE * i, y);
    }
}

void large_label_draw(int x, int y, int width_blocks, int type)
{
    int image_base = image_group(GROUP_PANEL_BUTTON);
    for (int i = 0; i < width_blocks; i++) {
        int image_id;
        if (i == 0) {
            image_id = 3 * type;
        } else if (i < width_blocks - 1) {
            image_id = 3 * type + 1;
        } else {
            image_id = 3 * type + 2;
        }
        image_draw(image_base + image_id, x + BLOCK_SIZE * i, y);
    }
}
