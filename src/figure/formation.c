#include "formation.h"

#include "core/calc.h"
#include "figure/type.h"

#include <string.h>

#define MAX_FORMATIONS 50

typedef struct {
    unsigned char ciid;
    short layoutBeforeMopUp;
    unsigned char monthsLowMorale;
    unsigned char prevXHome;
    unsigned char prevYHome;
    unsigned char __unknown66;
    unsigned char monthsFromHome;
    unsigned char monthsVeryLowMorale;
    unsigned char herdWolfSpawnDelay;
    short invasionSeq;
    
    formation visible;
} full_formation;

static full_formation formations[MAX_FORMATIONS];

static struct {
    int id_last_in_use;
    int id_last_legion;
    int num_legions;
} data;

void formations_clear()
{
    for (int i = 0; i < MAX_FORMATIONS; i++) {
        memset(&formations[i], 0, sizeof(full_formation));
        formations[i].visible.id = i;
    }
    data.id_last_in_use = 0;
    data.id_last_legion = 0;
    data.num_legions = 0;
}

void formation_clear(int formation_id)
{
    memset(&formations[formation_id], 0, sizeof(full_formation));
    formations[formation_id].visible.id = formation_id;
}

static int get_free_formation(int start_index)
{
    for (int i = start_index; i < MAX_FORMATIONS; i++) {
        if (!formations[i].visible.in_use) {
            return i;
        }
    }
    return 0;
}

int formation_create_legion(int building_id, int x, int y, figure_type figure_type)
{
    int formation_id = get_free_formation(1);
    if (!formation_id) {
        return 0;
    }
    formation *m = &formations[formation_id].visible;
    formations[formation_id].ciid = 1;
    m->in_use = 1;
    m->is_legion = 1;
    m->figure_type = figure_type;
    m->building_id = building_id;
    m->layout = FORMATION_DOUBLE_LINE_1;
    m->morale = 50;
    m->is_at_fort = 1;
    m->legion_id = formation_id - 1;
    m->x = m->x_standard = m->x_home = x + 3;
    m->y = m->y_standard = m->y_home = y - 1;
    
    data.num_legions++;
    if (formation_id > data.id_last_in_use) {
        data.id_last_in_use = formation_id;
    }
    return formation_id;
}

static int formation_create(int figure_type, int layout, int orientation, int x, int y)
{
    int formation_id = get_free_formation(10);
    if (!formation_id) {
        return 0;
    }
    formation *m = &formations[formation_id].visible;
    formations[formation_id].ciid = 0;
    m->x = x;
    m->y = y;
    m->in_use = 1;
    m->is_legion = 0;
    m->figure_type = figure_type;
    m->legion_id = formation_id - 10;
    m->morale = 100;
    if (layout == FORMATION_ENEMY10) {
        if (orientation == DIR_0_TOP || orientation == DIR_4_BOTTOM) {
            m->layout = FORMATION_DOUBLE_LINE_1;
        } else {
            m->layout = FORMATION_DOUBLE_LINE_2;
        }
    } else {
        m->layout = layout;
    }
    return formation_id;
}

int formation_create_herd(int figure_type, int x, int y, int num_animals)
{
    int formation_id = formation_create(figure_type, FORMATION_HERD, 0, x, y);
    if (!formation_id) {
        return 0;
    }
    full_formation *f = &formations[formation_id];
    f->visible.is_herd = 1;
    f->visible.wait_ticks = 24;
    f->visible.max_figures = num_animals;
    return formation_id;
}

int formation_create_enemy(int figure_type, int x, int y, int layout, int orientation,
                           int enemy_type, int attack_type, int invasion_id, int invasion_sequence)
{
    int formation_id = formation_create(figure_type, layout, orientation, x, y);
    if (!formation_id) {
        return 0;
    }
    full_formation *f = &formations[formation_id];
    f->visible.attack_type = attack_type;
    f->visible.orientation = orientation;
    f->visible.enemy_type = enemy_type;
    f->visible.invasion_id = invasion_id;
    f->invasionSeq = invasion_sequence;
    return formation_id;
}

const formation *formation_get(int formation_id)
{
    return &formations[formation_id].visible;
}

formation_state *formation_get_state(int formation_id)
{
    return &formations[formation_id].visible.enemy_state;
}


