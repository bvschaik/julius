#ifndef WALKERMOVEMENT_H
#define WALKERMOVEMENT_H

void WalkerMovement_initRoaming(int walkerId);

void WalkerMovement_roamTicks(int walkerId, int numTicks);

void WalkerMovement_walkTicks(int walkerId, int numTicks);

void WalkerMovement_followTicks(int walkerId, int leaderWalkerId, int numTicks);

void WalkerMovement_crossCountrySetDirection(int walkerId, int xSrc, int ySrc, int xDst, int yDst, int isProjectile);
int WalkerMovement_crossCountryWalkTicks(int walkerId, int numTicks);

#endif
