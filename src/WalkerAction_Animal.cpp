#include "WalkerAction_private.h"

#include "Walker.h"

#include "Data/CityInfo.h"
#include "Data/Formation.h"

static const int seagullOffsetsX[] = {0, 0, -2, 1, 2, -3, 4, -1};
static const int seagullOffsetsY[] = {0, -2, 0, 2, 0, 1, -3, 4};

static const int herdOffsetsX[] = {0, 2, -1, 1, 1, -1, 3, -2, 0, -4, -1, 0, 1, 4, 2, -5};
static const int herdOffsetsY[] = {0, 1, -1, 1, 0, 1, 1, -1, 2, 0, 3, 5, 4, 0, 3, 2};

static const int sheepGraphicOffsets[] = {
	0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 4, 4, 5, 5, -1, -1, -1, -1, -1, -1, -1, -1,
	0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 5, 5
};

void WalkerAction_seagulls(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = 0;
	w->isGhost = 0;
	w->useCrossCountry = 1;
	if (!(w->graphicOffset & 3) && WalkerMovement_crossCountryWalkTicks(walkerId, 1)) {
		w->progressOnTile++;
		if (w->progressOnTile > 8) {
			w->progressOnTile = 0;
		}
		WalkerAction_Common_setCrossCountryDestination(walkerId, w,
			seagullOffsetsX[(int)w->progressOnTile], seagullOffsetsY[(int)w->progressOnTile]);
	}
	if (walkerId & 1) {
		WalkerActionIncreaseGraphicOffset(w, 54);
		w->graphicId = GraphicId(ID_Graphic_Walker_Seagulls) + w->graphicOffset / 3;
	} else {
		WalkerActionIncreaseGraphicOffset(w, 72);
		w->graphicId = GraphicId(ID_Graphic_Walker_Seagulls) + 18 + w->graphicOffset / 3;
	}
}

void WalkerAction_sheep(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[walkerId];
	w->terrainUsage = WalkerTerrainUsage_Animal;
	w->useCrossCountry = 0;
	w->isGhost = 0;
	Data_CityInfo.numAnimalsInCity++;
	WalkerActionIncreaseGraphicOffset(w, 6);

	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_196_HerdAnimalAtRest:
			w->waitTicks++;
			if (w->waitTicks > 400) {
				w->waitTicks = walkerId & 0x1f;
				w->actionState = WalkerActionState_197_HerdAnimalMoving;
				w->destinationX = f->destinationX + herdOffsetsX[w->indexInFormation];
				w->destinationY = f->destinationY + herdOffsetsY[w->indexInFormation];
				w->roamLength = 0;
			}
			break;
		case WalkerActionState_197_HerdAnimalMoving:
			WalkerMovement_walkTicks(walkerId, 1);
			if (w->direction == 8 || w->direction == 10) {
				w->direction = w->previousTileDirection;
				w->actionState = WalkerActionState_196_HerdAnimalAtRest;
				w->waitTicks = walkerId & 0x1f;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			}
			break;
	}
	int dir = WalkerActionDirection(w);
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Sheep) + 104 +
			WalkerActionCorpseGraphicOffset(w);
	} else if (w->actionState == WalkerActionState_196_HerdAnimalAtRest) {
		if (walkerId & 3) {
			w->graphicId = GraphicId(ID_Graphic_Walker_Sheep) + 48 + dir +
				8 * sheepGraphicOffsets[w->waitTicks & 0x3f];
		} else {
			w->graphicId = GraphicId(ID_Graphic_Walker_Sheep) + 96 + dir;
		}
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Sheep) + dir + 8 * w->graphicOffset;
	}
}

