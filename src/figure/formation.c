#include "formation.h"

#include "city/military.h"
#include "core/calc.h"
#include "core/config.h"
#include "figure/enemy_army.h"
#include "figure/figure.h"
#include "figure/formation_enemy.h"
#include "figure/formation_herd.h"
#include "figure/formation_legion.h"
#include "figure/properties.h"
#include "map/grid.h"
#include "sound/effect.h"

#include <string.h>

static formation formations[MAX_FORMATIONS];

static struct {
    int id_last_in_use;
    int id_last_legion;
    int num_legions;
} data;

void formations_clear(void)
{
    for (int i = 0; i < MAX_FORMATIONS; i++) {
        memset(&formations[i], 0, sizeof(formation));
        formations[i].id = i;
    }
    data.id_last_in_use = 0;
    data.id_last_legion = 0;
    data.num_legions = 0;
}

void formation_clear(int formation_id)
{
    memset(&formations[formation_id], 0, sizeof(formation));
    formations[formation_id].id = formation_id;
}

static int get_free_formation(int start_index)
{
    for (int i = start_index; i < MAX_FORMATIONS; i++) {
        if (!formations[i].in_use) {
            return i;
        }
    }
    return 0;
}

formation *formation_create_legion(int building_id, int x, int y, figure_type type)
{
    int formation_id = get_free_formation(1);
    if (!formation_id) {
        return &formations[0];
    }
    formation *m = &formations[formation_id];
    m->faction_id = 1;
    m->in_use = 1;
    m->is_legion = 1;
    m->figure_type = type;
    m->building_id = building_id;
    m->layout = FORMATION_DOUBLE_LINE_1;
    m->morale = 50;
    m->is_at_fort = 1;
    m->legion_id = formation_id - 1;
    if (m->legion_id >= 9) {
        m->legion_id = 9;
    }
    m->x = m->standard_x = m->x_home = x + 3;
    m->y = m->standard_y = m->y_home = y - 1;

    data.num_legions++;
    if (formation_id > data.id_last_in_use) {
        data.id_last_in_use = formation_id;
    }
    return m;
}

static int formation_create(int figure_type, int layout, int orientation, int x, int y)
{
    int formation_id = get_free_formation(10);
    if (!formation_id) {
        return 0;
    }
    formation *f = &formations[formation_id];
    f->faction_id = 0;
    f->x = x;
    f->y = y;
    f->in_use = 1;
    f->is_legion = 0;
    f->figure_type = figure_type;
    f->legion_id = formation_id - 10;
    f->morale = 100;
    if (layout == FORMATION_ENEMY_DOUBLE_LINE) {
        if (orientation == DIR_0_TOP || orientation == DIR_4_BOTTOM) {
            f->layout = FORMATION_DOUBLE_LINE_1;
        } else {
            f->layout = FORMATION_DOUBLE_LINE_2;
        }
    } else {
        f->layout = layout;
    }
    return formation_id;
}

int formation_create_herd(int figure_type, int x, int y, int num_animals)
{
    int formation_id = formation_create(figure_type, FORMATION_HERD, 0, x, y);
    if (!formation_id) {
        return 0;
    }
    formation *f = &formations[formation_id];
    f->is_herd = 1;
    f->wait_ticks = 24;
    f->max_figures = num_animals;
    return formation_id;
}

int formation_create_enemy(int figure_type, int x, int y, int layout, int orientation,
                           int enemy_type, int attack_type, int invasion_id, int invasion_sequence)
{
    int formation_id = formation_create(figure_type, layout, orientation, x, y);
    if (!formation_id) {
        return 0;
    }
    formation *f = &formations[formation_id];
    f->attack_type = attack_type;
    f->orientation = orientation;
    f->enemy_type = enemy_type;
    f->invasion_id = invasion_id;
    f->invasion_sequence = invasion_sequence;
    return formation_id;
}

formation *formation_get(int formation_id)
{
    return &formations[formation_id];
}

