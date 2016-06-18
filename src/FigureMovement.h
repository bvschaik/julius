#ifndef FIGUREMOVEMENT_H
#define FIGUREMOVEMENT_H

void FigureMovement_initRoaming(int walkerId);

void FigureMovement_roamTicks(int walkerId, int numTicks);

void FigureMovement_walkTicks(int walkerId, int numTicks);

void FigureMovement_followTicks(int walkerId, int leaderWalkerId, int numTicks);

void FigureMovement_walkTicksTowerSentry(int walkerId, int numTicks);

void FigureMovement_crossCountrySetDirection(int walkerId, int xSrc, int ySrc, int xDst, int yDst, int isProjectile);
int FigureMovement_crossCountryWalkTicks(int walkerId, int numTicks);

int FigureMovement_canLaunchCrossCountryMissile(int xSrc, int ySrc, int xDst, int yDst);

void FigureMovement_advanceTick(struct Data_Walker *w);

#endif
