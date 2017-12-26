#include "missile.h"

#include "figure/formation.h"
#include "figure/properties.h"
#include "figure/sound.h"
#include "graphics/image.h"
#include "map/figure.h"
#include "map/point.h"
#include "sound/effect.h"

#include "Data/State.h"
#include "../Formation.h"
#include "FigureMovement.h"

static const int CLOUD_TILE_OFFSETS[] = {0, 0, 0, 1, 1, 2};

static const int CLOUD_CC_OFFSETS[] = {0, 7, 14, 7, 14, 7};

static const int CLOUD_SPEED[] = {
    1, 2, 1, 3, 2, 1, 3, 2, 1, 1, 2, 1, 2, 1, 3, 1
};

static const map_point CLOUD_DIRECTION[] = {
    {0, -6}, {-2, -5}, {-4, -4}, {-5, -2}, {-6, 0}, {-5, -2}, {-4, -4}, {-2, -5},
    {0, -6}, {-2, -5}, {-4, -4}, {-5, -2}, {-6, 0}, {-5, -2}, {-4, -4}, {-2, -5}
};

static const int CLOUD_IMAGE_OFFSETS[] = {
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2,
    2, 2, 2, 2, 3, 3, 3, 4, 4, 5, 6, 7
};

void figure_create_explosion_cloud(int x, int y, int size)
{
    int tileOffset = CLOUD_TILE_OFFSETS[size];
    int ccOffset = CLOUD_CC_OFFSETS[size];
    for (int i = 0; i < 16; i++) {
        figure *f = figure_create(FIGURE_EXPLOSION,
            x + tileOffset, y + tileOffset, DIR_0_TOP);
        if (f->id) {
            f->crossCountryX += ccOffset;
            f->crossCountryY += ccOffset;
            f->destinationX += CLOUD_DIRECTION[i].x;
            f->destinationY += CLOUD_DIRECTION[i].y;
            FigureMovement_crossCountrySetDirection(f,
                f->crossCountryX, f->crossCountryY,
                15 * f->destinationX + ccOffset,
                15 * f->destinationY + ccOffset, 0);
            f->speedMultiplier = CLOUD_SPEED[i];
        }
    }
    sound_effect_play(SOUND_EFFECT_EXPLOSION);
}

void figure_create_missile(int building_id, int x, int y, int x_dst, int y_dst, figure_type type)
{
    figure *f = figure_create(type, x, y, DIR_0_TOP);
    if (f->id) {
        f->missileDamage = (type == FIGURE_BOLT) ? 60 : 10;
        f->buildingId = building_id;
        f->destinationX = x_dst;
        f->destinationY = y_dst;
        FigureMovement_crossCountrySetDirection(
            f, f->crossCountryX, f->crossCountryY,
            15 * x_dst, 15 * y_dst, 1);
    }
}

static int is_citizen(figure *f)
{
    if (f->actionState != FIGURE_ACTION_149_CORPSE) {
        if (f->type && f->type != FIGURE_EXPLOSION && f->type != FIGURE_FORT_STANDARD &&
            f->type != FIGURE_MAP_FLAG && f->type != FIGURE_FLOTSAM && f->type < FIGURE_INDIGENOUS_NATIVE) {
            return f->id;
        }
    }
    return 0;
}

static int get_citizen_on_tile(int grid_offset)
{
    return map_figure_foreach_until(grid_offset, is_citizen);
}

static int is_non_citizen(figure *f)
{
    if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        return 0;
    }
    if (FigureIsEnemy(f->type)) {
        return f->id;
    }
    if (f->type == FIGURE_INDIGENOUS_NATIVE && f->actionState == FIGURE_ACTION_159_NATIVE_ATTACKING) {
        return f->id;
    }
    if (f->type == FIGURE_WOLF || f->type == FIGURE_SHEEP || f->type == FIGURE_ZEBRA) {
        return f->id;
    }
    return 0;
}

static int get_non_citizen_on_tile(int grid_offset)
{
    return map_figure_foreach_until(grid_offset, is_non_citizen);
}

void figure_explosion_cloud_action(figure *f)
{
    f->useCrossCountry = 1;
    f->progressOnTile++;
    if (f->progressOnTile > 44) {
        f->state = FigureState_Dead;
    }
    FigureMovement_crossCountryWalkTicks(f, f->speedMultiplier);
    if (f->progressOnTile < 48) {
        f->graphicId = image_group(GROUP_FIGURE_EXPLOSION) +
                       CLOUD_IMAGE_OFFSETS[f->progressOnTile / 2];
    } else {
        f->graphicId = image_group(GROUP_FIGURE_EXPLOSION) + 7;
    }
}