void formation_toggle_empire_service(int formation_id)
{
    formations[formation_id].empire_service = formations[formation_id].empire_service ? 0 : 1;
}

void formation_record_missile_fired(formation *m)
{
    m->missile_fired = 6;
}

void formation_record_missile_attack(formation *m, int from_formation_id)
{
    m->missile_attack_timeout = 6;
    m->missile_attack_formation_id = from_formation_id;
}

void formation_record_fight(formation *m)
{
    m->recent_fight = 6;
}

int formation_grid_offset_for_invasion(int invasion_sequence)
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        formation *m = &formations[i];
        if (m->in_use == 1 && !m->is_legion && !m->is_herd && m->invasion_sequence == invasion_sequence) {
            if (m->x_home > 0 || m->y_home > 0) {
                return map_grid_offset(m->x_home, m->y_home);
            } else {
                return 0;
            }
        }
    }
    return 0;
}

void formation_caesar_pause(void)
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        if (formations[i].in_use == 1 && formations[i].figure_type == FIGURE_ENEMY_CAESAR_LEGIONARY) {
            formations[i].wait_ticks = 20;
        }
    }
}

void formation_caesar_retreat(void)
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        if (formations[i].in_use == 1 && formations[i].figure_type == FIGURE_ENEMY_CAESAR_LEGIONARY) {
            formations[i].months_low_morale = 1;
        }
    }
}

int formation_has_low_morale(formation *m)
{
    return m->months_low_morale || m->months_very_low_morale;
}

int formation_get_num_legions_cached(void)
{
    return data.num_legions;
}

void formation_calculate_legion_totals(void)
{
    data.id_last_legion = 0;
    data.num_legions = 0;
    city_military_clear_legionary_legions();
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        formation *m = formation_get(i);
        if (m->in_use) {
            if (m->is_legion) {
                data.id_last_legion = i;
                data.num_legions++;
                if (m->figure_type == FIGURE_FORT_LEGIONARY) {
                    city_military_add_legionary_legion();
                }
            }
            if (m->missile_attack_timeout <= 0 && m->figures[0]) {
                figure *f = figure_get(m->figures[0]);
                if (f->state == FIGURE_STATE_ALIVE) {
                    formation_set_home(m, f->x, f->y);
                }
            }
        }
    }
}

int formation_get_num_legions(void)
{
    int total = 0;
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        if (formations[i].in_use && formations[i].is_legion) {
            total++;
        }
    }
    return total;
}

int formation_get_max_legions(void)
{
   if (config_get(CONFIG_GP_CH_EXTRA_FORTS)) {
	  return MAX_LEGIONS+4;
   } else {
	  return MAX_LEGIONS; 
   }
}

int formation_for_legion(int legion_index)
{
    int index = 1;
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        if (formations[i].in_use && formations[i].is_legion) {
            if (index++ == legion_index) {
                return i;
            }
        }
    }
    return 0;
}

void formation_change_morale(formation *m, int amount)
{
    int max_morale;
    if (m->figure_type == FIGURE_FORT_LEGIONARY) {
        max_morale = m->has_military_training ? 100 : 80;
    } else if (m->figure_type == FIGURE_ENEMY_CAESAR_LEGIONARY) {
        max_morale = 100;
    } else if (m->figure_type == FIGURE_FORT_JAVELIN || m->figure_type == FIGURE_FORT_MOUNTED) {
        max_morale = m->has_military_training ? 80 : 60;
    } else {
        switch (m->enemy_type) {
            case ENEMY_0_BARBARIAN:
            case ENEMY_1_NUMIDIAN:
            case ENEMY_2_GAUL:
            case ENEMY_3_CELT:
            case ENEMY_4_GOTH:
                max_morale = 80;
                break;
            case ENEMY_8_GREEK:
            case ENEMY_10_CARTHAGINIAN:
                max_morale = 90;
                break;
            default:
                max_morale = 70;
                break;
        }
    }
    m->morale = calc_bound(m->morale + amount, 0, max_morale);
}

