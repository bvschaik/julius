#include "WalkerAction_private.h"

#include "Terrain.h"
#include "Walker.h"

static void WalkerAction_cultureCommon(int walkerId, int numTicks)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_125_Roaming:
			w->isGhost = 0;
			w->roamLength++;
			if (w->roamLength >= w->maxRoamLength) {
				int x, y;
				if (Terrain_getClosestRoadWithinRadius(
						Data_Buildings[w->buildingId].x,
						Data_Buildings[w->buildingId].y,
						Data_Buildings[w->buildingId].size,
						2, &x, &y)) {
					w->actionState = WalkerActionState_126_RoamerReturning;
					w->destinationX = x;
					w->destinationY = y;
					WalkerRoute_remove(walkerId);
					w->roamLength = 0;
				} else {
					w->state = WalkerState_Dead;
				}
			}
			WalkerMovement_roamTicks(walkerId, numTicks);
			break;
		case WalkerActionState_126_RoamerReturning:
			WalkerMovement_walkTicks(walkerId, numTicks);
			if (w->direction == 8 || w->direction == 9 || w->direction == 10) {
				w->state = WalkerState_Dead;
			}
			break;
	}
}

static void WalkerAction_culture(int walkerId, int graphicCategory)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = 1;
	w->__unknown_0c = 0;
	w->maxRoamLength = 384;
	int buildingId = w->buildingId;
	if (Data_Buildings[buildingId].inUse != 1 || Data_Buildings[buildingId].walkerId != walkerId) {
		w->state = WalkerState_Dead;
	}
	WalkerActionIncreaseGraphicOffset(w, 12);
	WalkerAction_cultureCommon(walkerId, 1);
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(graphicCategory) +
			WalkerActionCorpseGraphicOffset(w) + 96;
	} else {
		w->graphicId = GraphicId(graphicCategory) +
			WalkerActionDirection(w) + 8 * w->graphicOffset;
	}
}

void WalkerAction_priest(int walkerId)
{
	WalkerAction_culture(walkerId, ID_Graphic_Walker_Priest);
}

void WalkerAction_schoolChild(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = 1;
	w->__unknown_0c = 0;
	w->maxRoamLength = 96;
	int buildingId = w->buildingId;
	if (Data_Buildings[buildingId].inUse != 1 || Data_Buildings[buildingId].walkerId != walkerId) {
		w->state = WalkerState_Dead;
	}
	WalkerActionIncreaseGraphicOffset(w, 12);
	switch (w->actionState) {
		case WalkerActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case WalkerActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case WalkerActionState_125_Roaming:
			w->isGhost = 0;
			w->roamLength++;
			if (w->roamLength >= w->maxRoamLength) {
				w->state = WalkerState_Dead;
			}
			WalkerMovement_roamTicks(walkerId, 2);
			break;
	}
	if (w->actionState == WalkerActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Walker_SchoolChild) +
			WalkerActionCorpseGraphicOffset(w) + 96;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Walker_SchoolChild) +
			WalkerActionDirection(w) + 8 * w->graphicOffset;
	}
}

void WalkerAction_teacher(int walkerId)
{
	WalkerAction_culture(walkerId, ID_Graphic_Walker_TeacherLibrarian);
}

void WalkerAction_librarian(int walkerId)
{
	WalkerAction_culture(walkerId, ID_Graphic_Walker_TeacherLibrarian);
}

void WalkerAction_barber(int walkerId)
{
	WalkerAction_culture(walkerId, ID_Graphic_Walker_Barber);
}

void WalkerAction_bathhouseWorker(int walkerId)
{
	WalkerAction_culture(walkerId, ID_Graphic_Walker_BathhouseWorker);
}

void WalkerAction_doctor(int walkerId)
{
	WalkerAction_culture(walkerId, ID_Graphic_Walker_DoctorSurgeon);
}

void WalkerAction_surgeon(int walkerId)
{
	WalkerAction_culture(walkerId, ID_Graphic_Walker_DoctorSurgeon);
}
