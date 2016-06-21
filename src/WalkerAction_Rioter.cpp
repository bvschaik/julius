#include "FigureAction_private.h"

#include "Building.h"
#include "Figure.h"
#include "FigureMovement.h"
#include "Formation.h"
#include "PlayerMessage.h"
#include "Time.h"

#include "Data/CityInfo.h"
#include "Data/Grid.h"
#include "Data/Message.h"

static const int criminalOffsets[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1
};

void FigureAction_protestor(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->terrainUsage = FigureTerrainUsage_Roads;
	FigureActionIncreaseGraphicOffset(f, 64);
	f->cartGraphicId = 0;
	if (f->actionState == FigureActionState_149_Corpse) {
		f->state = FigureState_Dead;
	}
	f->waitTicks++;
	if (f->waitTicks > 200) {
		f->state = FigureState_Dead;
		f->graphicOffset = 0;
	}
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = GraphicId(ID_Graphic_Figure_Criminal) +
			WalkerActionCorpseGraphicOffset(f) + 96;
	} else {
		f->graphicId = GraphicId(ID_Graphic_Figure_Criminal) +
			criminalOffsets[f->graphicOffset / 4] + 104;
	}
}

void FigureAction_criminal(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->terrainUsage = FigureTerrainUsage_Roads;
	FigureActionIncreaseGraphicOffset(f, 32);
	f->cartGraphicId = 0;
	if (f->actionState == FigureActionState_149_Corpse) {
		f->state = FigureState_Dead;
	}
	f->waitTicks++;
	if (f->waitTicks > 200) {
		f->state = FigureState_Dead;
		f->graphicOffset = 0;
	}
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = GraphicId(ID_Graphic_Figure_Criminal) +
			WalkerActionCorpseGraphicOffset(f) + 96;
	} else {
		f->graphicId = GraphicId(ID_Graphic_Figure_Criminal) +
			criminalOffsets[f->graphicOffset / 2] + 104;
	}
}

void FigureAction_rioter(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	Data_CityInfo.numRiotersInCity++;
	if (!f->targetedByFigureId) {
		Data_CityInfo.riotersOrAttackingNativesInCity = 10;
	}
	f->terrainUsage = FigureTerrainUsage_Enemy;
	f->maxRoamLength = 480;
	f->cartGraphicId = 0;
	f->isGhost = 0;
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_120_RioterCreated:
			FigureActionIncreaseGraphicOffset(f, 32);
			f->waitTicks++;
			if (f->waitTicks >= 160) {
				f->actionState = FigureActionState_121_RioterMoving;
				int xTile, yTile;
				int buildingId = Formation_Rioter_getTargetBuilding(&xTile, &yTile);
				if (buildingId) {
					f->destinationX = xTile;
					f->destinationY = yTile;
					f->destinationBuildingId = buildingId;
					FigureRoute_remove(walkerId);
				} else {
					f->state = FigureState_Dead;
				}
			}
			break;
		case FigureActionState_121_RioterMoving:
			FigureActionIncreaseGraphicOffset(f, 12);
			FigureMovement_walkTicks(walkerId, 1);
			if (f->direction == DirFigure_8_AtDestination) {
				int xTile, yTile;
				int buildingId = Formation_Rioter_getTargetBuilding(&xTile, &yTile);
				if (buildingId) {
					f->destinationX = xTile;
					f->destinationY = yTile;
					f->destinationBuildingId = buildingId;
					FigureRoute_remove(walkerId);
				} else {
					f->state = FigureState_Dead;
				}
			} else if (f->direction == DirFigure_9_Reroute || f->direction == DirFigure_10_Lost) {
				f->actionState = FigureActionState_120_RioterCreated;
				FigureRoute_remove(walkerId);
			} else if (f->direction == DirFigure_11_Attack) {
				if (f->graphicOffset > 12) {
					f->graphicOffset = 0;
				}
			}
			break;
	}
	int dir;
	if (f->direction == DirFigure_11_Attack) {
		dir = f->attackDirection;
	} else if (f->direction < 8) {
		dir = f->direction;
	} else {
		dir = f->previousTileDirection;
	}
	WalkerActionNormalizeDirection(dir);
	
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = GraphicId(ID_Graphic_Figure_Criminal) +
			96 + WalkerActionCorpseGraphicOffset(f);
	} else if (f->direction == DirFigure_11_Attack) {
		f->graphicId = GraphicId(ID_Graphic_Figure_Criminal) +
			104 + criminalOffsets[f->graphicOffset];
	} else if (f->actionState == FigureActionState_121_RioterMoving) {
		f->graphicId = GraphicId(ID_Graphic_Figure_Criminal) +
			dir + 8 * f->graphicOffset;
	} else {
		f->graphicId = GraphicId(ID_Graphic_Figure_Criminal) +
			104 + criminalOffsets[f->graphicOffset / 2];
	}
}

int FigureAction_Rioter_collapseBuilding(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	for (int dir = 0; dir < 8; dir += 2) {
		int gridOffset = f->gridOffset + Constant_DirectionGridOffsets[dir];
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
		PlayerMessage_post(0, Message_14_DestroyedBuilding, b->type, f->gridOffset);
		Data_Message.messageCategoryCount[MessageDelay_RiotCollapse]++;
		Building_collapseOnFire(buildingId, 0);
		f->actionState = FigureActionState_120_RioterCreated;
		f->waitTicks = 0;
		f->direction = dir;
		return 1;
	}
	return 0;
}
