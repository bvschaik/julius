#include "WalkerAction_private.h"

const int walkerActionCorpseGraphicOffsets[128] = {
	0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

void WalkerAction_Common_handleCorpse(int walkerId)
{
	if (Data_Walkers[walkerId].waitTicks < 0) {
		Data_Walkers[walkerId].waitTicks = 0;
	}
	Data_Walkers[walkerId].waitTicks++;
	if (Data_Walkers[walkerId].waitTicks >= 128) {
		Data_Walkers[walkerId].waitTicks = 127;
		Data_Walkers[walkerId].state = WalkerState_Dead;
	}
}

void WalkerAction_Common_handleAttack(int walkerId)
{
	// TODO
}
