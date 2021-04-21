#ifndef CITY_FIGURES_H
#define CITY_FIGURES_H

void city_figures_reset(void);

void city_figures_add_animal(void);
void city_figures_add_attacking_native(void);
void city_figures_add_enemy(void);
void city_figures_add_imperial_soldier(void);
void city_figures_add_rioter(int is_attacking);
void city_figures_add_robber(int is_attacking);
void city_figures_add_looter(int is_attacking);
void city_figures_add_protester(void);
void city_figures_add_soldier(void);
void city_figures_set_gladiator_revolt(void);

int city_figures_animals(void);
int city_figures_attacking_natives(void);
int city_figures_imperial_soldiers(void);
int city_figures_enemies(void);
int city_figures_rioters(void);
int city_figures_looters(void);
int city_figures_robbers(void);
int city_figures_protesters(void);
int city_figures_criminals(void);
int city_figures_soldiers(void);

int city_figures_total_invading_enemies(void);
int city_figures_has_security_breach(void);

#endif // CITY_FIGURES_H
