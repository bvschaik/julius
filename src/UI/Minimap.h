#ifndef UI_MINIMAP_H
#define UI_MINIMAP_H

#include "graphics/mouse.h"

void UI_Minimap_draw(int xOffset, int yOffset, int widthTiles, int heightTiles);

int UI_Minimap_handleClick(const mouse *m);

#endif
