#include "formation_legion.h"

#include "city/warning.h"
#include "figure/figure.h"
#include "figure/route.h"
#include "map/grid.h"
#include "map/routing.h"

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
