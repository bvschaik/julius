#ifndef FIGUREACTION_H
#define FIGUREACTION_H

#include "figure/figure.h"

void FigureAction_handle();

int FigureAction_HerdEnemy_moveFormationTo(int formationId, int x, int y, int *xTile, int *yTile);

void FigureAction_Common_handleCorpse(figure *f);
void FigureAction_Common_handleAttack(figure *f);
void FigureAction_Common_setCrossCountryDestination(figure *f, int xDst, int yDst);

int FigureAction_CombatSoldier_getMissileTarget(figure *shooter, int maxDistance, int *xTile, int *yTile);
int FigureAction_CombatSoldier_getTarget(int x, int y, int maxDistance);
int FigureAction_CombatWolf_getTarget(int x, int y, int maxDistance);
int FigureAction_CombatEnemy_getTarget(int x, int y);
int FigureAction_CombatEnemy_getMissileTarget(figure *enemy, int maxDistance, int attackCitizens, int *xTile, int *yTile);

void FigureAction_Combat_attackFigureAt(figure *f, int grid_offset);

#endif
