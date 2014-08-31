#include "WalkerAction_private.h"

#include "Building.h"
#include "PlayerMessage.h"
#include "Time.h"

#include "Data/Grid.h"
#include "Data/Message.h"

static const int criminalOffsets[] =
	{0, 0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1};

void WalkerAction_protestor(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = 1;
	WalkerActionIncreaseGraphicOffset(w, 64);
	w->cartGraphicId = 0;
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->state = WalkerState_Dead;
	}
	w->waitTicks++;
	if (w->waitTicks > 200) {
		w->state = WalkerState_Dead;
		w->graphicOffset = 0;
	}
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Criminal) +
			WalkerActionCorpseGraphicOffset(w) + 96;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Criminal) +
			criminalOffsets[w->graphicOffset / 4] + 104;
	}
}

void WalkerAction_criminal(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = 1;
	WalkerActionIncreaseGraphicOffset(w, 32);
	w->cartGraphicId = 0;
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->state = WalkerState_Dead;
	}
	w->waitTicks++;
	if (w->waitTicks > 200) {
		w->state = WalkerState_Dead;
		w->graphicOffset = 0;
	}
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Criminal) +
			WalkerActionCorpseGraphicOffset(w) + 96;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Criminal) +
			criminalOffsets[w->graphicOffset / 2] + 104;
	}
}

int WalkerAction_Rioter_collapseBuilding(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	for (int dir = 0; dir < 8; dir += 2) {
		int gridOffset = w->gridOffset + Constant_DirectionGridOffsets[dir];
		if (!Data_Grid_buildingIds[gridOffset]) {
			continue;
		}
		int buildingId = Data_Grid_buildingIds[gridOffset];
		struct Data_Building *b = &Data_Buildings[buildingId];
		switch (b->type) {
			case Building_WarehouseSpace:
			case Building_Warehouse:
			case Building_FortGround:
			case Building_FortGround__:
			case Building_BurningRuin:
				continue;
		}
		if (b->houseSize && b->subtype.houseLevel < HouseLevel_SmallCasa) {
			continue;
		}
		TimeMillis now = Time_getMillis();
		if (now - Data_Message.lastSoundTime.rioterCollapse <= 15000) {
			PlayerMessage_disableSoundForNextMessage();
		} else {
			Data_Message.lastSoundTime.rioterCollapse = now;
		}
		PlayerMessage_post(0, 14, b->type, w->gridOffset);
		Data_Message.messageCategoryCount[MessageDelay_RiotCollapse]++;
		Building_collapseOnFire(buildingId, 0);
		w->actionState = WalkerActionState_120_RioterCreated;
		w->waitTicks = 0;
		w->direction = dir;
		return 1;
	}
	return 0;
}

int WalkerAction_Rioter_getTargetBuilding(int *xTile, int *yTile)
{
	// TODO
	return 0;
}
