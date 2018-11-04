#include "figures.h"

#include "city/data_private.h"

void city_figures_reset(void)
{
    city_data.figure.enemies = 0;
    city_data.figure.rioters = 0;
    city_data.figure.attacking_natives = 0;
    city_data.figure.animals = 0;
    city_data.figure.imperial_soldiers = 0;
    city_data.figure.soldiers = 0;
    if (city_data.figure.security_breach_duration > 0) {
        city_data.figure.security_breach_duration--;
    }
}

void city_figures_add_animal(void)
{
    city_data.figure.animals++;
}

void city_figures_add_attacking_native(void)
{
    city_data.figure.security_breach_duration = 10;
    city_data.figure.attacking_natives++;
}

void city_figures_add_enemy(void)
{
    city_data.figure.enemies++;
}

void city_figures_add_imperial_soldier(void)
{
    city_data.figure.imperial_soldiers++;
}

void city_figures_add_rioter(int is_attacking)
{
    city_data.figure.rioters++;
    if (is_attacking) {
        city_data.figure.security_breach_duration = 10;
    }
}

void city_figures_add_soldier(void)
{
    city_data.figure.soldiers++;
}

void city_figures_set_gladiator_revolt(void)
{
    city_data.figure.attacking_natives = 10;
}

int city_figures_animals(void)
{
    return city_data.figure.animals;
}

int city_figures_attacking_natives(void)
{
    return city_data.figure.attacking_natives;
}

int city_figures_imperial_soldiers(void)
{
    return city_data.figure.imperial_soldiers;
}

int city_figures_enemies(void)
{
    return city_data.figure.enemies;
}

int city_figures_rioters(void)
{
    return city_data.figure.rioters;
}

int city_figures_soldiers(void)
{
    return city_data.figure.soldiers;
}

int city_figures_total_invading_enemies(void)
{
    return city_data.figure.imperial_soldiers + city_data.figure.enemies;
}

int city_figures_has_security_breach(void)
{
    return city_data.figure.security_breach_duration > 0;
}
