#include "Formation.h"

#include "TerrainGraphics.h"

#include "Data/CityInfo.h"
#include "Data/State.h"

#include "city/message.h"
#include "core/calc.h"
#include "core/random.h"
#include "figure/combat.h"
#include "figure/enemy_army.h"
#include "figure/formation.h"
#include "figure/formation_enemy.h"
#include "figure/formation_layout.h"
#include "figure/formation_legion.h"
#include "figure/route.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/routing.h"
#include "map/routing_path.h"
#include "map/soldier_strength.h"
#include "map/terrain.h"
#include "sound/effect.h"

static void tickDecreaseLegionDamage()
{
	for (int i = 1; i < MAX_FIGURES; i++) {
		figure *f = figure_get(i);
		if (f->state == FigureState_Alive && FigureIsLegion(f->type)) {
			if (f->actionState == FIGURE_ACTION_80_SOLDIER_AT_REST) {
				if (f->damage) {
					f->damage--;
				}
			}
		}
	}
}

static void tickUpdateDirection()
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        formation *m = formation_get(i);
        if (m->in_use && !m->is_herd) {
            formation_update_direction(m->id, figure_get(m->figures[0])->direction);
        }
    }
}

static void tickUpdateLegions()
{
	for (int i = 1; i <= MAX_LEGIONS; i++) {
		formation *m = formation_get(i);
		if (m->in_use != 1 || !m->is_legion) {
			continue;
		}
		formation_decrease_monthly_counters(m);
		if (Data_CityInfo.numEnemiesInCity <= 0) {
			formation_clear_monthly_counters(m);
		}
		for (int n = 0; n < MAX_FORMATION_FIGURES; n++) {
			if (figure_get(m->figures[n])->actionState == FIGURE_ACTION_150_ATTACK) {
                formation_record_fight(m);
			}
		}
		if (formation_has_low_morale(m->id)) {
			// flee back to fort
			for (int n = 0; n < MAX_FORMATION_FIGURES; n++) {
				figure *f = figure_get(m->figures[n]);
				if (f->actionState != FIGURE_ACTION_150_ATTACK &&
					f->actionState != FIGURE_ACTION_149_CORPSE &&
					f->actionState != FIGURE_ACTION_148_FLEEING) {
					f->actionState = FIGURE_ACTION_148_FLEEING;
					figure_route_remove(f);
				}
			}
		} else if (m->layout == FORMATION_MOP_UP) {
			if (enemy_army_total_enemy_formations() +
				Data_CityInfo.numRiotersInCity +
				Data_CityInfo.numAttackingNativesInCity > 0) {
				for (int n = 0; n < MAX_FORMATION_FIGURES; n++) {
                    if (m->figures[n] != 0) {
                        figure *f = figure_get(m->figures[n]);
                        if (f->actionState != FIGURE_ACTION_150_ATTACK &&
                            f->actionState != FIGURE_ACTION_149_CORPSE) {
                            f->actionState = FIGURE_ACTION_86_SOLDIER_MOPPING_UP;
                        }
                    }
				}
			} else {
				formation_legion_restore_layout(m);
			}
		}
	}
}

static int getHerdRoamingDestination(int formationId, int allowNegativeDesirability,
	int x, int y, int distance, int direction, int *xTile, int *yTile)
{
	int targetDirection = (formationId + random_byte()) & 6;
	if (direction) {
		targetDirection = direction;
		allowNegativeDesirability = 1;
	}
	for (int i = 0; i < 4; i++) {
		int xTarget, yTarget;
		switch (targetDirection) {
			case DIR_0_TOP:         xTarget = x; yTarget = y - distance; break;
			case DIR_1_TOP_RIGHT:   xTarget = x + distance; yTarget = y - distance; break;
			case DIR_2_RIGHT:       xTarget = x + distance; yTarget = y; break;
			case DIR_3_BOTTOM_RIGHT:xTarget = x + distance; yTarget = y + distance; break;
			case DIR_4_BOTTOM:      xTarget = x; yTarget = y + distance; break;
			case DIR_5_BOTTOM_LEFT: xTarget = x - distance; yTarget = y + distance; break;
			case DIR_6_LEFT:        xTarget = x - distance; yTarget = y; break;
			case DIR_7_TOP_LEFT:    xTarget = x - distance; yTarget = y - distance; break;
			default: continue;
		}
		if (xTarget <= 0) {
			xTarget = 1;
		} else if (yTarget <= 0) {
			yTarget = 1;
		} else if (xTarget >= Data_State.map.width - 1) {
			xTarget = Data_State.map.width - 2;
		} else if (yTarget >= Data_State.map.height - 1) {
			yTarget = Data_State.map.height - 2;
		}
		if (TerrainGraphics_getFreeTileForHerd(xTarget, yTarget, allowNegativeDesirability, xTile, yTile)) {
			return 1;
		}
		targetDirection += 2;
		if (targetDirection > 6) {
			targetDirection = 0;
		}
	}
	return 0;
}

