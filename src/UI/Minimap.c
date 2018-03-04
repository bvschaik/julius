#include "Minimap.h"
#include "../Data/CityView.h"
#include "../Data/State.h"

#include "building/building.h"
#include "city/view.h"
#include "figure/figure.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/property.h"
#include "map/random.h"
#include "map/terrain.h"
#include "scenario/property.h"
#include "widget/sidebar.h"

static void setBounds(int xOffset, int yOffset, int widthTiles, int heightTiles);
static void drawMinimap(int xOffset, int yOffset, int widthTiles, int heightTiles);
static int drawFigure(int xView, int yView, int gridOffset);
static void drawTile(int xView, int yView, int gridOffset);
static void drawViewportRectangle(int xView, int yView, int widthTiles, int heightTiles);
static int getMouseGridOffset(const mouse *m, int xOffset, int yOffset, int widthTiles, int heightTiles);

static int minimapAbsoluteX;
static int minimapAbsoluteY;
static int minimapLeft;
static int minimapTop;
static int minimapRight;
static int minimapBottom;
static color_t soldierColor;
static color_t enemyColor;

static struct {
    
    struct {
        int x;
        int y;
        int grid_offset;
    } mouse;
} data;

void foreach_map_tile(int xOffset, int yOffset, int widthTiles, int heightTiles,
                      void (*callback)(int xView, int yView, int gridOffset))
{
    int odd = 0;
    int yAbs = minimapAbsoluteY - 4;
    int yView = yOffset - 4;
    for (int yRel = -4; yRel < heightTiles + 4; yRel++, yAbs++, yView++) {
        int xView;
        if (odd) {
            xView = xOffset - 9;
            odd = 0;
        } else {
            xView = xOffset - 8;
            odd = 1;
        }
        int xAbs = minimapAbsoluteX - 4;
        for (int xRel = -4; xRel < widthTiles; xRel++, xAbs++, xView += 2) {
            if (xAbs < 0 || xAbs >= VIEW_X_MAX) continue;
            if (yAbs < 0 || yAbs >= VIEW_Y_MAX) continue;
            callback(xView, yView, ViewToGridOffset(xAbs, yAbs));
        }
    }    
}

void UI_Minimap_draw(int xOffset, int yOffset, int widthTiles, int heightTiles)
{
	graphics_set_clip_rectangle(xOffset, yOffset, 2 * widthTiles, heightTiles);
	
	soldierColor = COLOR_SOLDIER;
    switch (scenario_property_climate()) {
        case CLIMATE_CENTRAL: enemyColor = COLOR_ENEMY_CENTRAL; break;
        case CLIMATE_NORTHERN: enemyColor = COLOR_ENEMY_NORTHERN; break;
        default: enemyColor = COLOR_ENEMY_DESERT; break;
    }

	setBounds(xOffset, yOffset, widthTiles, heightTiles);
	drawMinimap(xOffset, yOffset, widthTiles, heightTiles);
	drawViewportRectangle(xOffset, yOffset, widthTiles, heightTiles);

	graphics_reset_clip_rectangle();
}

static void setBounds(int xOffset, int yOffset, int widthTiles, int heightTiles)
{
	minimapAbsoluteX = (VIEW_X_MAX - widthTiles) / 2;
	minimapAbsoluteY = (VIEW_Y_MAX - heightTiles) / 2;
	minimapLeft = xOffset;
	minimapTop = yOffset;
	minimapRight = xOffset + 2 * widthTiles;
	minimapBottom = yOffset + heightTiles;

	if ((Data_State.map.width - widthTiles) / 2 > 0) {
		if (Data_CityView.xInTiles < minimapAbsoluteX) {
			minimapAbsoluteX = Data_CityView.xInTiles;
		} else if (Data_CityView.xInTiles > widthTiles + minimapAbsoluteX - Data_CityView.widthInTiles) {
			minimapAbsoluteX = Data_CityView.widthInTiles + Data_CityView.xInTiles - widthTiles;
		}
	}
	if ((2 * Data_State.map.height - heightTiles) / 2 > 0) {
		if (Data_CityView.yInTiles < minimapAbsoluteY) {
			minimapAbsoluteY = Data_CityView.yInTiles;
		} else if (Data_CityView.yInTiles > heightTiles + minimapAbsoluteY - Data_CityView.heightInTiles) {
			minimapAbsoluteY = Data_CityView.heightInTiles + Data_CityView.yInTiles - heightTiles;
		}
	}
	// ensure even height
	minimapAbsoluteY &= ~1;
}

static void drawMinimap(int xOffset, int yOffset, int widthTiles, int heightTiles)
{
    foreach_map_tile(xOffset, yOffset, widthTiles, heightTiles, drawTile);
}

enum {
    FIGURE_COLOR_NONE = 0,
    FIGURE_COLOR_SOLDIER = 1,
    FIGURE_COLOR_ENEMY = 2,
    FIGURE_COLOR_WOLF = 3
};

static int has_figure_color(figure *f)
{
    int type = f->type;
    if (figure_is_legion(f)) {
        return FIGURE_COLOR_SOLDIER;
    }
    if (figure_is_enemy(f)) {
        return FIGURE_COLOR_ENEMY;
    }
    if (f->type == FIGURE_INDIGENOUS_NATIVE &&
        f->actionState == FIGURE_ACTION_159_NATIVE_ATTACKING) {
        return FIGURE_COLOR_ENEMY;
    }
    if (type == FIGURE_WOLF) {
        return FIGURE_COLOR_WOLF;
    }
    return FIGURE_COLOR_NONE;
}

