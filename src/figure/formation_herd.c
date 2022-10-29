#include "formation_herd.h"

#include "city/figures.h"
#include "city/sound.h"
#include "core/config.h"
#include "core/random.h"
#include "figure/combat.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "figure/formation_enemy.h"
#include "figure/route.h"
#include "map/desirability.h"
#include "map/grid.h"
#include "map/soldier_strength.h"
#include "map/terrain.h"
#include "scenario/property.h"
#include "sound/effect.h"

#define FREE_TILE_SEARCH_RADIUS 4
#define ANIMAL_MOVE_WAIT_TICKS 401
#define SHEEP_ROAM_DISTANCE 8
#define SHEEP_ROAM_DELAY 20
#define ZEBRA_ROAM_DISTANCE 20
#define ZEBRA_ROAM_DELAY 4
#define WOLF_ROAM_DISTANCE 16
#define WOLF_ROAM_DELAY 6
#define WOLF_SPAWN_DELAY_TICKS 32

// Look for a free tile, in the neighborhood of (x,y)
static int get_free_tile(int x, int y, int allow_negative_desirability, int *x_tile, int *y_tile)
{
    unsigned short disallowed_terrain = ~(TERRAIN_ACCESS_RAMP | TERRAIN_MEADOW);
    int tile_found = 0;
    int x_found = 0, y_found = 0;

    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, 1, FREE_TILE_SEARCH_RADIUS, &x_min, &y_min, &x_max, &y_max);

    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            int grid_offset = map_grid_offset(xx, yy);
            if (!map_terrain_is(grid_offset, disallowed_terrain)) {
                if (map_soldier_strength_get(grid_offset)) {
                    return 0;
                }
                int desirability = map_desirability_get(grid_offset);
                if (allow_negative_desirability) {
                    if (desirability > 1) {
                        return 0;
                    }
                } else if (desirability) {
                    return 0;
                }
                tile_found = 1;
                x_found = xx;
                y_found = yy;
            }
        }
    }
    *x_tile = x_found;
    *y_tile = y_found;
    return tile_found;
}

// Try to find an open destination point (x_tile,y_tile) in the general direction, at the given distance
static int get_roaming_destination(formation *m, int distance, int *x_tile, int *y_tile)
{
    int formation_id = m->id;
    int x = m->x_home;
    int y = m->y_home;
    int direction = m->herd_direction;
    int allow_negative_desirability = m->figure_type == FIGURE_WOLF;
    int target_direction = (formation_id + random_byte()) & 7;
    if (direction >= DIR_0_TOP && direction < DIR_8_NONE) {
        target_direction = direction;
        allow_negative_desirability = 1;
    }
    // Do up to 4 checks to find a place to walk to
    for (int i = 0; i < 4; i++) {
        int x_target, y_target;
        switch (target_direction) {
            case DIR_0_TOP:
                x_target = x;
                y_target = y - distance;
                break;
            case DIR_1_TOP_RIGHT:
                x_target = x + distance;
                y_target = y - distance;
                break;
            case DIR_2_RIGHT:
                x_target = x + distance;
                y_target = y;
                break;
            case DIR_3_BOTTOM_RIGHT:
                x_target = x + distance;
                y_target = y + distance;
                break;
            case DIR_4_BOTTOM:
                x_target = x;
                y_target = y + distance;
                break;
            case DIR_5_BOTTOM_LEFT:
                x_target = x - distance;
                y_target = y + distance;
                break;
            case DIR_6_LEFT:
                x_target = x - distance;
                y_target = y;
                break;
            case DIR_7_TOP_LEFT:
                x_target = x - distance;
                y_target = y - distance;
                break;
            default:
                continue;
        }
        if (x_target <= 0) {
            x_target = 1;
        } else if (y_target <= 0) {
            y_target = 1;
        } else if (x_target >= map_grid_width() - 1) {
            x_target = map_grid_width() - 2;
        } else if (y_target >= map_grid_height() - 1) {
            y_target = map_grid_height() - 2;
        }
        // If we can find a free tile in this direction, return 1
        if (get_free_tile(x_target, y_target, allow_negative_desirability, x_tile, y_tile)) {
            return 1;
        }
        // ...otherwise turn right and try again
        target_direction = (target_direction + 2) % 8;
    }
    return 0;
}

