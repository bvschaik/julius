#include "destruction.h"

#include "figuretype/missile.h"
#include "game/undo.h"
#include "map/building_tiles.h"
#include "map/routing_terrain.h"
#include "sound/effect.h"

#include "../Building.h"

void building_destroy_collapse(building *b)
{
    b->state = BuildingState_Rubble;
    map_building_tiles_set_rubble(b->id, b->x, b->y, b->size);
    figure_create_explosion_cloud(b->x, b->y, b->size);
    Building_collapseLinked(b->id, 0);
}

int building_destroy_first_of_type(building_type type)
{
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (BuildingIsInUse(b) && b->type == type) {
            int grid_offset = b->gridOffset;
            game_undo_disable();
            b->state = BuildingState_Rubble;
            map_building_tiles_set_rubble(i, b->x, b->y, b->size);
            sound_effect_play(SOUND_EFFECT_EXPLOSION);
            map_routing_update_land();
            return grid_offset;
        }
    }
    return 0;
}
