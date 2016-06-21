#ifndef FIGUREMOVEMENT_H
#define FIGUREMOVEMENT_H

void FigureMovement_initRoaming(int figureId);

void FigureMovement_roamTicks(int figureId, int numTicks);

void FigureMovement_walkTicks(int figureId, int numTicks);

void FigureMovement_followTicks(int figureId, int leaderFigureId, int numTicks);

void FigureMovement_walkTicksTowerSentry(int figureId, int numTicks);

void FigureMovement_crossCountrySetDirection(int figureId, int xSrc, int ySrc, int xDst, int yDst, int isProjectile);
int FigureMovement_crossCountryWalkTicks(int figureId, int numTicks);

int FigureMovement_canLaunchCrossCountryMissile(int xSrc, int ySrc, int xDst, int yDst);

void FigureMovement_advanceTick(struct Data_Walker *f);

#endif