static void move_animals(const formation *m, int attacking_animals)
{
    for (int i = 0; i < MAX_FORMATION_FIGURES; i++) {
        if (m->figures[i] <= 0) {
            continue;
        }
        figure *f = figure_get(m->figures[i]);
        if (f->action_state == FIGURE_ACTION_149_CORPSE ||
            f->action_state == FIGURE_ACTION_150_ATTACK) {
            continue;
        }
        f->wait_ticks = ANIMAL_MOVE_WAIT_TICKS;
        if (attacking_animals) {
            int target_id = figure_combat_get_target_for_wolf(f->x, f->y, 6);
            if (target_id) {
                figure *target = figure_get(target_id);
                f->action_state = FIGURE_ACTION_199_WOLF_ATTACKING;
                f->destination_x = target->x;
                f->destination_y = target->y;
                f->target_figure_id = target_id;
                target->targeted_by_figure_id = f->id;
                f->target_figure_created_sequence = target->created_sequence;
                figure_route_remove(f);
            } else {
                f->action_state = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
            }
        } else {
            f->action_state = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
        }
    }
}

static int can_spawn_wolf(formation *m)
{
    if (m->num_figures < m->max_figures && m->figure_type == FIGURE_WOLF) {
        m->herd_wolf_spawn_delay++;
        if (m->herd_wolf_spawn_delay > WOLF_SPAWN_DELAY_TICKS) {
            m->herd_wolf_spawn_delay = 0;
            return 1;
        }
    }
    return 0;
}

static void update_herd_formation(formation *m, int infinite_wolves_spawning)
{
    if (infinite_wolves_spawning && can_spawn_wolf(m)) {
        // spawn new wolf
        if (!map_terrain_is(map_grid_offset(m->x, m->y), TERRAIN_IMPASSABLE_WOLF)) {
            figure *wolf = figure_create(m->figure_type, m->x, m->y, DIR_0_TOP);
            wolf->action_state = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
            wolf->formation_id = m->id;
            wolf->wait_ticks = wolf->id & 0x1f;
        }
    }

    if (m->figures[0]) {
        figure *f = figure_get(m->figures[0]);
        if (f->state == FIGURE_STATE_ALIVE) {
            formation_set_home(m, f->x, f->y);
        }
    }

    int attacking_animals = 0;
    int roam_distance;
    int roam_delay;
    switch (m->figure_type) {
        case FIGURE_SHEEP:
            roam_distance = SHEEP_ROAM_DISTANCE;
            roam_delay = SHEEP_ROAM_DELAY;
            break;
        case FIGURE_ZEBRA:
            roam_distance = ZEBRA_ROAM_DISTANCE;
            roam_delay = ZEBRA_ROAM_DELAY;
            break;
        case FIGURE_WOLF:
            roam_distance = WOLF_ROAM_DISTANCE;
            roam_delay = WOLF_ROAM_DELAY;
            for (int fig = 0; fig < MAX_FORMATION_FIGURES; fig++) {
                int figure_id = m->figures[fig];
                if (figure_id > 0 && figure_get(figure_id)->action_state == FIGURE_ACTION_150_ATTACK) {
                    attacking_animals++;
                }
            }
            if (m->missile_attack_timeout) {
                attacking_animals = 1;
            }
            break;
        default:
            return;
    }
    m->wait_ticks++;
    if (m->wait_ticks > roam_delay || attacking_animals) {
        m->wait_ticks = 0;
        if (attacking_animals) {
            formation_set_destination(m, m->x_home, m->y_home);
            move_animals(m, attacking_animals);
        } else {
            int x_tile, y_tile;
            if (get_roaming_destination(m, roam_distance, &x_tile, &y_tile)) {
                m->herd_direction = DIR_8_NONE;
                if (formation_enemy_move_formation_to(m, x_tile, y_tile, &x_tile, &y_tile)) {
                    formation_set_destination(m, x_tile, y_tile);
                    if (m->figure_type == FIGURE_WOLF && city_sound_update_march_wolf()) {
                        sound_effect_play(SOUND_EFFECT_WOLF_HOWL);
                    }
                    move_animals(m, 0);
                }
            }
        }
    }
}

void formation_herd_update(void)
{
    if (city_figures_animals() <= 0 && scenario_property_climate() != CLIMATE_NORTHERN) {
        return;
    }
    for (int i = 1; i < formation_count(); i++) {
        formation *m = formation_get(i);
        int infinite_wolves_spawning = m->figure_type == FIGURE_WOLF && !config_get(CONFIG_GP_CH_DISABLE_INFINITE_WOLVES_SPAWNING);
        if (m->in_use && m->is_herd && !m->is_legion && (m->num_figures > 0 || infinite_wolves_spawning)) {
            update_herd_formation(m, infinite_wolves_spawning);
        }
    }
}
