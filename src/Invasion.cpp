#include "Invasion.h"
#include "Data/Invasion.h"

int Invasion_existsUpcomingInvasion()
{
	for (int i = 0; i < 101; i++) {
		if (Data_InvasionWarnings[i].inUse && Data_InvasionWarnings[i].handled) {
			return 1;
		}
	}
	return 0;
}
