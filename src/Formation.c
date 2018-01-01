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

static void update_legion_enemy_totals(formation *m)
{
    if (m->is_legion) {
        if (m->num_figures > 0) {
            int wasHalted = m->is_halted;
            m->is_halted = 1;
            for (int fig = 0; fig < m->num_figures; fig++) {
                int figureId = m->figures[fig];
                if (figureId && figure_get(figureId)->direction != DIR_8_NONE) {
                    m->is_halted = 0;
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
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        formation *m = formation_get(i);
        if (m->in_use && !m->is_herd) {
            update_legion_enemy_totals(m);
        }
    }

    Data_CityInfo.militaryTotalLegionsEmpireService = 0;
    Data_CityInfo.militaryTotalSoldiers = 0;
    Data_CityInfo.militaryTotalLegions = 0;
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        const formation *m = formation_get(i);
        if (m->in_use && m->is_legion) {
            Data_CityInfo.militaryTotalLegions++;
            Data_CityInfo.militaryTotalSoldiers += m->num_figures;
            if (m->empire_service && m->num_figures > 0) {
                Data_CityInfo.militaryTotalLegionsEmpireService++;
            }
        }
    }
}

void Formation_updateAfterDeath(int formationId)
{
	Formation_calculateFigures();
    formation *m = formation_get(formationId);
	int pctDead = calc_percentage(1, m->num_figures);
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
	formation_change_morale(m, morale);
}
