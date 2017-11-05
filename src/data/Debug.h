#ifndef DATA_DEBUG_H
#define DATA_DEBUG_H

extern struct _Data_Debug {
	int incorrectHousePositions;
	int unfixableHousePositions;
	int maxConnectsEver; // unused, only in saved game
} Data_Debug;

#endif
