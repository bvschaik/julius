#ifndef DATA_STATE_H
#define DATA_STATE_H

#include "core/time.h"

enum {
	Overlay_None = 0,
	Overlay_Water = 2,
	Overlay_Religion = 4,
	Overlay_Fire = 8,
	Overlay_Damage = 9,
	Overlay_Crime = 10,
	Overlay_Entertainment = 11,
	Overlay_Theater = 12,
	Overlay_Amphitheater = 13,
	Overlay_Colosseum = 14,
	Overlay_Hippodrome = 15,
	Overlay_Education = 16,
	Overlay_School = 17,
	Overlay_Library = 18,
	Overlay_Academy = 19,
	Overlay_Barber = 20,
	Overlay_Bathhouse = 21,
	Overlay_Clinic = 22,
	Overlay_Hospital = 23,
	Overlay_TaxIncome = 24,
	Overlay_FoodStocks = 25,
	Overlay_Desirability = 26,
	Overlay_Workers_Unused = 27,
	Overlay_Native = 28,
	Overlay_Problems = 29
};

enum {
	WinState_Lose = -1,
	WinState_None = 0,
	WinState_Win = 1
};

extern struct _Data_State {
    int gamePaused;
	int sidebarCollapsed;
	int undoReady;
	int undoAvailable;
	int currentOverlay;
	int previousOverlay;
	struct {
		int type;
		int placementInProgress;
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
		int cost;
		int reservoirOffsetX;
		int reservoirOffsetY;
	} selectedBuilding;
	int selectedLegionFormationId;
	int winState;
	int forceWinCheat;
	int missionBriefingShown;
    struct {
        struct {
            int x;
            int y;
        } camera;
        struct {
            int gridOffset;
            int x;
            int y;
        } current;
        int width;
        int height;
        int gridStartOffset;
        int gridBorderSize;
        int orientation;
    } map;
} Data_State;

#endif
