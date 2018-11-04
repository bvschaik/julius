#ifndef MAP_SPRITE_H
#define MAP_SPRITE_H

#include "core/buffer.h"

/**
 * @file
 * Grid to store bridge/animation sprites.
 * For the lack of better naming. This grid stores:
 * 1) for bridges: which bridge part is where
 * 2) for buildings: which offset in the animation cycle is currently shown
 */

int map_sprite_animation_at(int grid_offset);

void map_sprite_animation_set(int grid_offset, int value);

int map_sprite_bridge_at(int grid_offset);

void map_sprite_bridge_set(int grid_offset, int value);

void map_sprite_clear_tile(int grid_offset);

void map_sprite_clear(void);

void map_sprite_backup(void);

void map_sprite_restore(void);

void map_sprite_save_state(buffer *buf, buffer *backup);

void map_sprite_load_state(buffer *buf, buffer *backup);

#endif // MAP_SPRITE_H