void formation_update_morale_after_death(formation *m)
{
    formation_calculate_figures();
    int pct_dead = calc_percentage(1, m->num_figures);
    int morale;
    if (pct_dead < 8) {
        morale = -5;
    } else if (pct_dead < 10) {
        morale = -7;
    } else if (pct_dead < 14) {
        morale = -10;
    } else if (pct_dead < 20) {
        morale = -12;
    } else if (pct_dead < 30) {
        morale = -15;
    } else {
        morale = -20;
    }
    formation_change_morale(m, morale);
}

static void change_all_morale(int legion, int enemy)
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        formation *m = &formations[i];
        if (m->in_use && !m->is_herd) {
            if (m->is_legion) {
                formation_change_morale(m, legion);
            } else {
                formation_change_morale(m, enemy);
            }
        }
    }
}

void formation_update_monthly_morale_deployed(void)
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        formation *f = &formations[i];
        if (f->in_use != 1 || f->is_herd) {
            continue;
        }
        if (f->is_legion) {
            if (!f->is_at_fort && !f->in_distant_battle) {
                if (f->morale <= 20 && !f->months_low_morale && !f->months_very_low_morale) {
                    change_all_morale(-10, 10);
                }
                if (f->morale <= 10) {
                    f->months_very_low_morale++;
                } else if (f->morale <= 20) {
                    f->months_low_morale++;
                }
            }
        } else { // enemy
            if (f->morale <= 20 && !f->months_low_morale && !f->months_very_low_morale) {
                change_all_morale(10, -10);
            }
            if (f->morale <= 10) {
                f->months_very_low_morale++;
            } else if (f->morale <= 20) {
                f->months_low_morale++;
            }
        }
    }
}

void formation_update_monthly_morale_at_rest(void)
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        formation *m = &formations[i];
        if (m->in_use != 1 || m->is_herd) {
            continue;
        }
        if (m->is_legion) {
            if (m->is_at_fort) {
                m->months_from_home = 0;
                m->months_very_low_morale = 0;
                m->months_low_morale = 0;
                formation_change_morale(m, 5);
                formation_legion_restore_layout(m);
            } else if (!m->recent_fight) {
                m->months_from_home++;
                if (m->months_from_home > 3) {
                    if (m->months_from_home > 100) {
                        m->months_from_home = 100;
                    }
                    formation_change_morale(m, -5);
                }
            }
        } else {
            formation_change_morale(m, 0);
        }
    }
}

void formation_decrease_monthly_counters(formation *m)
{
    if (m->is_legion) {
        if (m->cursed_by_mars) {
            m->cursed_by_mars--;
        }
    }
    if (m->missile_fired) {
        m->missile_fired--;
    }
    if (m->missile_attack_timeout) {
        m->missile_attack_timeout--;
    }
    if (m->recent_fight) {
        m->recent_fight--;
    }
}

void formation_clear_monthly_counters(formation *m)
{
    m->missile_fired = 0;
    m->missile_attack_timeout = 0;
    m->recent_fight = 0;
}

void formation_set_destination(formation *m, int x, int y)
{
    m->destination_x = x;
    m->destination_y = y;
}

void formation_set_destination_building(formation *m, int x, int y, int building_id)
{
    m->destination_x = x;
    m->destination_y = y;
    m->destination_building_id = building_id;
}

void formation_set_home(formation *m, int x, int y)
{
    m->x_home = x;
    m->y_home = y;
}

void formation_clear_figures(void)
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        formation *f = &formations[i];
        for (int fig = 0; fig < MAX_FORMATION_FIGURES; fig++) {
            f->figures[fig] = 0;
        }
        f->num_figures = 0;
        f->is_at_fort = 1;
        f->total_damage = 0;
        f->max_total_damage = 0;
    }
}

int formation_add_figure(int formation_id, int figure_id, int deployed, int damage, int max_damage)
{
    formation *f = &formations[formation_id];
    f->num_figures++;
    f->total_damage += damage;
    f->max_total_damage += max_damage;
    if (deployed) {
        f->is_at_fort = 0;
    }
    for (int fig = 0; fig < MAX_FORMATION_FIGURES; fig++) {
        if (!f->figures[fig]) {
            f->figures[fig] = figure_id;
            return fig;
        }
    }
    return 0; // shouldn't happen
}

