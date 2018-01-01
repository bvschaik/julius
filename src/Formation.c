#include "Formation.h"

#include "Data/CityInfo.h"

#include "building/model.h"
#include "city/warning.h"
#include "core/calc.h"
#include "figure/enemy_army.h"
#include "figure/formation.h"
#include "figure/properties.h"
#include "figure/route.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/routing.h"
#include "scenario/distant_battle.h"
#include "sound/effect.h"

#include <string.h>

static int is_legion(figure *f)
{
    if (FigureIsLegion(f->type) || f->type == FIGURE_FORT_STANDARD) {
        return f->formationId;
    }
    return 0;
}

int Formation_getLegionFormationAtGridOffset(int gridOffset)
{
    return map_figure_foreach_until(gridOffset, is_legion);
}

int Formation_getFormationForBuilding(int gridOffset)
{
	int buildingId = map_building_at(gridOffset);
	if (buildingId > 0) {
		building *b = building_get(buildingId);
		if (BuildingIsInUse(b) && (b->type == BUILDING_FORT || b->type == BUILDING_FORT_GROUND)) {
			return b->formationId;
		}
	}
	return 0;
}

static void update_totals(const formation *m)
{
    if (m->is_legion) {
        formation_totals_add_legion(m->id);
        if (m->figure_type == FIGURE_FORT_LEGIONARY) {
            Data_CityInfo.militaryLegionaryLegions++;
        }
    }
    if (m->missile_attack_timeout <= 0 && m->figures[0]) {
        figure *f = figure_get(m->figures[0]);
        if (f->state == FigureState_Alive) {
            formation_set_home(m->id, f->x, f->y);
        }
    }
}
void Formation_calculateLegionTotals()
{
    formation_totals_clear_legions();
    Data_CityInfo.militaryLegionaryLegions = 0;
    formation_foreach(update_totals);
}

int Formation_getClosestMilitaryAcademy(int formationId)
{
	building *fort = building_get(formation_get(formationId)->building_id);
	int minBuildingId = 0;
	int minDistance = 10000;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
		if (BuildingIsInUse(b) && b->type == BUILDING_MILITARY_ACADEMY &&
			b->numWorkers >= model_get_building(BUILDING_MILITARY_ACADEMY)->laborers) {
			int dist = calc_maximum_distance(fort->x, fort->y, b->x, b->y);
			if (dist < minDistance) {
				minDistance = dist;
				minBuildingId = i;
			}
		}
	}
	return minBuildingId;
}

static void update_legion_enemy_totals(const formation *m, void *data)
{
    if (m->is_legion) {
        if (m->num_figures > 0) {
            int wasHalted = m->is_halted;
            formation_set_halted(m->id, 1);
            for (int fig = 0; fig < m->num_figures; fig++) {
                int figureId = m->figures[fig];
                if (figureId && figure_get(figureId)->direction != DIR_8_NONE) {
                    formation_set_halted(m->id, 0);
                }
            }
            int total_strength = m->num_figures;
            if (m->figure_type == FIGURE_FORT_LEGIONARY) {
                total_strength += m->num_figures / 2;
            }
            enemy_army_totals_add_legion_formation(total_strength);
            if (m->figure_type == FIGURE_FORT_LEGIONARY) {
                if (!wasHalted && m->is_halted) {
                    sound_effect_play(SOUND_EFFECT_FORMATION_SHIELD);
                }
            }
        }
    } else {
        // enemy
        if (m->num_figures <= 0) {
            formation_clear(m->id);
        } else {
            enemy_army_totals_add_enemy_formation(m->num_figures);
        }
    }
}

static void update_soldier_totals(const formation *m, void *data)
{
    Data_CityInfo.militaryTotalLegions++;
    Data_CityInfo.militaryTotalSoldiers += m->num_figures;
    if (m->empire_service && m->num_figures > 0) {
        Data_CityInfo.militaryTotalLegionsEmpireService++;
    }
}

void Formation_calculateFigures()
{
    formation_clear_figures();
	for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
		if (f->state != FigureState_Alive) {
			continue;
		}
		if (!FigureIsLegion(f->type) && !FigureIsEnemy(f->type) && !FigureIsHerd(f->type)) {
			continue;
		}
		if (f->type == FIGURE_ENEMY54_GLADIATOR) {
			continue;
		}
        int index = formation_add_figure(f->formationId, i,
            f->formationAtRest != 1, f->damage,
            figure_properties_for_type(f->type)->max_damage
        );
        f->indexInFormation = index;
	}

    enemy_army_totals_clear();
    formation_foreach_non_herd(update_legion_enemy_totals, 0);

    Data_CityInfo.militaryTotalLegionsEmpireService = 0;
    Data_CityInfo.militaryTotalSoldiers = 0;
    Data_CityInfo.militaryTotalLegions = 0;
    formation_foreach_legion(update_soldier_totals, 0);
}

void Formation_updateAfterDeath(int formationId)
{
	Formation_calculateFigures();
	int pctDead = calc_percentage(1, formation_get(formationId)->num_figures);
	int morale;
	if (pctDead < 8) {
		morale = -5;
	} else if (pctDead < 10) {
		morale = -7;
	} else if (pctDead < 14) {
		morale = -10;
	} else if (pctDead < 20) {
		morale = -12;
	} else if (pctDead < 30) {
		morale = -15;
	} else {
		morale = -20;
	}
	formation_change_morale(formationId, morale);
}

int Formation_getInvasionGridOffset(int invasionSeq)
{
    int formationId = formation_for_invasion(invasionSeq);
    if (formationId) {
        const formation *m = formation_get(formationId);
        if (m->x_home > 0 || m->y_home > 0) {
            return map_grid_offset(m->x_home, m->y_home);
        }
    }
    return 0;
}


int Formation_marsCurseFort()
{
	int bestLegionId = 0;
	int bestLegionWeight = 0;
	for (int i = 1; i <= 6; i++) { // BUG assumes no legions beyond index 6
		const formation *m = formation_get(i);
		if (m->in_use == 1 && m->is_legion) {
			int weight = m->num_figures;
			if (m->figure_type == FIGURE_FORT_LEGIONARY) {
				weight *= 2;
			}
			if (weight > bestLegionWeight) {
				bestLegionWeight = weight;
				bestLegionId = i;
			}
		}
	}
	if (bestLegionId <= 0) {
		return 0;
	}
	const formation *m = formation_get(bestLegionId);
	for (int i = 0; i < MAX_FORMATION_FIGURES - 1; i++) { // BUG: last figure not cursed
		if (m->figures[i] > 0) {
			figure_get(m->figures[i])->actionState = FIGURE_ACTION_82_SOLDIER_RETURNING_TO_BARRACKS;
		}
	}
	formation_set_cursed(bestLegionId);
	Formation_calculateFigures();
	return 1;
}
