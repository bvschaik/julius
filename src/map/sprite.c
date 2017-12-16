#include "sprite.h"

#include "map/grid.h"

static grid_u8 sprite;
static grid_u8 sprite_backup;

int map_sprite_animation_at(int grid_offset)
{
    return sprite.items[grid_offset];
}

void map_sprite_animation_set(int grid_offset, int value)
{
    sprite.items[grid_offset] = value;
}

int map_sprite_bridge_at(int grid_offset)
{
    return sprite.items[grid_offset];
}

void map_sprite_bridge_set(int grid_offset, int value)
{
    sprite.items[grid_offset] = value;
}

void map_sprite_clear_tile(int grid_offset)
{
    sprite.items[grid_offset] = 0;
}

void map_sprite_clear()
{
    map_grid_clear_u8(sprite.items);
}

void map_sprite_backup()
{
    map_grid_copy_u8(sprite.items, sprite_backup.items);
}

void map_sprite_restore()
{
    map_grid_copy_u8(sprite_backup.items, sprite.items);
}

void map_sprite_save_state(buffer *buf, buffer *backup)
{
    map_grid_save_state_u8(sprite.items, buf);
    map_grid_save_state_u8(sprite_backup.items, backup);
}

void map_sprite_load_state(buffer *buf, buffer *backup)
{
    map_grid_load_state_u8(sprite.items, buf);
    map_grid_load_state_u8(sprite_backup.items, backup);
}
