#ifndef MAP_TERRAIN_H
#define MAP_TERRAIN_H

#include "core/buffer.h"

enum {
    TERRAIN_TREE = 1,
    TERRAIN_ROCK = 2,
    TERRAIN_WATER = 4,
    TERRAIN_BUILDING = 8,
    TERRAIN_SCRUB = 0x10,
    TERRAIN_GARDEN = 0x20,
    TERRAIN_ROAD = 0x40,
    TERRAIN_RESERVOIR_RANGE = 0x80,
    TERRAIN_AQUEDUCT = 0x100,
    TERRAIN_ELEVATION = 0x200,
    TERRAIN_ACCESS_RAMP = 0x400,
    TERRAIN_MEADOW = 0x800,
    TERRAIN_RUBBLE = 0x1000,
    TERRAIN_FOUNTAIN_RANGE = 0x2000,
    TERRAIN_WALL = 0x4000,
    TERRAIN_GATEHOUSE = 0x8000,
    // combined
    TERRAIN_WALL_OR_GATEHOUSE = TERRAIN_WALL | TERRAIN_GATEHOUSE,
    TERRAIN_NOT_CLEAR = 0xd77f,
    TERRAIN_CLEARABLE = 0xd17f,
    TERRAIN_IMPASSABLE = 0xc75f,
    TERRAIN_IMPASSABLE_ENEMY = 0x1237,
    TERRAIN_IMPASSABLE_WOLF = 0xd73f,
    TERRAIN_ALL = 0xffff
};

int map_terrain_is(int grid_offset, int terrain);

int map_terrain_get(int grid_offset);

void map_terrain_set(int grid_offset, int terrain);

void map_terrain_add(int grid_offset, int terrain);

void map_terrain_remove(int grid_offset, int terrain);

void map_terrain_remove_all(int terrain);

void map_terrain_backup();

void map_terrain_restore();

void map_terrain_clear();

void map_terrain_save_state(buffer *buf);

void map_terrain_load_state(buffer *buf);

#endif // MAP_TERRAIN_H