static void moveAnimals(const formation *m, int attackingAnimals)
{
	for (int i = 0; i < MAX_FORMATION_FIGURES; i++) {
		if (m->figures[i] <= 0) continue;
		figure *f = figure_get(m->figures[i]);
		if (f->actionState == FIGURE_ACTION_149_CORPSE ||
			f->actionState == FIGURE_ACTION_150_ATTACK) {
			continue;
		}
		f->waitTicks = 401;
		if (attackingAnimals) {
			int targetId = figure_combat_get_target_for_wolf(f->x, f->y, 6);
			if (targetId) {
                figure *target = figure_get(targetId);
				f->actionState = FIGURE_ACTION_199_WOLF_ATTACKING;
				f->destinationX = target->x;
				f->destinationY = target->y;
				f->targetFigureId = targetId;
				target->targetedByFigureId = f->id;
				f->targetFigureCreatedSequence = target->createdSequence;
				figure_route_remove(f);
			} else {
				f->actionState = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
			}
		} else {
			f->actionState = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
		}
	}
}

static void update_herd_formation(formation *m)
{
    if (formation_can_spawn_wolf(m->id)) {
        // spawn new wolf
        if (!map_terrain_is(map_grid_offset(m->x, m->y), TERRAIN_IMPASSABLE_WOLF)) {
            figure *wolf = figure_create(m->figure_type, m->x, m->y, DIR_0_TOP);
            wolf->actionState = FIGURE_ACTION_196_HERD_ANIMAL_AT_REST;
            wolf->formationId = m->id;
            wolf->waitTicks = wolf->id & 0x1f;
        }
    }
    int attackingAnimals = 0;
    for (int fig = 0; fig < MAX_FORMATION_FIGURES; fig++) {
        int figureId = m->figures[fig];
        if (figureId > 0 && figure_get(figureId)->actionState == FIGURE_ACTION_150_ATTACK) {
            attackingAnimals++;
        }
    }
    if (m->missile_attack_timeout) {
        attackingAnimals = 1;
    }
    if (m->figures[0]) {
        figure *f = figure_get(m->figures[0]);
        if (f->state == FigureState_Alive) {
            formation_set_home(m, f->x, f->y);
        }
    }
    int roamDistance;
    int roamDelay;
    int allowNegativeDesirability;
    switch (m->figure_type) {
        case FIGURE_SHEEP:
            roamDistance = 8;
            roamDelay = 20;
            allowNegativeDesirability = 0;
            attackingAnimals = 0;
            break;
        case FIGURE_ZEBRA:
            roamDistance = 20;
            roamDelay = 4;
            allowNegativeDesirability = 0;
            attackingAnimals = 0;
            break;
        case FIGURE_WOLF:
            roamDistance = 16;
            roamDelay = 6;
            allowNegativeDesirability = 1;
            break;
        default:
            return;
    }
    formation_increase_wait_ticks(m->id);
    if (m->wait_ticks > roamDelay || attackingAnimals) {
        formation_reset_wait_ticks(m->id);
        if (attackingAnimals) {
            formation_set_destination(m, m->x_home, m->y_home);
            moveAnimals(m, attackingAnimals);
        } else {
            int xTile, yTile;
            if (getHerdRoamingDestination(m->id, allowNegativeDesirability, m->x_home, m->y_home, roamDistance, m->herd_direction, &xTile, &yTile)) {
                formation_herd_clear_direction(m->id);
                if (formation_enemy_move_formation_to(m, xTile, yTile, &xTile, &yTile)) {
                    formation_set_destination(m, xTile, yTile);
                    if (m->figure_type == FIGURE_WOLF) {
                        Data_CityInfo.soundMarchWolf--;
                        if (Data_CityInfo.soundMarchWolf <= 0) {
                            Data_CityInfo.soundMarchWolf = 12;
                            sound_effect_play(SOUND_EFFECT_WOLF_HOWL);
                        }
                    }
                    moveAnimals(m, attackingAnimals);
                }
            }
        }
    }
}
static void tickUpdateHerds()
{
	if (Data_CityInfo.numAnimalsInCity <= 0) {
		return;
	}
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        formation *m = formation_get(i);
        if (m->in_use && m->is_herd && !m->is_legion && m->num_figures > 0) {
            update_herd_formation(m);
        }
    }
}

void Formation_Tick_updateAll(int secondTime)
{
	Formation_calculateLegionTotals();
	Formation_calculateFigures();
	tickUpdateDirection();
	tickDecreaseLegionDamage();
	if (!secondTime) {
		formation_update_monthly_morale_deployed();
	}
	formation_legion_set_max_figures();
	tickUpdateLegions();
	formation_enemy_update();
	tickUpdateHerds();
}
