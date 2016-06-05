#include "WalkerAction_private.h"

#include "Building.h"
#include "Formation.h"
#include "PlayerMessage.h"
#include "Time.h"
#include "Walker.h"
#include "WalkerMovement.h"

#include "Data/CityInfo.h"
#include "Data/Grid.h"
#include "Data/Message.h"

static const int criminalOffsets[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1
};

void WalkerAction_protestor(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = FigureTerrainUsage_Roads;
	WalkerActionIncreaseGraphicOffset(w, 64);
	w->cartGraphicId = 0;
	if (w->actionState == FigureActionState_149_Corpse) {
		w->state = FigureState_Dead;
	}
	w->waitTicks++;
	if (w->waitTicks > 200) {
		w->state = FigureState_Dead;
		w->graphicOffset = 0;
	}
	if (w->actionState == FigureActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Figure_Criminal) +
			WalkerActionCorpseGraphicOffset(w) + 96;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Figure_Criminal) +
			criminalOffsets[w->graphicOffset / 4] + 104;
	}
}

void WalkerAction_criminal(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = FigureTerrainUsage_Roads;
	WalkerActionIncreaseGraphicOffset(w, 32);
	w->cartGraphicId = 0;
	if (w->actionState == FigureActionState_149_Corpse) {
		w->state = FigureState_Dead;
	}
	w->waitTicks++;
	if (w->waitTicks > 200) {
		w->state = FigureState_Dead;
		w->graphicOffset = 0;
	}
	if (w->actionState == FigureActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Figure_Criminal) +
			WalkerActionCorpseGraphicOffset(w) + 96;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Figure_Criminal) +
			criminalOffsets[w->graphicOffset / 2] + 104;
	}
}

void WalkerAction_rioter(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	Data_CityInfo.numRiotersInCity++;
	if (!w->targetedByWalkerId) {
		Data_CityInfo.riotersOrAttackingNativesInCity = 10;
	}
	w->terrainUsage = FigureTerrainUsage_Enemy;
	w->maxRoamLength = 480;
	w->cartGraphicId = 0;
	w->isGhost = 0;
	switch (w->actionState) {
		case FigureActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_120_RioterCreated:
			WalkerActionIncreaseGraphicOffset(w, 32);
			w->waitTicks++;
			if (w->waitTicks >= 160) {
				w->actionState = FigureActionState_121_RioterMoving;
				int xTile, yTile;
				int buildingId = Formation_Rioter_getTargetBuilding(&xTile, &yTile);
				if (buildingId) {
					w->destinationX = xTile;
					w->destinationY = yTile;
					w->destinationBuildingId = buildingId;
					FigureRoute_remove(walkerId);
				} else {
					w->state = FigureState_Dead;
				}
			}
			break;
		case FigureActionState_121_RioterMoving:
			WalkerActionIncreaseGraphicOffset(w, 12);
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == DirFigure_8_AtDestination) {
				int xTile, yTile;
				int buildingId = Formation_Rioter_getTargetBuilding(&xTile, &yTile);
				if (buildingId) {
					w->destinationX = xTile;
					w->destinationY = yTile;
					w->destinationBuildingId = buildingId;
					FigureRoute_remove(walkerId);
				} else {
					w->state = FigureState_Dead;
				}
			} else if (w->direction == DirFigure_9_Reroute || w->direction == DirFigure_10_Lost) {
				w->actionState = FigureActionState_120_RioterCreated;
				FigureRoute_remove(walkerId);
			} else if (w->direction == DirFigure_11_Attack) {
				if (w->graphicOffset > 12) {
					w->graphicOffset = 0;
				}
			}
			break;
	}
	int dir;
	if (w->direction == DirFigure_11_Attack) {
		dir = w->attackDirection;
	} else if (w->direction < 8) {
		dir = w->direction;
	} else {
		dir = w->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);
	
	if (w->actionState == FigureActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Figure_Criminal) +
			96 + WalkerActionCorpseGraphicOffset(w);
	} else if (w->direction == DirFigure_11_Attack) {
		w->graphicId = GraphicId(ID_Graphic_Figure_Criminal) +
			104 + criminalOffsets[w->graphicOffset];
	} else if (w->actionState == FigureActionState_121_RioterMoving) {
		w->graphicId = GraphicId(ID_Graphic_Figure_Criminal) +
			dir + 8 * w->graphicOffset;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Figure_Criminal) +
			104 + criminalOffsets[w->graphicOffset / 2];
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
			case Building_Fort:
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
		PlayerMessage_post(0, Message_14_DestroyedBuilding, b->type, w->gridOffset);
		Data_Message.messageCategoryCount[MessageDelay_RiotCollapse]++;
		Building_collapseOnFire(buildingId, 0);
		w->actionState = FigureActionState_120_RioterCreated;
		w->waitTicks = 0;
		w->direction = dir;
		return 1;
	}
	return 0;
}