void formation_set_standard(int formation_id, int standard_figure_id)
{
    formations[formation_id].visible.standard_figure_id = standard_figure_id;
}

void formation_move_standard(int formation_id, int x, int y)
{
    formations[formation_id].visible.x_standard = x;
    formations[formation_id].visible.y_standard = y;
    formations[formation_id].visible.is_at_fort = 0;
}

void formation_set_figure_type(int formation_id, figure_type type)
{
    formations[formation_id].visible.figure_type = type;
}

void formation_set_recruit_type(int formation_id, int recruit_type)
{
    formations[formation_id].visible.legion_recruit_type = recruit_type;
}

void formation_set_halted(int formation_id, int halted)
{
    formations[formation_id].visible.is_halted = halted;
}

void formation_set_at_fort(int formation_id, int at_fort)
{
    formations[formation_id].visible.is_at_fort = at_fort;
}

void formation_set_distant_battle(int formation_id, int distant_battle)
{
    formations[formation_id].visible.in_distant_battle = distant_battle;
}

void formation_set_cursed(int formation_id)
{
    formations[formation_id].visible.cursed_by_mars = 96;
}

void formation_change_layout(int formation_id, int new_layout)
{
    full_formation *m = &formations[formation_id];
    if (new_layout == FORMATION_MOP_UP && m->visible.layout != FORMATION_MOP_UP) {
        m->layoutBeforeMopUp = m->visible.layout;
    }
    m->visible.layout = new_layout;
}

void formation_restore_layout(int formation_id)
{
    full_formation *f = &formations[formation_id];
    if (f->visible.layout == FORMATION_MOP_UP) {
        f->visible.layout = f->layoutBeforeMopUp;
    }
}

void formation_toggle_empire_service(int formation_id)
{
    full_formation *m = &formations[formation_id];
    m->visible.empire_service = m->visible.empire_service ? 0 : 1;
}

void formation_record_missile_fired(int formation_id)
{
    formations[formation_id].visible.missile_fired = 6;
}

void formation_record_missile_attack(int formation_id, int from_formation_id)
{
    formations[formation_id].visible.missile_attack_timeout = 6;
    formations[formation_id].visible.missile_attack_formation_id = from_formation_id;
}

void formation_record_fight(int formation_id)
{
    formations[formation_id].visible.recent_fight = 6;
}

int formation_for_invasion(int invasion_sequence)
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        full_formation *f = &formations[i];
        if (f->visible.in_use == 1 && !f->visible.is_legion && !f->visible.is_herd && f->invasionSeq == invasion_sequence) {
            return i;
        }
    }
    return 0;
}

void formation_caesar_pause()
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        if (formations[i].visible.in_use == 1 && formations[i].visible.figure_type == FIGURE_ENEMY_CAESAR_LEGIONARY) {
            formations[i].visible.wait_ticks = 20;
        }
    }
}

void formation_caesar_retreat()
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        if (formations[i].visible.in_use == 1 && formations[i].visible.figure_type == FIGURE_ENEMY_CAESAR_LEGIONARY) {
            formations[i].monthsLowMorale = 1;
        }
    }
}

void formation_foreach(void (*callback)(const formation *))
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        if (formations[i].visible.in_use) {
            callback(&formations[i].visible);
        }
    }
}

void formation_foreach_herd(void (*callback)(const formation *))
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        formation *m = &formations[i].visible;
        if (m->in_use && m->is_herd && !m->is_legion && m->num_figures > 0) {
            callback(&formations[i].visible);
        }
    }
}

void formation_foreach_legion(void (*callback)(const formation *, void *), void *data)
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        if (formations[i].visible.in_use && formations[i].visible.is_legion) {
            callback(&formations[i].visible, data);
        }
    }
}

void formation_foreach_non_herd(void (*callback)(const formation *, void *), void *data)
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        if (formations[i].visible.in_use && !formations[i].visible.is_herd) {
            callback(&formations[i].visible, data);
        }
    }
}

int formation_has_low_morale(int formation_id)
{
    return formations[formation_id].monthsLowMorale ||
           formations[formation_id].monthsVeryLowMorale;
}

void formation_legion_set_trained(int formation_id)
{
    formations[formation_id].visible.has_military_training = 1;
}

