#ifndef DATA_STATE_H
#define DATA_STATE_H

#include "core/time.h"

enum {
	WinState_Lose = -1,
	WinState_None = 0,
	WinState_Win = 1
};

extern struct _Data_State {
	struct {
		int gridOffsetStart;
		int gridOffsetEnd;
		int wallRequired;
		int waterRequired;
		int treesRequired;
		int rockRequired;
		int meadowRequired;
		int roadRequired;
		time_millis roadLastUpdate;
		int drawAsConstructing;
		int reservoirOffsetX;
		int reservoirOffsetY;
	} selectedBuilding;
	int selectedLegionFormationId;
	int winState;
	int forceWinCheat;
	int missionBriefingShown;
    struct {
        struct {
            int gridOffset;
            int x;
            int y;
        } current;
        int width;
        int height;
        int gridStartOffset;
        int gridBorderSize;
    } map;
} Data_State;

#endif
