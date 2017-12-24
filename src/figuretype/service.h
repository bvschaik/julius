#ifndef FIGURETYPE_SERVICE_H
#define FIGURETYPE_SERVICE_H

#include "figure/figure.h"

#include "service.h"

#include "building/building.h"
#include "building/market.h"
#include "figure/image.h"
#include "figure/route.h"
#include "graphics/image.h"

#include "FigureAction.h"
#include "FigureMovement.h"
#include "Terrain.h"

void figure_priest_action(figure *f);

void figure_school_child_action(figure *f);

void figure_teacher_action(figure *f);

void figure_librarian_action(figure *f);

void figure_barber_action(figure *f);

void figure_bathhouse_worker_action(figure *f);

void figure_doctor_action(figure *f);

void figure_missionary_action(figure *f);

void figure_patrician_action(figure *f);

void figure_labor_seeker_action(figure *f);

void figure_market_trader_action(figure *f);

#endif // FIGURETYPE_SERVICE_H
