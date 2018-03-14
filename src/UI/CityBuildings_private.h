#ifndef UI_CITYBUILDINGS_PRIVATE_H
#define UI_CITYBUILDINGS_PRIVATE_H

#include "graphics/tooltip.h"

#define DRAWFOOT_SIZE1(g,x,y) image_draw_isometric_footprint(g, x, y, 0)
#define DRAWFOOT_SIZE2(g,x,y) image_draw_isometric_footprint(g, x + 30, y - 15, 0)
#define DRAWFOOT_SIZE3(g,x,y) image_draw_isometric_footprint(g, x + 60, y - 30, 0)
#define DRAWFOOT_SIZE4(g,x,y) image_draw_isometric_footprint(g, x + 90, y - 45, 0)
#define DRAWFOOT_SIZE5(g,x,y) image_draw_isometric_footprint(g, x + 120, y - 60, 0)

#define DRAWTOP_SIZE1(g,x,y) image_draw_isometric_top(g, x, y, 0)
#define DRAWTOP_SIZE3(g,x,y) image_draw_isometric_top(g, x + 60, y - 30, 0)
#define DRAWTOP_SIZE2(g,x,y) image_draw_isometric_top(g, x + 30, y - 15, 0)
#define DRAWTOP_SIZE4(g,x,y) image_draw_isometric_top(g, x + 90, y - 45, 0)
#define DRAWTOP_SIZE5(g,x,y) image_draw_isometric_top(g, x + 120, y - 60, 0)

#define DRAWTOP_SIZE1_C(g,x,y,c) image_draw_isometric_top(g, x, y, c)
#define DRAWTOP_SIZE2_C(g,x,y,c) image_draw_isometric_top(g, x + 30, y - 15, c)
#define DRAWTOP_SIZE3_C(g,x,y,c) image_draw_isometric_top(g, x + 60, y - 30, c)
#define DRAWTOP_SIZE4_C(g,x,y,c) image_draw_isometric_top(g, x + 90, y - 45, c)
#define DRAWTOP_SIZE5_C(g,x,y,c) image_draw_isometric_top(g, x + 120, y - 60, c)

void UI_CityBuildings_drawSelectedBuildingGhost();

#endif
