#include "missile.h"

#include "city/view.h"
#include "core/image.h"
#include "figure/formation.h"
#include "figure/movement.h"
#include "figure/properties.h"
#include "figure/sound.h"
#include "map/figure.h"
#include "map/point.h"
#include "sound/effect.h"

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
    int tile_offset = CLOUD_TILE_OFFSETS[size];
    int cc_offset = CLOUD_CC_OFFSETS[size];
    for (int i = 0; i < 16; i++) {
        figure *f = figure_create(FIGURE_EXPLOSION,
            x + tile_offset, y + tile_offset, DIR_0_TOP);
        if (f->id) {
            f->cross_country_x += cc_offset;
            f->cross_country_y += cc_offset;
            f->destination_x += CLOUD_DIRECTION[i].x;
            f->destination_y += CLOUD_DIRECTION[i].y;
            figure_movement_set_cross_country_direction(f,
                f->cross_country_x, f->cross_country_y,
                15 * f->destination_x + cc_offset,
                15 * f->destination_y + cc_offset, 0);
            f->speed_multiplier = CLOUD_SPEED[i];
        }
    }
    sound_effect_play(SOUND_EFFECT_EXPLOSION);
}

void figure_create_missile(int building_id, int x, int y, int x_dst, int y_dst, figure_type type)
{
    figure *f = figure_create(type, x, y, DIR_0_TOP);
    if (f->id) {
        if (type == FIGURE_BOLT || type == FIGURE_FRIENDLY_ARROW) {
            f->missile_damage = 60;
        } else {
            f->missile_damage = 10;
        }
        f->building_id = building_id;
        f->destination_x = x_dst;
        f->destination_y = y_dst;
        figure_movement_set_cross_country_direction(
            f, f->cross_country_x, f->cross_country_y,
            15 * x_dst, 15 * y_dst, 1);
    }
}