void formation_legion_set_max_figures()
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        if (formations[i].visible.in_use && formations[i].visible.is_legion) {
            formations[i].visible.max_figures = 16;
        }
    }
}

int formation_get_num_legions_cached()
{
    return data.num_legions;
}

void formation_cache_clear_legions()
{
    data.id_last_legion = 0;
    data.num_legions = 0;
}

void formation_cache_add_legion(int formation_id)
{
    data.id_last_legion = formation_id;
    data.num_legions++;
}

int formation_get_num_legions()
{
    int total = 0;
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        if (formations[i].visible.in_use && formations[i].visible.is_legion) {
            total++;
        }
    }
    return total;
}

int formation_for_legion(int legion_index)
{
    int index = 1;
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        if (formations[i].visible.in_use && formations[i].visible.is_legion) {
            if (index++ == legion_index) {
                return i;
            }
        }
    }
    return 0;
}

void formation_change_morale(int formation_id, int amount)
{
    formation *m = &formations[formation_id].visible;
    int max_morale;
    if (m->figure_type == FIGURE_FORT_LEGIONARY) {
        max_morale = m->has_military_training ? 100 : 80;
    } else if (m->figure_type == FIGURE_ENEMY_CAESAR_LEGIONARY) {
        max_morale = 100;
    } else if (m->figure_type == FIGURE_FORT_JAVELIN || m->figure_type == FIGURE_FORT_MOUNTED) {
        max_morale = m->has_military_training ? 80 : 60;
    } else {
        switch (m->enemy_type) {
            case 0:
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

int formation_legion_prepare_to_move(int formation_id)
{
    full_formation *f = &formations[formation_id];
    if (f->monthsVeryLowMorale || f->monthsLowMorale > 1) {
        return 0;
    }
    if (f->monthsLowMorale == 1) {
        formation_change_morale(formation_id, 10); // yay, we can move!
    }
    return 1;
}

static void change_all_morale(int legion, int enemy)
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        if (formations[i].visible.in_use && !formations[i].visible.is_herd) {
            if (formations[i].visible.is_legion) {
                formation_change_morale(i, legion);
            } else {
                formation_change_morale(i, enemy);
            }
        }
    }
}

void formation_update_monthly_morale_deployed()
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        full_formation *m = &formations[i];
        if (m->visible.in_use != 1 || m->visible.is_herd) {
            continue;
        }
        if (m->visible.is_legion) {
            if (!m->visible.is_at_fort && !m->visible.in_distant_battle) {
                if (m->visible.morale <= 20 && !m->monthsLowMorale && !m->monthsVeryLowMorale) {
                    change_all_morale(-10, 10);
                }
                if (m->visible.morale <= 10) {
                    m->monthsVeryLowMorale++;
                } else if (m->visible.morale <= 20) {
                    m->monthsLowMorale++;
                }
            }
        } else { // enemy
            if (m->visible.morale <= 20 && !m->monthsLowMorale && !m->monthsVeryLowMorale) {
                change_all_morale(10, -10);
            }
            if (m->visible.morale <= 10) {
                m->monthsVeryLowMorale++;
            } else if (m->visible.morale <= 20) {
                m->monthsLowMorale++;
            }
        }
    }
}

void formation_update_monthly_morale_at_rest()
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        full_formation *m = &formations[i];
        if (m->visible.in_use != 1 || m->visible.is_herd) {
            continue;
        }
        if (m->visible.is_legion) {
            if (m->visible.is_at_fort) {
                m->monthsFromHome = 0;
                m->monthsVeryLowMorale = 0;
                m->monthsLowMorale = 0;
                formation_change_morale(i, 5);
                formation_restore_layout(i);
            } else if (!m->visible.recent_fight) {
                m->monthsFromHome++;
                if (m->monthsFromHome > 3) {
                    if (m->monthsFromHome > 100) {
                        m->monthsFromHome = 100;
                    }
                    formation_change_morale(i, -5);
                }
            }
        } else {
            formation_change_morale(i, 0);
        }
    }
}