void formation_move_herds_away(int x, int y)
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        formation *f = &formations[i];
        if (f->in_use != 1 || f->is_legion || !f->is_herd || f->num_figures <= 0) {
            continue;
        }
        if (calc_maximum_distance(x, y, f->x_home, f->y_home) <= 6) {
            formations[i].wait_ticks = 50;
            formations[i].herd_direction = calc_general_direction(x, y, f->x_home, f->y_home);
        }
    }
}

void formation_calculate_figures(void)
{
    formation_clear_figures();
    for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
        if (f->state != FIGURE_STATE_ALIVE) {
            continue;
        }
        if (!figure_is_legion(f) && !figure_is_enemy(f) && !figure_is_herd(f)) {
            continue;
        }
        if (f->type == FIGURE_ENEMY54_GLADIATOR) {
            continue;
        }
        int index = formation_add_figure(f->formation_id, i,
            f->formation_at_rest != 1, f->damage,
            figure_properties_for_type(f->type)->max_damage
        );
        f->index_in_formation = index;
    }

    enemy_army_totals_clear();
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        formation *m = formation_get(i);
        if (m->in_use && !m->is_herd) {
            if (m->is_legion) {
                if (m->num_figures > 0) {
                    int was_halted = m->is_halted;
                    m->is_halted = 1;
                    for (int fig = 0; fig < m->num_figures; fig++) {
                        int figure_id = m->figures[fig];
                        if (figure_id && figure_get(figure_id)->direction != DIR_8_NONE) {
                            m->is_halted = 0;
                        }
                    }
                    int total_strength = m->num_figures;
                    if (m->figure_type == FIGURE_FORT_LEGIONARY) {
                        total_strength += m->num_figures / 2;
                    }
                    enemy_army_totals_add_legion_formation(total_strength);
                    if (m->figure_type == FIGURE_FORT_LEGIONARY) {
                        if (!was_halted && m->is_halted) {
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
    }

    city_military_update_totals();
}

static void update_direction(int formation_id, int first_figure_direction)
{
    formation *f = &formations[formation_id];
    if (f->unknown_fired) {
        f->unknown_fired--;
    } else if (f->missile_fired) {
        f->direction = first_figure_direction;
    } else if (f->layout == FORMATION_DOUBLE_LINE_1 || f->layout == FORMATION_SINGLE_LINE_1) {
        if (f->y_home < f->prev.y_home) {
            f->direction = DIR_0_TOP;
        } else if (f->y_home > f->prev.y_home) {
            f->direction = DIR_4_BOTTOM;
        }
    } else if (f->layout == FORMATION_DOUBLE_LINE_2 || f->layout == FORMATION_SINGLE_LINE_2) {
        if (f->x_home < f->prev.x_home) {
            f->direction = DIR_6_LEFT;
        } else if (f->x_home > f->prev.x_home) {
            f->direction = DIR_2_RIGHT;
        }
    } else if (f->layout == FORMATION_TORTOISE || f->layout == FORMATION_COLUMN) {
        int dx = (f->x_home < f->prev.x_home) ? (f->prev.x_home - f->x_home) : (f->x_home - f->prev.x_home);
        int dy = (f->y_home < f->prev.y_home) ? (f->prev.y_home - f->y_home) : (f->y_home - f->prev.y_home);
        if (dx > dy) {
            if (f->x_home < f->prev.x_home) {
                f->direction = DIR_6_LEFT;
            } else if (f->x_home > f->prev.x_home) {
                f->direction = DIR_2_RIGHT;
            }
        } else {
            if (f->y_home < f->prev.y_home) {
                f->direction = DIR_0_TOP;
            } else if (f->y_home > f->prev.y_home) {
                f->direction = DIR_4_BOTTOM;
            }
        }
    }
    f->prev.x_home = f->x_home;
    f->prev.y_home = f->y_home;
}

static void update_directions(void)
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        formation *m = &formations[i];
        if (m->in_use && !m->is_herd) {
            update_direction(m->id, figure_get(m->figures[0])->direction);
        }
    }
}

static void set_legion_max_figures(void)
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        if (formations[i].in_use && formations[i].is_legion) {
            formations[i].max_figures = 16;
        }
    }
}

