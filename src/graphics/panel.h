#ifndef GRAPHICS_PANEL_H
#define GRAPHICS_PANEL_H

#define BLOCK_SIZE 16

void outer_panel_draw(int x, int y, int width_blocks, int height_blocks);

void inner_panel_draw(int x, int y, int width_blocks, int height_blocks);

void unbordered_panel_draw(int x, int y, int width_blocks, int height_blocks);

void label_draw(int x, int y, int width_blocks, int type);

void large_label_draw(int x, int y, int width_blocks, int type);

#endif // GRAPHICS_PANEL_H
