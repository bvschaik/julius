#ifndef WIDGET_OVERLAY_RISKS_H
#define WIDGET_OVERLAY_RISKS_H

#include "overlay.h"

void overlay_problems_prepare_building(building *b);

const city_overlay *overlay_for_fire();

const city_overlay *overlay_for_damage();

const city_overlay *overlay_for_crime();

const city_overlay *overlay_for_problems();

const city_overlay *overlay_for_native();

#endif // WIDGET_OVERLAY_RISKS_H
