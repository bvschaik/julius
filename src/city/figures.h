#ifndef CITY_FIGURES_H
#define CITY_FIGURES_H

void city_figures_reset();

void city_figures_add_animal();
void city_figures_add_attacking_native();
void city_figures_add_enemy();
void city_figures_add_imperial_soldier();
void city_figures_add_rioter(int is_attacking);
void city_figures_add_soldier();
void city_figures_set_gladiator_revolt();

int city_figures_animals();
int city_figures_attacking_natives();
int city_figures_imperial_soldiers();
int city_figures_enemies();
int city_figures_rioters();
int city_figures_soldiers();

int city_figures_total_invading_enemies();
int city_figures_has_security_breach();

#endif // CITY_FIGURES_H