void formation_decrease_monthly_counters(int formation_id)
{
    formation *m = &formations[formation_id].visible;
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

void formation_clear_monthly_counters(int formation_id)
{
    formation *m = &formations[formation_id].visible;
    m->missile_fired = 0;
    m->missile_attack_timeout = 0;
    m->recent_fight = 0;
}

void formation_set_destination(int formation_id, int x, int y)
{
    formation *m = &formations[formation_id].visible;
    m->destination_x = x;
    m->destination_y = y;
}

void formation_set_destination_building(int formation_id, int x, int y, int building_id)
{
    formation *m = &formations[formation_id].visible;
    m->destination_x = x;
    m->destination_y = y;
    m->destination_building_id = building_id;
}

void formation_set_home(int formation_id, int x, int y)
{
    formation *m = &formations[formation_id].visible;
    m->x_home = x;
    m->y_home = y;
}

void formation_clear_figures()
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        formation *m = &formations[i].visible;
        for (int f = 0; f < MAX_FORMATION_FIGURES; f++) {
            m->figures[f] = 0;
        }
        m->num_figures = 0;
        m->is_at_fort = 1;
        m->total_damage = 0;
        m->max_total_damage = 0;
    }
}

int formation_add_figure(int formation_id, int figure_id, int deployed, int damage, int max_damage)
{
    formation *m = &formations[formation_id].visible;
    m->num_figures++;
    m->total_damage += damage;
    m->max_total_damage += max_damage;
    if (deployed) {
        m->is_at_fort = 0;
    }
    for (int f = 0; f < MAX_FORMATION_FIGURES; f++) {
        if (!m->figures[f]) {
            m->figures[f] = figure_id;
            return f;
        }
    }
    return 0; // shouldn't happen
}

void formation_move_herds_away(int x, int y)
{
    for (int i = 1; i < MAX_FORMATIONS; i++) {
        formation *m = &formations[i].visible;
        if (m->in_use != 1 || m->is_legion || !m->is_herd || m->num_figures <= 0) {
            continue;
        }
        if (calc_maximum_distance(x, y, m->x_home, m->y_home) <= 6) {
            formations[i].visible.wait_ticks = 50;
            formations[i].visible.herd_direction = calc_general_direction(x, y, m->x_home, m->y_home);
        }
    }
}

int formation_can_spawn_wolf(int formation_id)
{
    full_formation *f = &formations[formation_id];
    formation *m = &f->visible;
    if (m->num_figures < m->max_figures && m->figure_type == FIGURE_WOLF) {
        f->herdWolfSpawnDelay++;
        if (f->herdWolfSpawnDelay > 32) {
            f->herdWolfSpawnDelay = 0;
            return 1;
        }
    }
    return 0;
}

void formation_update_direction(int formation_id, int first_figure_direction)
{
    full_formation *f = &formations[formation_id];
    if (f->__unknown66) {
        f->__unknown66--;
    } else if (f->visible.missile_fired) {
        f->visible.direction = first_figure_direction;
    } else if (f->visible.layout == FORMATION_DOUBLE_LINE_1 || f->visible.layout == FORMATION_SINGLE_LINE_1) {
        if (f->visible.y_home < f->prevYHome) {
            f->visible.direction = DIR_0_TOP;
        } else if (f->visible.y_home > f->prevYHome) {
            f->visible.direction = DIR_4_BOTTOM;
        }
    } else if (f->visible.layout == FORMATION_DOUBLE_LINE_2 || f->visible.layout == FORMATION_SINGLE_LINE_2) {
        if (f->visible.x_home < f->prevXHome) {
            f->visible.direction = DIR_6_LEFT;
        } else if (f->visible.x_home > f->prevXHome) {
            f->visible.direction = DIR_2_RIGHT;
        }
    } else if (f->visible.layout == FORMATION_TORTOISE || f->visible.layout == FORMATION_COLUMN) {
        int dx = (f->visible.x_home < f->prevXHome) ? (f->prevXHome - f->visible.x_home) : (f->visible.x_home - f->prevXHome);
        int dy = (f->visible.y_home < f->prevYHome) ? (f->prevYHome - f->visible.y_home) : (f->visible.y_home - f->prevYHome);
        if (dx > dy) {
            if (f->visible.x_home < f->prevXHome) {
                f->visible.direction = DIR_6_LEFT;
            } else if (f->visible.x_home > f->prevXHome) {
                f->visible.direction = DIR_2_RIGHT;
            }
        } else {
            if (f->visible.y_home < f->prevYHome) {
                f->visible.direction = DIR_0_TOP;
            } else if (f->visible.y_home > f->prevYHome) {
                f->visible.direction = DIR_4_BOTTOM;
            }
        }
    }
    f->prevXHome = f->visible.x_home;
    f->prevYHome = f->visible.y_home;
}

