#ifndef CITY_MILITARY_H
#define CITY_MILITARY_H

void city_military_clear_legionary_legions(void);
void city_military_add_legionary_legion(void);
int city_military_has_legionary_legions(void);

int city_military_total_legions(void);
int city_military_total_soldiers(void);
int city_military_empire_service_legions(void);
void city_military_clear_empire_service_legions(void);

void city_military_update_totals(void);

int city_military_is_native_attack_active(void);
void city_military_start_native_attack(void);
void city_military_decrease_native_attack_duration(void);

void city_military_determine_distant_battle_city(void);
int city_military_distant_battle_city(void);
int city_military_distant_battle_city_is_roman(void);

int city_military_distant_battle_enemy_strength(void);

void city_military_dispatch_to_distant_battle(int roman_strength);
int city_military_distant_battle_roman_army_is_traveling(void);
int city_military_distant_battle_roman_army_is_traveling_forth(void);
int city_military_distant_battle_roman_army_is_traveling_back(void);

int city_military_distant_battle_enemy_months_traveled(void);
int city_military_distant_battle_roman_months_traveled(void);

void city_military_init_distant_battle(int enemy_strength);
int city_military_has_distant_battle(void);
int city_military_months_until_distant_battle(void);

void city_military_process_distant_battle(void);

#endif // CITY_MILITARY_H
