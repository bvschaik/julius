#ifndef FIGUREMOVEMENT_H
#define FIGUREMOVEMENT_H

#include "figure/figure.h"

void FigureMovement_initRoaming(figure *f);

void FigureMovement_roamTicks(figure *f, int numTicks);

void FigureMovement_walkTicks(figure *f, int numTicks);

void FigureMovement_followTicks(figure *f, int numTicks);

void FigureMovement_walkTicksTowerSentry(figure *f, int numTicks);

void FigureMovement_setCrossCountryDestination(figure *f, int x_dst, int y_dst);
void FigureMovement_crossCountrySetDirection(figure *f, int xSrc, int ySrc, int xDst, int yDst, int isProjectile);
int FigureMovement_crossCountryWalkTicks(figure *f, int numTicks);

int FigureMovement_canLaunchCrossCountryMissile(int xSrc, int ySrc, int xDst, int yDst);

void FigureMovement_advanceTick(figure *f);

#endif