void formation_herd_clear_direction(int formation_id)
{
    formations[formation_id].visible.herd_direction = 0;
}

void formation_increase_wait_ticks(int formation_id)
{
    formations[formation_id].visible.wait_ticks++;
}

void formation_reset_wait_ticks(int formation_id)
{
    formations[formation_id].visible.wait_ticks = 0;
}

void formation_set_enemy_legion(int formation_id, int enemy_legion_index)
{
    formations[formation_id].visible.enemy_legion_index = enemy_legion_index;
}


void formations_save_state(buffer *buf, buffer *totals)
{
    for (int i = 0; i < MAX_FORMATIONS; i++) {
        full_formation *f = &formations[i];
        buffer_write_u8(buf, f->visible.in_use);
        buffer_write_u8(buf, f->ciid);
        buffer_write_u8(buf, f->visible.legion_id);
        buffer_write_u8(buf, f->visible.is_at_fort);
        buffer_write_i16(buf, f->visible.figure_type);
        buffer_write_i16(buf, f->visible.building_id);
        for (int fig = 0; fig < MAX_FORMATION_FIGURES; fig++) {
            buffer_write_i16(buf, f->visible.figures[fig]);
        }
        buffer_write_u8(buf, f->visible.num_figures);
        buffer_write_u8(buf, f->visible.max_figures);
        buffer_write_i16(buf, f->visible.layout);
        buffer_write_i16(buf, f->visible.morale);
        buffer_write_u8(buf, f->visible.x_home);
        buffer_write_u8(buf, f->visible.y_home);
        buffer_write_u8(buf, f->visible.x_standard);
        buffer_write_u8(buf, f->visible.y_standard);
        buffer_write_u8(buf, f->visible.x);
        buffer_write_u8(buf, f->visible.y);
        buffer_write_u8(buf, f->visible.destination_x);
        buffer_write_u8(buf, f->visible.destination_y);
        buffer_write_i16(buf, f->visible.destination_building_id);
        buffer_write_i16(buf, f->visible.standard_figure_id);
        buffer_write_u8(buf, f->visible.is_legion);
        buffer_skip(buf, 1);
        buffer_write_i16(buf, f->visible.attack_type);
        buffer_write_i16(buf, f->visible.legion_recruit_type);;
        buffer_write_i16(buf, f->visible.has_military_training);
        buffer_write_i16(buf, f->visible.total_damage);
        buffer_write_i16(buf, f->visible.max_total_damage);
        buffer_write_i16(buf, f->visible.wait_ticks);
        buffer_write_i16(buf, f->visible.recent_fight);
        buffer_write_i16(buf, f->visible.enemy_state.duration_advance);
        buffer_write_i16(buf, f->visible.enemy_state.duration_regroup);
        buffer_write_i16(buf, f->visible.enemy_state.duration_halt);
        buffer_write_i16(buf, f->visible.enemy_legion_index);
        buffer_write_i16(buf, f->visible.is_halted);
        buffer_write_i16(buf, f->visible.missile_fired);
        buffer_write_i16(buf, f->visible.missile_attack_timeout);
        buffer_write_i16(buf, f->visible.missile_attack_formation_id);
        buffer_write_i16(buf, f->layoutBeforeMopUp);
        buffer_write_i16(buf, f->visible.cursed_by_mars);
        buffer_write_u8(buf, f->monthsLowMorale);
        buffer_write_u8(buf, f->visible.empire_service);
        buffer_write_u8(buf, f->visible.in_distant_battle);
        buffer_write_u8(buf, f->visible.is_herd);
        buffer_write_u8(buf, f->visible.enemy_type);
        buffer_write_u8(buf, f->visible.direction);
        buffer_write_u8(buf, f->prevXHome);
        buffer_write_u8(buf, f->prevYHome);
        buffer_write_u8(buf, f->__unknown66);
        buffer_write_u8(buf, f->visible.orientation);
        buffer_write_u8(buf, f->monthsFromHome);
        buffer_write_u8(buf, f->monthsVeryLowMorale);
        buffer_write_u8(buf, f->visible.invasion_id);
        buffer_write_u8(buf, f->herdWolfSpawnDelay);
        buffer_write_u8(buf, f->visible.herd_direction);
        buffer_skip(buf, 17);
        buffer_write_i16(buf, f->invasionSeq);
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
        full_formation *f = &formations[i];
        f->visible.id = i;
        f->visible.in_use = buffer_read_u8(buf);
        f->ciid = buffer_read_u8(buf);
        f->visible.legion_id = buffer_read_u8(buf);
        f->visible.is_at_fort = buffer_read_u8(buf);
        f->visible.figure_type = buffer_read_i16(buf);
        f->visible.building_id = buffer_read_i16(buf);
        for (int fig = 0; fig < MAX_FORMATION_FIGURES; fig++) {
            f->visible.figures[fig] = buffer_read_i16(buf);
        }
        f->visible.num_figures = buffer_read_u8(buf);
        f->visible.max_figures = buffer_read_u8(buf);
        f->visible.layout = buffer_read_i16(buf);
        f->visible.morale = buffer_read_i16(buf);
        f->visible.x_home = buffer_read_u8(buf);
        f->visible.y_home = buffer_read_u8(buf);
        f->visible.x_standard = buffer_read_u8(buf);
        f->visible.y_standard = buffer_read_u8(buf);
        f->visible.x = buffer_read_u8(buf);
        f->visible.y = buffer_read_u8(buf);
        f->visible.destination_x = buffer_read_u8(buf);
        f->visible.destination_y = buffer_read_u8(buf);
        f->visible.destination_building_id = buffer_read_i16(buf);
        f->visible.standard_figure_id = buffer_read_i16(buf);
        f->visible.is_legion = buffer_read_u8(buf);
        buffer_skip(buf, 1);
        f->visible.attack_type = buffer_read_i16(buf);
        f->visible.legion_recruit_type = buffer_read_i16(buf);;
        f->visible.has_military_training = buffer_read_i16(buf);
        f->visible.total_damage = buffer_read_i16(buf);
        f->visible.max_total_damage = buffer_read_i16(buf);
        f->visible.wait_ticks = buffer_read_i16(buf);
        f->visible.recent_fight = buffer_read_i16(buf);
        f->visible.enemy_state.duration_advance = buffer_read_i16(buf);
        f->visible.enemy_state.duration_regroup = buffer_read_i16(buf);
        f->visible.enemy_state.duration_halt = buffer_read_i16(buf);
        f->visible.enemy_legion_index = buffer_read_i16(buf);
        f->visible.is_halted = buffer_read_i16(buf);
        f->visible.missile_fired = buffer_read_i16(buf);
        f->visible.missile_attack_timeout = buffer_read_i16(buf);
        f->visible.missile_attack_formation_id = buffer_read_i16(buf);
        f->layoutBeforeMopUp = buffer_read_i16(buf);
        f->visible.cursed_by_mars = buffer_read_i16(buf);
        f->monthsLowMorale = buffer_read_u8(buf);
        f->visible.empire_service = buffer_read_u8(buf);
        f->visible.in_distant_battle = buffer_read_u8(buf);
        f->visible.is_herd = buffer_read_u8(buf);
        f->visible.enemy_type = buffer_read_u8(buf);
        f->visible.direction = buffer_read_u8(buf);
        f->prevXHome = buffer_read_u8(buf);
        f->prevYHome = buffer_read_u8(buf);
        f->__unknown66 = buffer_read_u8(buf);
        f->visible.orientation = buffer_read_u8(buf);
        f->monthsFromHome = buffer_read_u8(buf);
        f->monthsVeryLowMorale = buffer_read_u8(buf);
        f->visible.invasion_id = buffer_read_u8(buf);
        f->herdWolfSpawnDelay = buffer_read_u8(buf);
        f->visible.herd_direction = buffer_read_u8(buf);
        buffer_skip(buf, 17);
        f->invasionSeq = buffer_read_i16(buf);
    }
}
