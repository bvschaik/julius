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

// figure action callbacks
// enemy
void FigureAction_enemy43_Spear(figure *f);
void FigureAction_enemy44_Sword(figure *f);
void FigureAction_enemy45_Sword(figure *f);
void FigureAction_enemy46_Camel(figure *f);
void FigureAction_enemy47_Elephant(figure *f);
void FigureAction_enemy48_Chariot(figure *f);
void FigureAction_enemy49_FastSword(figure *f);
void FigureAction_enemy50_Sword(figure *f);
void FigureAction_enemy51_Spear(figure *f);
void FigureAction_enemy52_MountedArcher(figure *f);
void FigureAction_enemy53_Axe(figure *f);
void FigureAction_enemy54_Gladiator(figure *f);
void FigureAction_enemyCaesarLegionary(figure *f);

#endif
