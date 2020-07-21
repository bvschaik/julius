#ifndef FIGURE_PROPERTIES_H
#define FIGURE_PROPERTIES_H

#include "figure/type.h"

typedef enum {
    FIGURE_CATEGORY_INACTIVE= 0,
    FIGURE_CATEGORY_CITIZEN = 1,
    FIGURE_CATEGORY_ARMED = 2,
    FIGURE_CATEGORY_HOSTILE = 3,
    FIGURE_CATEGORY_CRIMINAL = 4,
    FIGURE_CATEGORY_NATIVE = 5,
    FIGURE_CATEGORY_ANIMAL = 6
} figure_category;

typedef struct {
    figure_category category;
    int max_damage;
    int attack_value;
    int defense_value;
    int missile_defense_value;
    int missile_attack_value;
    int missile_delay;
} figure_properties;

const figure_properties *figure_properties_for_type(figure_type type);

#endif // FIGURE_PROPERTIES_H
