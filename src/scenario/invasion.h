#ifndef SCENARIO_INVASION_H
#define SCENARIO_INVASION_H

#include "core/buffer.h"
#include "figure/formation.h"
#include "scenario/types.h"

#define MAX_ORIGINAL_INVASIONS 20
#define INVASIONS_REPEAT_INFINITE -1

typedef enum {
    INVASION_OLD_STATE_FIRST_SECTION = 0,
    INVASION_OLD_STATE_LAST_SECTION = 1
} invasion_old_state_sections;

typedef struct {
    unsigned int id;
    int year;
    int type;
    struct {
        unsigned int min;
        unsigned int max;
    } amount;
    int from;
    int attack_type;
    int month;
    struct {
        int times;
        struct {
            unsigned int min;
            unsigned int max;
        } interval; // in years
    } repeat;
} invasion_t;

void scenario_invasion_clear(void);
void scenario_invasion_init(void);

int scenario_invasion_new(void);

const invasion_t *scenario_invasion_get(int id);
void scenario_invasion_update(const invasion_t *invasion);
void scenario_invasion_delete(int id);

int scenario_invasion_exists_upcoming(void);

void scenario_invasion_foreach_warning(void (*callback)(int x, int y, int image_id));

int scenario_invasion_count_total(void);
int scenario_invasion_count_active(void);
int scenario_invasion_count_active_from_buffer(buffer *buf);

int scenario_invasion_get_years_remaining(void);

int scenario_invasion_start_from_mars(void);

int scenario_invasion_start_from_caesar(int size);

void scenario_invasion_start_from_cheat(void);

void scenario_invasion_start_from_action(invasion_type_enum invasion_type, int size, int invasion_point, formation_attack_enum attack_type, enemy_type_t enemy_id);
void scenario_invasion_start_from_console(invasion_type_enum invasion_type, int size, int invasion_point);

void scenario_invasion_process(void);

void scenario_invasion_warning_save_state(buffer *invasion_id, buffer *warnings);

void scenario_invasion_warning_load_state(buffer *invasion_id, buffer *warnings, int has_dynamic_warnings);

void scenario_invasion_save_state(buffer *buf);

void scenario_invasion_load_state(buffer *buf);
void scenario_invasion_load_state_old_version(buffer *buf, invasion_old_state_sections section);

#endif // SCENARIO_INVASION_H
