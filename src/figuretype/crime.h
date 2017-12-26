#ifndef FIGURETYPE_CRIME_H
#define FIGURETYPE_CRIME_H

#include "figure/figure.h"

void figure_protestor_action(figure *f);

void figure_criminal_action(figure *f);

void figure_rioter_action(figure *f);

int figure_rioter_collapse_building(figure *f);

#endif // FIGURETYPE_CRIME_H