static int is_citizen(figure *f)
{
    if (f->action_state != FIGURE_ACTION_149_CORPSE) {
        if (f->type && f->type != FIGURE_EXPLOSION && f->type != FIGURE_FORT_STANDARD &&
            f->type != FIGURE_MAP_FLAG && f->type != FIGURE_FLOTSAM && (f->type < FIGURE_INDIGENOUS_NATIVE || f->type == FIGURE_TOWER_SENTRY)) {
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
    if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        return 0;
    }
    if (figure_is_enemy(f)) {
        return f->id;
    }
    if (f->type == FIGURE_INDIGENOUS_NATIVE && f->action_state == FIGURE_ACTION_159_NATIVE_ATTACKING) {
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
    f->use_cross_country = 1;
    f->progress_on_tile++;
    if (f->progress_on_tile > 44) {
        f->state = FIGURE_STATE_DEAD;
    }
    figure_movement_move_ticks_cross_country(f, f->speed_multiplier);
    if (f->progress_on_tile < 48) {
        f->image_id = image_group(GROUP_FIGURE_EXPLOSION) +
                       CLOUD_IMAGE_OFFSETS[f->progress_on_tile / 2];
    } else {
        f->image_id = image_group(GROUP_FIGURE_EXPLOSION) + 7;
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
    formation *m = formation_get(target->formation_id);
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
        target->action_state = FIGURE_ACTION_149_CORPSE;
        target->wait_ticks = 0;
        figure_play_die_sound(target);
        formation_update_morale_after_death(m);
    }
    f->state = FIGURE_STATE_DEAD;
    // for missiles: building_id contains the figure who shot it
    int missile_formation = figure_get(f->building_id)->formation_id;
    formation_record_missile_attack(m, missile_formation);
}

void figure_arrow_action(figure *f)
{
    f->use_cross_country = 1;
    f->progress_on_tile++;
    if (f->progress_on_tile > 120) {
        f->state = FIGURE_STATE_DEAD;
    }
    int should_die = figure_movement_move_ticks_cross_country(f, 4);
    int target_id = get_citizen_on_tile(f->grid_offset);
    if (target_id) {
        missile_hit_target(f, target_id, FIGURE_FORT_LEGIONARY);
        sound_effect_play(SOUND_EFFECT_ARROW_HIT);
    } else if (should_die) {
        f->state = FIGURE_STATE_DEAD;
    }
    int dir = (16 + f->direction - 2 * city_view_orientation()) % 16;
    f->image_id = image_group(GROUP_FIGURE_MISSILE) + 16 + dir;
}

void figure_spear_action(figure *f)
{
    f->use_cross_country = 1;
    f->progress_on_tile++;
    if (f->progress_on_tile > 120) {
        f->state = FIGURE_STATE_DEAD;
    }
    int should_die = figure_movement_move_ticks_cross_country(f, 4);
    int target_id = get_citizen_on_tile(f->grid_offset);
    if (target_id) {
        missile_hit_target(f, target_id, FIGURE_FORT_LEGIONARY);
        sound_effect_play(SOUND_EFFECT_JAVELIN);
    } else if (should_die) {
        f->state = FIGURE_STATE_DEAD;
    }
    int dir = (16 + f->direction - 2 * city_view_orientation()) % 16;
    f->image_id = image_group(GROUP_FIGURE_MISSILE) + dir;
}

void figure_friendly_arrow_action(figure* f)
{
	f->use_cross_country = 1;
	f->progress_on_tile++;
	if (f->progress_on_tile > 120) {
		f->state = FIGURE_STATE_DEAD;
	}
	int should_die = figure_movement_move_ticks_cross_country(f, 4);
	int target_id = get_non_citizen_on_tile(f->grid_offset);
	if (target_id) {
		missile_hit_target(f, target_id, FIGURE_ENEMY_CAESAR_LEGIONARY);
		sound_effect_play(SOUND_EFFECT_ARROW_HIT);
	}
	else if (should_die) {
		f->state = FIGURE_STATE_DEAD;
	}
	int dir = (16 + f->direction - 2 * city_view_orientation()) % 16;
	f->image_id = image_group(GROUP_FIGURE_MISSILE) + 16 + dir;
}


void figure_javelin_action(figure *f)
{
    f->use_cross_country = 1;
    f->progress_on_tile++;
    if (f->progress_on_tile > 120) {
        f->state = FIGURE_STATE_DEAD;
    }
    int should_die = figure_movement_move_ticks_cross_country(f, 4);
    int target_id = get_non_citizen_on_tile(f->grid_offset);
    if (target_id) {
        missile_hit_target(f, target_id, FIGURE_ENEMY_CAESAR_LEGIONARY);
        sound_effect_play(SOUND_EFFECT_JAVELIN);
    } else if (should_die) {
        f->state = FIGURE_STATE_DEAD;
    }
    int dir = (16 + f->direction - 2 * city_view_orientation()) % 16;
    f->image_id = image_group(GROUP_FIGURE_MISSILE) + dir;
}

void figure_bolt_action(figure *f)
{
    f->use_cross_country = 1;
    f->progress_on_tile++;
    if (f->progress_on_tile > 120) {
        f->state = FIGURE_STATE_DEAD;
    }
    int should_die = figure_movement_move_ticks_cross_country(f, 4);
    int target_id = get_non_citizen_on_tile(f->grid_offset);
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
            target->action_state = FIGURE_ACTION_149_CORPSE;
            target->wait_ticks = 0;
            figure_play_die_sound(target);
            formation_update_morale_after_death(formation_get(target->formation_id));
        }
        sound_effect_play(SOUND_EFFECT_BALLISTA_HIT_PERSON);
        f->state = FIGURE_STATE_DEAD;
    } else if (should_die) {
        f->state = FIGURE_STATE_DEAD;
        sound_effect_play(SOUND_EFFECT_BALLISTA_HIT_GROUND);
    }
    int dir = (16 + f->direction - 2 * city_view_orientation()) % 16;
    f->image_id = image_group(GROUP_FIGURE_MISSILE) + 32 + dir;
}
