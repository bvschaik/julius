#include "formation_legion.h"

#include "city/warning.h"
#include "figure/figure.h"
#include "figure/route.h"
#include "map/grid.h"
#include "map/routing.h"

#include "../Formation.h"

int formation_legion_create_for_fort(building *fort)
{
    Formation_calculateLegionTotals();

    formation *m = formation_create_legion(fort->id, fort->x, fort->y, fort->subtype.fortFigureType);
    if (!m->id) {
        return 0;
    }
    figure *standard = figure_create(FIGURE_FORT_STANDARD, 0, 0, DIR_0_TOP);
    standard->buildingId = fort->id;
    standard->formationId = m->id;
    m->standard_figure_id = standard->id;

    return m->id;
}

void formation_legion_delete_for_fort(building *fort)
{
    if (fort->formationId > 0) {
        formation *m = formation_get(fort->formationId);
        if (m->in_use) {
            if (m->standard_figure_id) {
                figure_delete(figure_get(m->standard_figure_id));
            }
            formation_clear(fort->formationId);
            Formation_calculateLegionTotals();
        }
    }
}

int formation_legion_recruits_needed()
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        formation *m = formation_get(i);
        if (m->in_use && m->is_legion && m->legion_recruit_type != LEGION_RECRUIT_NONE) {
            return 1;
        }
    }
    return 0;
}

void formation_legion_update_recruit_status(building *fort)
{
    formation *m = formation_get(fort->formationId);
    m->legion_recruit_type = LEGION_RECRUIT_NONE;
    if (!m->is_at_fort || m->cursed_by_mars || m->num_figures == m->max_figures) {
        return;
    }
    if (m->num_figures < m->max_figures) {
        int type = fort->subtype.fortFigureType;
        if (type == FIGURE_FORT_LEGIONARY) {
            m->legion_recruit_type = LEGION_RECRUIT_LEGIONARY;
        } else if (type == FIGURE_FORT_JAVELIN) {
            m->legion_recruit_type = LEGION_RECRUIT_JAVELIN;
        } else if (type == FIGURE_FORT_MOUNTED) {
            m->legion_recruit_type = LEGION_RECRUIT_MOUNTED;
        }
    } else { // too many figures
        int tooMany = m->num_figures - m->max_figures;
        for (int i = MAX_FORMATION_FIGURES - 1; i >= 0 && tooMany > 0; i--) {
            if (m->figures[i]) {
                figure_get(m->figures[i])->actionState = FIGURE_ACTION_82_SOLDIER_RETURNING_TO_BARRACKS;
                tooMany--;
            }
        }
        Formation_calculateFigures();
    }
}

static int prepare_to_move(formation *m)
{
    if (m->months_very_low_morale || m->months_low_morale > 1) {
        return 0;
    }
    if (m->months_low_morale == 1) {
        formation_change_morale(m->id, 10); // yay, we can move!
    }
    return 1;
}

void formation_legion_move_to(formation *m, int x, int y)
{
    map_routing_calculate_distances(m->x_home, m->y_home);
    if (map_routing_distance(map_grid_offset(x, y)) <= 0) {
        return; // unable to route there
    }
    if (x == m->x_home && y == m->y_home) {
        return; // use legionReturnHome
    }
    if (m->cursed_by_mars) {
        return;
    }
    m->standard_x = x;
    m->standard_y = y;
    m->is_at_fort = 0;

    if (m->morale <= 20) {
        city_warning_show(WARNING_LEGION_MORALE_TOO_LOW);
    }
    for (int i = 0; i < MAX_FORMATION_FIGURES && m->figures[i]; i++) {
        figure *f = figure_get(m->figures[i]);
        if (f->actionState == FIGURE_ACTION_149_CORPSE ||
            f->actionState == FIGURE_ACTION_150_ATTACK) {
            continue;
        }
        if (prepare_to_move(m)) {
            f->alternativeLocationIndex = 0;
            f->actionState = FIGURE_ACTION_83_SOLDIER_GOING_TO_STANDARD;
            figure_route_remove(f);
        }
    }
}

void formation_legion_return_home(formation *m)
{
    map_routing_calculate_distances(m->x_home, m->y_home);
    if (map_routing_distance(map_grid_offset(m->x, m->y)) <= 0) {
        return; // unable to route home
    }
    if (m->cursed_by_mars) {
        return;
    }
    m->is_at_fort = 1;
    formation_restore_layout(m);
    for (int i = 0; i < MAX_FORMATION_FIGURES && m->figures[i]; i++) {
        figure *f = figure_get(m->figures[i]);
        if (f->actionState == FIGURE_ACTION_149_CORPSE ||
            f->actionState == FIGURE_ACTION_150_ATTACK) {
            continue;
        }
        if (prepare_to_move(m)) {
            f->actionState = FIGURE_ACTION_81_SOLDIER_GOING_TO_FORT;
            figure_route_remove(f);
        }
    }
}