void WalkerAction_wolf(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[walkerId];
	w->terrainUsage = WalkerTerrainUsage_Animal;
	w->useCrossCountry = 0;
	w->isGhost = 0;
	Data_CityInfo.numAnimalsInCity++;
	WalkerActionIncreaseGraphicOffset(w, 12);

	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_196_HerdAnimalAtRest:
			w->waitTicks++;
			if (w->waitTicks > 400) {
				w->waitTicks = walkerId & 0x1f;
				w->actionState = WalkerActionState_197_HerdAnimalMoving;
				w->destinationX = f->destinationX + herdOffsetsX[w->indexInFormation];
				w->destinationY = f->destinationY + herdOffsetsY[w->indexInFormation];
				w->roamLength = 0;
			}
			break;
		case WalkerActionState_197_HerdAnimalMoving:
			WalkerMovement_walkTicks(walkerId, 2);
			if (w->direction == 8 || w->direction == 10) {
				w->direction = w->previousTileDirection;
				w->actionState = WalkerActionState_196_HerdAnimalAtRest;
				w->waitTicks = walkerId & 0x1f;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			}
			break;
		case WalkerActionState_199_WolfAttacking:
			WalkerMovement_walkTicks(walkerId, 2);
			if (w->direction == 8) {
				int targetId = WalkerEnemy_wolfGetTargetWalker(w->x, w->y, 6);
				if (targetId) {
					w->destinationX = Data_Walkers[targetId].x;
					w->destinationY = Data_Walkers[targetId].y;
					w->targetWalkerId = targetId;
					Data_Walkers[targetId].targetedByWalkerId = walkerId;
					w->targetWalkerCreatedSequence = Data_Walkers[targetId].createdSequence;
					WalkerRoute_remove(walkerId);
				} else {
					w->direction = w->previousTileDirection;
					w->actionState = WalkerActionState_196_HerdAnimalAtRest;
					w->waitTicks = walkerId & 0x1f;
				}
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			} else if (w->direction == 10) {
				w->direction = w->previousTileDirection;
				w->actionState = WalkerActionState_196_HerdAnimalAtRest;
				w->waitTicks = walkerId & 0x1f;
			}
	}
	int dir = WalkerActionDirection(w);
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Wolf) + 96 +
			WalkerActionCorpseGraphicOffset(w);
	} else if (w->actionState == WalkerActionState_150_Attack) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Wolf) + 104 +
			dir + 8 * (w->attackGraphicOffset / 4);
	} else if (w->actionState == WalkerActionState_196_HerdAnimalAtRest) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Wolf) + 152 + dir;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Wolf) + dir + 8 * w->graphicOffset;
	}
}

void WalkerAction_zebra(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	struct Data_Formation *f = &Data_Formations[walkerId];
	w->terrainUsage = WalkerTerrainUsage_Animal;
	w->useCrossCountry = 0;
	w->isGhost = 0;
	Data_CityInfo.numAnimalsInCity++;
	WalkerActionIncreaseGraphicOffset(w, 12);

	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_196_HerdAnimalAtRest:
			w->waitTicks++;
			if (w->waitTicks > 200) {
				w->waitTicks = walkerId & 0x1f;
				w->actionState = WalkerActionState_197_HerdAnimalMoving;
				w->destinationX = f->destinationX + herdOffsetsX[w->indexInFormation];
				w->destinationY = f->destinationY + herdOffsetsY[w->indexInFormation];
				w->roamLength = 0;
			}
			break;
		case WalkerActionState_197_HerdAnimalMoving:
			WalkerMovement_walkTicks(walkerId, 2);
			if (w->direction == 8 || w->direction == 10) {
				w->direction = w->previousTileDirection;
				w->actionState = WalkerActionState_196_HerdAnimalAtRest;
				w->waitTicks = walkerId & 0x1f;
			} else if (w->direction == 9) {
				WalkerRoute_remove(walkerId);
			}
			break;
	}
	int dir = WalkerActionDirection(w);
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Zebra) + 96 +
			WalkerActionCorpseGraphicOffset(w);
	} else if (w->actionState == WalkerActionState_196_HerdAnimalAtRest) {
		w->graphicId = GraphicId(ID_Graphic_Walker_Zebra) + dir;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_Zebra) + dir + 8 * w->graphicOffset;
	}
}
