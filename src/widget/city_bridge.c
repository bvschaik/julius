#include "city_bridge.h"

#include "graphics/image.h"
#include "map/property.h"
#include "map/sprite.h"
#include "map/terrain.h"

void city_draw_bridge(int x, int y, float scale, int grid_offset)
{
    if (!map_terrain_is(grid_offset, TERRAIN_WATER)) {
        map_sprite_clear_tile(grid_offset);
        return;
    }
    if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
        return;
    }
    color_t color_mask = 0;
    if (map_property_is_deleted(grid_offset)) {
        color_mask = COLOR_MASK_RED;
    }
    city_draw_bridge_tile(x, y, scale, map_sprite_bridge_at(grid_offset), color_mask);
}

void city_draw_bridge_tile(int x, int y, float scale, int bridge_sprite_id, color_t color_mask)
{
    int image_id = image_group(GROUP_BUILDING_BRIDGE);
    switch (bridge_sprite_id) {
        case 1:
            image_draw(image_id + 5, x, y - 20, color_mask, scale);
            break;
        case 2:
            image_draw(image_id, x - 1, y - 8, color_mask, scale);
            break;
        case 3:
            image_draw(image_id + 3, x, y - 8, color_mask, scale);
            break;
        case 4:
            image_draw(image_id + 2, x + 7, y - 20, color_mask, scale);
            break;
        case 5:
            image_draw(image_id + 4, x , y - 21, color_mask, scale);
            break;
        case 6:
            image_draw(image_id + 1, x + 5, y - 21, color_mask, scale);
            break;
        case 7:
            image_draw(image_id + 11, x - 3, y - 50, color_mask, scale);
            break;
        case 8:
            image_draw(image_id + 6, x - 1, y - 12, color_mask, scale);
            break;
        case 9:
            image_draw(image_id + 9, x - 30, y - 12, color_mask, scale);
            break;
        case 10:
            image_draw(image_id + 8, x - 23, y - 53, color_mask, scale);
            break;
        case 11:
            image_draw(image_id + 10, x, y - 37, color_mask, scale);
            break;
        case 12:
            image_draw(image_id + 7, x + 7, y - 38, color_mask, scale);
            break;
            // Note: no nr 13
        case 14:
            image_draw(image_id + 13, x, y - 38, color_mask, scale);
            break;
        case 15:
            image_draw(image_id + 12, x + 7, y - 38, color_mask, scale);
            break;
    }
}
