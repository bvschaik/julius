#ifndef CITY_MILITARY_H
#define CITY_MILITARY_H

void city_military_clear_legionary_legions();
void city_military_add_legionary_legion();
int city_military_has_legionary_legions();

int city_military_total_legions();
int city_military_total_soldiers();
int city_military_empire_service_legions();
void city_military_clear_empire_service_legions();

void city_military_update_totals();

void city_military_determine_distant_battle_city();
int city_military_distant_battle_city();
int city_military_distant_battle_city_is_roman();

int city_military_distant_battle_enemy_strength();

void city_military_dispatch_to_distant_battle(int roman_strength);
int city_military_distant_battle_roman_army_is_traveling();
int city_military_distant_battle_roman_army_is_traveling_forth();
int city_military_distant_battle_roman_army_is_traveling_back();

void city_military_init_distant_battle(int enemy_strength);
int city_military_has_distant_battle();
int city_military_months_until_distant_battle();

void city_military_process_distant_battle();

#endif // CITY_MILITARY_H
