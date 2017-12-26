#ifndef FIGUREACTION_H
#define FIGUREACTION_H

#include "figure/figure.h"

void FigureAction_handle();

int FigureAction_HerdEnemy_moveFormationTo(int formationId, int x, int y, int *xTile, int *yTile);

#endif
