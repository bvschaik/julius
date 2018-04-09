#ifndef DATA_STATE_H
#define DATA_STATE_H

#include "core/time.h"
#include "map/point.h"

extern struct _Data_State {
	struct {
		int gridOffsetStart;
		int gridOffsetEnd;
		int roadRequired;
		time_millis roadLastUpdate;
		int drawAsConstructing;
	} selectedBuilding;
	int missionBriefingShown;
} Data_State;

#endif
