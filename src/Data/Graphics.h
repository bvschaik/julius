#ifndef DATA_GRAPHICS_H
#define DATA_GRAPHICS_H

#include "Types.h"
#include "graphics/image.h"

#define GraphicId(index) image_group(index)
#define GraphicHeight(id) (image_get(id)->height)
#define GraphicWidth(id) (image_get(id)->width)
#define GraphicAnimationSpeed(id) (image_get(id)->animation_speed_id)
#define GraphicNumAnimationSprites(id) (image_get(id)->num_animation_sprites)
#define GraphicAnimationCanReverse(id) (image_get(id)->animation_can_reverse)
#define GraphicSpriteOffsetX(id) (image_get(id)->sprite_offset_x)
#define GraphicSpriteOffsetY(id) (image_get(id)->sprite_offset_y)
#define GraphicEnemySpriteOffsetX(id) (image_get_enemy(id)->sprite_offset_x)
#define GraphicEnemySpriteOffsetY(id) (image_get_enemy(id)->sprite_offset_y)

#endif
