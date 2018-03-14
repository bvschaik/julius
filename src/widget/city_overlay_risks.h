#ifndef WIGET_CITY_OVERLAY_RISKS_H
#define WIGET_CITY_OVERLAY_RISKS_H

#include "city_overlay.h"

void overlay_problems_prepare_building(building *b);

const city_overlay *city_overlay_for_fire();

const city_overlay *city_overlay_for_damage();

const city_overlay *city_overlay_for_crime();

const city_overlay *city_overlay_for_problems();

const city_overlay *city_overlay_for_native();

#endif // WIGET_CITY_OVERLAY_RISKS_H