static int drawFigure(int xView, int yView, int gridOffset)
{
    int color_type = map_figure_foreach_until(gridOffset, has_figure_color);
    if (color_type == FIGURE_COLOR_NONE) {
        return 0;
    }
    color_t color = COLOR_BLACK;
    if (color_type == FIGURE_COLOR_SOLDIER) {
        color = soldierColor;
    } else if (color_type == FIGURE_COLOR_ENEMY) {
        color = enemyColor;
    }
    graphics_draw_line(xView, yView, xView+1, yView, color);
    return 1;
}

static void drawTile(int xView, int yView, int gridOffset)
{
	if (gridOffset < 0) {
		image_draw(image_group(GROUP_MINIMAP_BLACK), xView, yView);
		return;
	}

	if (drawFigure(xView, yView, gridOffset)) {
		return;
	}
	
	int terrain = map_terrain_get(gridOffset);
	// exception for fort ground: display as empty land
	if (terrain & TERRAIN_BUILDING) {
		if (building_get(map_building_at(gridOffset))->type == BUILDING_FORT_GROUND) {
			terrain = 0;
		}
	}

	if (terrain & TERRAIN_BUILDING) {
		if (map_property_is_draw_tile(gridOffset)) {
			int graphicId;
			building *b = building_get(map_building_at(gridOffset));
			if (b->houseSize) {
				graphicId = image_group(GROUP_MINIMAP_HOUSE);
			} else if (b->type == BUILDING_RESERVOIR) {
				graphicId = image_group(GROUP_MINIMAP_AQUEDUCT) - 1;
			} else {
				graphicId = image_group(GROUP_MINIMAP_BUILDING);
			}
			switch (map_property_multi_tile_size(gridOffset)) {
				case 1:
					image_draw(graphicId, xView, yView); break;
				case 2:
					image_draw(graphicId + 1, xView, yView - 1); break;
				case 3:
					image_draw(graphicId + 2, xView, yView - 2); break;
				case 4:
					image_draw(graphicId + 3, xView, yView - 3); break;
				case 5:
					image_draw(graphicId + 4, xView, yView - 4); break;
			}
		}
	} else {
		int rand = map_random_get(gridOffset);
		int graphicId;
		if (terrain & TERRAIN_WATER) {
			graphicId = image_group(GROUP_MINIMAP_WATER) + (rand & 3);
		} else if (terrain & TERRAIN_SCRUB) {
			graphicId = image_group(GROUP_MINIMAP_TREE) + (rand & 3);
		} else if (terrain & TERRAIN_TREE) {
			graphicId = image_group(GROUP_MINIMAP_TREE) + (rand & 3);
		} else if (terrain & TERRAIN_ROCK) {
			graphicId = image_group(GROUP_MINIMAP_ROCK) + (rand & 3);
		} else if (terrain & TERRAIN_ELEVATION) {
			graphicId = image_group(GROUP_MINIMAP_ROCK) + (rand & 3);
		} else if (terrain & TERRAIN_ROAD) {
			graphicId = image_group(GROUP_MINIMAP_ROAD);
		} else if (terrain & TERRAIN_AQUEDUCT) {
			graphicId = image_group(GROUP_MINIMAP_AQUEDUCT);
		} else if (terrain & TERRAIN_WALL) {
			graphicId = image_group(GROUP_MINIMAP_WALL);
		} else if (terrain & TERRAIN_MEADOW) {
			graphicId = image_group(GROUP_MINIMAP_MEADOW) + (rand & 3);
		} else {
			graphicId = image_group(GROUP_MINIMAP_EMPTY_LAND) + (rand & 7);
		}
		image_draw(graphicId, xView, yView);
	}
}

static void drawViewportRectangle(int xView, int yView, int widthTiles, int heightTiles)
{
	int xOffset = xView + 2 * (Data_CityView.xInTiles - minimapAbsoluteX) - 2;
	if (xOffset < xView) {
		xOffset = xView;
	}
	if (xOffset + 2 * Data_CityView.widthInTiles + 4 > xView + widthTiles) {
		xOffset -= 2;
	}
	int yOffset = yView + Data_CityView.yInTiles - minimapAbsoluteY + 2;
	graphics_draw_rect(xOffset, yOffset,
		Data_CityView.widthInTiles * 2 + 4,
		Data_CityView.heightInTiles - 4,
		COLOR_YELLOW);
}

static void update_mouse_grid_offset(int x_view, int y_view, int grid_offset)
{
    if (data.mouse.y == y_view && (data.mouse.x == x_view || data.mouse.x == x_view + 1)) {
        data.mouse.grid_offset = grid_offset < 0 ? 0 : grid_offset;
    }
}

static int getMouseGridOffset(const mouse *m, int xOffset, int yOffset, int widthTiles, int heightTiles)
{
    data.mouse.x = m->x;
    data.mouse.y = m->y;
    data.mouse.grid_offset = 0;
    foreach_map_tile(xOffset, yOffset, widthTiles, heightTiles, update_mouse_grid_offset);
    return data.mouse.grid_offset;
}

static int isMinimapClick(const mouse *m)
{
	if (m->x >= minimapLeft && m->x < minimapRight &&
		m->y >= minimapTop && m->y < minimapBottom) {
		return 1;
	}
	return 0;
}

int UI_Minimap_handleClick(const mouse *m)
{
	if (isMinimapClick(m)) {
		if (m->left.went_down || m->right.went_down) {
			int gridOffset = getMouseGridOffset(m, minimapLeft, minimapTop, 73, 111);
			if (gridOffset > 0) {
				city_view_go_to_grid_offset(gridOffset);
				widget_sidebar_invalidate_minimap();
				return 1;
			}
		}
	}
	return 0;
}