void formation_update_all(int second_time)
{
    formation_calculate_legion_totals();
    formation_calculate_figures();
    update_directions();
    formation_legion_decrease_damage();
    if (!second_time) {
        formation_update_monthly_morale_deployed();
    }
    set_legion_max_figures();
    formation_legion_update();
    formation_enemy_update();
    formation_herd_update();
}


void formations_save_state(buffer *buf, buffer *totals)
{
    for (int i = 0; i < MAX_FORMATIONS; i++) {
        formation *f = &formations[i];
        buffer_write_u8(buf, f->in_use);
        buffer_write_u8(buf, f->faction_id);
        buffer_write_u8(buf, f->legion_id);
        buffer_write_u8(buf, f->is_at_fort);
        buffer_write_i16(buf, f->figure_type);
        buffer_write_i16(buf, f->building_id);
        for (int fig = 0; fig < MAX_FORMATION_FIGURES; fig++) {
            buffer_write_i16(buf, f->figures[fig]);
        }
        buffer_write_u8(buf, f->num_figures);
        buffer_write_u8(buf, f->max_figures);
        buffer_write_i16(buf, f->layout);
        buffer_write_i16(buf, f->morale);
        buffer_write_u8(buf, f->x_home);
        buffer_write_u8(buf, f->y_home);
        buffer_write_u8(buf, f->standard_x);
        buffer_write_u8(buf, f->standard_y);
        buffer_write_u8(buf, f->x);
        buffer_write_u8(buf, f->y);
        buffer_write_u8(buf, f->destination_x);
        buffer_write_u8(buf, f->destination_y);
        buffer_write_i16(buf, f->destination_building_id);
        buffer_write_i16(buf, f->standard_figure_id);
        buffer_write_u8(buf, f->is_legion);
        buffer_skip(buf, 1);
        buffer_write_i16(buf, f->attack_type);
        buffer_write_i16(buf, f->legion_recruit_type);
        buffer_write_i16(buf, f->has_military_training);
        buffer_write_i16(buf, f->total_damage);
        buffer_write_i16(buf, f->max_total_damage);
        buffer_write_i16(buf, f->wait_ticks);
        buffer_write_i16(buf, f->recent_fight);
        buffer_write_i16(buf, f->enemy_state.duration_advance);
        buffer_write_i16(buf, f->enemy_state.duration_regroup);
        buffer_write_i16(buf, f->enemy_state.duration_halt);
        buffer_write_i16(buf, f->enemy_legion_index);
        buffer_write_i16(buf, f->is_halted);
        buffer_write_i16(buf, f->missile_fired);
        buffer_write_i16(buf, f->missile_attack_timeout);
        buffer_write_i16(buf, f->missile_attack_formation_id);
        buffer_write_i16(buf, f->prev.layout);
        buffer_write_i16(buf, f->cursed_by_mars);
        buffer_write_u8(buf, f->months_low_morale);
        buffer_write_u8(buf, f->empire_service);
        buffer_write_u8(buf, f->in_distant_battle);
        buffer_write_u8(buf, f->is_herd);
        buffer_write_u8(buf, f->enemy_type);
        buffer_write_u8(buf, f->direction);
        buffer_write_u8(buf, f->prev.x_home);
        buffer_write_u8(buf, f->prev.y_home);
        buffer_write_u8(buf, f->unknown_fired);
        buffer_write_u8(buf, f->orientation);
        buffer_write_u8(buf, f->months_from_home);
        buffer_write_u8(buf, f->months_very_low_morale);
        buffer_write_u8(buf, f->invasion_id);
        buffer_write_u8(buf, f->herd_wolf_spawn_delay);
        buffer_write_u8(buf, f->herd_direction);
        buffer_skip(buf, 17);
        buffer_write_i16(buf, f->invasion_sequence);
    }
    buffer_write_i32(totals, data.id_last_in_use);
    buffer_write_i32(totals, data.id_last_legion);
    buffer_write_i32(totals, data.num_legions);
}