static void missile_hit_target(figure *f, int target_id, figure_type legionary_type)
{
    figure *target = figure_get(target_id);
    const figure_properties *target_props = figure_properties_for_type(target->type);
    int max_damage = target_props->max_damage;
    int damage_inflicted =
        figure_properties_for_type(f->type)->missile_attack_value -
        target_props->missile_defense_value;
    const formation *m = formation_get(target->formationId);
    if (damage_inflicted < 0) {
        damage_inflicted = 0;
    }
    if (target->type == legionary_type && m->is_halted && m->layout == FORMATION_COLUMN) {
        damage_inflicted = 1;
    }
    int target_damage = damage_inflicted + target->damage;
    if (target_damage <= max_damage) {
        target->damage = target_damage;
    } else { // kill target
        target->damage = max_damage + 1;
        target->actionState = FIGURE_ACTION_149_CORPSE;
        target->waitTicks = 0;
        figure_play_die_sound(target->type);
        Formation_updateAfterDeath(m->id);
    }
    f->state = FigureState_Dead;
    // for missiles: building_id contains the figure who shot it
    int missile_formation = figure_get(f->buildingId)->formationId;
    formation_record_missile_attack(m->id, missile_formation);
}

void figure_arrow_action(figure *f)
{
    f->useCrossCountry = 1;
    f->progressOnTile++;
    if (f->progressOnTile > 120) {
        f->state = FigureState_Dead;
    }
    int should_die = FigureMovement_crossCountryWalkTicks(f, 4);
    int target_id = get_citizen_on_tile(f->gridOffset);
    if (target_id) {
        missile_hit_target(f, target_id, FIGURE_FORT_LEGIONARY);
        sound_effect_play(SOUND_EFFECT_ARROW_HIT);
    } else if (should_die) {
        f->state = FigureState_Dead;
    }
    int dir = (16 + f->direction - 2 * Data_State.map.orientation) % 16;
    f->graphicId = image_group(GROUP_FIGURE_MISSILE) + 16 + dir;
}

void figure_spear_action(figure *f)
{
    f->useCrossCountry = 1;
    f->progressOnTile++;
    if (f->progressOnTile > 120) {
        f->state = FigureState_Dead;
    }
    int should_die = FigureMovement_crossCountryWalkTicks(f, 4);
    int target_id = get_citizen_on_tile(f->gridOffset);
    if (target_id) {
        missile_hit_target(f, target_id, FIGURE_FORT_LEGIONARY);
        sound_effect_play(SOUND_EFFECT_JAVELIN);
    } else if (should_die) {
        f->state = FigureState_Dead;
    }
    int dir = (16 + f->direction - 2 * Data_State.map.orientation) % 16;
    f->graphicId = image_group(GROUP_FIGURE_MISSILE) + dir;
}

void figure_javelin_action(figure *f)
{
    f->useCrossCountry = 1;
    f->progressOnTile++;
    if (f->progressOnTile > 120) {
        f->state = FigureState_Dead;
    }
    int should_die = FigureMovement_crossCountryWalkTicks(f, 4);
    int target_id = get_non_citizen_on_tile(f->gridOffset);
    if (target_id) {
        missile_hit_target(f, target_id, FIGURE_ENEMY_CAESAR_LEGIONARY);
        sound_effect_play(SOUND_EFFECT_JAVELIN);
    } else if (should_die) {
        f->state = FigureState_Dead;
    }
    int dir = (16 + f->direction - 2 * Data_State.map.orientation) % 16;
    f->graphicId = image_group(GROUP_FIGURE_MISSILE) + dir;
}

void figure_bolt_action(figure *f)
{
    f->useCrossCountry = 1;
    f->progressOnTile++;
    if (f->progressOnTile > 120) {
        f->state = FigureState_Dead;
    }
    int should_die = FigureMovement_crossCountryWalkTicks(f, 4);
    int target_id = get_non_citizen_on_tile(f->gridOffset);
    if (target_id) {
        figure *target = figure_get(target_id);
        const figure_properties *target_props = figure_properties_for_type(target->type);
        int max_damage = target_props->max_damage;
        int damage_inflicted =
            figure_properties_for_type(f->type)->missile_attack_value -
            target_props->missile_defense_value;
        if (damage_inflicted < 0) {
            damage_inflicted = 0;
        }
        int target_damage = damage_inflicted + target->damage;
        if (target_damage <= max_damage) {
            target->damage = target_damage;
        } else { // kill target
            target->damage = max_damage + 1;
            target->actionState = FIGURE_ACTION_149_CORPSE;
            target->waitTicks = 0;
            figure_play_die_sound(target->type);
            Formation_updateAfterDeath(target->formationId);
        }
        sound_effect_play(SOUND_EFFECT_BALLISTA_HIT_PERSON);
        f->state = FigureState_Dead;
    } else if (should_die) {
        f->state = FigureState_Dead;
        sound_effect_play(SOUND_EFFECT_BALLISTA_HIT_GROUND);
    }
    int dir = (16 + f->direction - 2 * Data_State.map.orientation) % 16;
    f->graphicId = image_group(GROUP_FIGURE_MISSILE) + 32 + dir;
}