void formations_load_state(buffer *buf, buffer *totals)
{
    data.id_last_in_use = buffer_read_i32(totals);
    data.id_last_legion = buffer_read_i32(totals);
    data.num_legions = buffer_read_i32(totals);
    for (int i = 0; i < MAX_FORMATIONS; i++) {
        formation *f = &formations[i];
        f->id = i;
        f->in_use = buffer_read_u8(buf);
        f->faction_id = buffer_read_u8(buf);
        f->legion_id = buffer_read_u8(buf);
        f->is_at_fort = buffer_read_u8(buf);
        f->figure_type = buffer_read_i16(buf);
        f->building_id = buffer_read_i16(buf);
        for (int fig = 0; fig < MAX_FORMATION_FIGURES; fig++) {
            f->figures[fig] = buffer_read_i16(buf);
        }
        f->num_figures = buffer_read_u8(buf);
        f->max_figures = buffer_read_u8(buf);
        f->layout = buffer_read_i16(buf);
        f->morale = buffer_read_i16(buf);
        f->x_home = buffer_read_u8(buf);
        f->y_home = buffer_read_u8(buf);
        f->standard_x = buffer_read_u8(buf);
        f->standard_y = buffer_read_u8(buf);
        f->x = buffer_read_u8(buf);
        f->y = buffer_read_u8(buf);
        f->destination_x = buffer_read_u8(buf);
        f->destination_y = buffer_read_u8(buf);
        f->destination_building_id = buffer_read_i16(buf);
        f->standard_figure_id = buffer_read_i16(buf);
        f->is_legion = buffer_read_u8(buf);
        buffer_skip(buf, 1);
        f->attack_type = buffer_read_i16(buf);
        f->legion_recruit_type = buffer_read_i16(buf);
        f->has_military_training = buffer_read_i16(buf);
        f->total_damage = buffer_read_i16(buf);
        f->max_total_damage = buffer_read_i16(buf);
        f->wait_ticks = buffer_read_i16(buf);
        f->recent_fight = buffer_read_i16(buf);
        f->enemy_state.duration_advance = buffer_read_i16(buf);
        f->enemy_state.duration_regroup = buffer_read_i16(buf);
        f->enemy_state.duration_halt = buffer_read_i16(buf);
        f->enemy_legion_index = buffer_read_i16(buf);
        f->is_halted = buffer_read_i16(buf);
        f->missile_fired = buffer_read_i16(buf);
        f->missile_attack_timeout = buffer_read_i16(buf);
        f->missile_attack_formation_id = buffer_read_i16(buf);
        f->prev.layout = buffer_read_i16(buf);
        f->cursed_by_mars = buffer_read_i16(buf);
        f->months_low_morale = buffer_read_u8(buf);
        f->empire_service = buffer_read_u8(buf);
        f->in_distant_battle = buffer_read_u8(buf);
        f->is_herd = buffer_read_u8(buf);
        f->enemy_type = buffer_read_u8(buf);
        f->direction = buffer_read_u8(buf);
        f->prev.x_home = buffer_read_u8(buf);
        f->prev.y_home = buffer_read_u8(buf);
        f->unknown_fired = buffer_read_u8(buf);
        f->orientation = buffer_read_u8(buf);
        f->months_from_home = buffer_read_u8(buf);
        f->months_very_low_morale = buffer_read_u8(buf);
        f->invasion_id = buffer_read_u8(buf);
        f->herd_wolf_spawn_delay = buffer_read_u8(buf);
        f->herd_direction = buffer_read_u8(buf);
        buffer_skip(buf, 17);
        f->invasion_sequence = buffer_read_i16(buf);
    }
}
