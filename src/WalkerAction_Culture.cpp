#include "FigureAction_private.h"

#include "Building.h"
#include "Terrain.h"
#include "Walker.h"

static void FigureAction_cultureCommon(int walkerId, int numTicks)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	switch (w->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_125_Roaming:
			w->isGhost = 0;
			w->roamLength++;
			if (w->roamLength >= w->maxRoamLength) {
				int x, y;
				if (Terrain_getClosestRoadWithinRadius(
						Data_Buildings[w->buildingId].x,
						Data_Buildings[w->buildingId].y,
						Data_Buildings[w->buildingId].size,
						2, &x, &y)) {
					w->actionState = FigureActionState_126_RoamerReturning;
					w->destinationX = x;
					w->destinationY = y;
					FigureRoute_remove(walkerId);
					w->roamLength = 0;
				} else {
					w->state = FigureState_Dead;
				}
			}
			FigureMovement_roamTicks(walkerId, numTicks);
			break;
		case FigureActionState_126_RoamerReturning:
			FigureMovement_walkTicks(walkerId, numTicks);
			if (w->direction == DirFigure_8_AtDestination ||
				w->direction == DirFigure_9_Reroute || w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
	}
}

static void FigureAction_culture(int walkerId, int graphicCategory)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = FigureTerrainUsage_Roads;
	w->useCrossCountry = 0;
	w->maxRoamLength = 384;
	int buildingId = w->buildingId;
	if (!BuildingIsInUse(buildingId) || Data_Buildings[buildingId].walkerId != walkerId) {
		w->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(w, 12);
	FigureAction_cultureCommon(walkerId, 1);
	if (w->actionState == FigureActionState_149_Corpse) {
		w->graphicId = GraphicId(graphicCategory) +
			WalkerActionCorpseGraphicOffset(w) + 96;
	} else {
		w->graphicId = GraphicId(graphicCategory) +
			WalkerActionDirection(w) + 8 * w->graphicOffset;
	}
}

void FigureAction_priest(int walkerId)
{
	FigureAction_culture(walkerId, ID_Graphic_Figure_Priest);
}

void FigureAction_schoolChild(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = FigureTerrainUsage_Roads;
	w->useCrossCountry = 0;
	w->maxRoamLength = 96;
	int buildingId = w->buildingId;
	if (!BuildingIsInUse(buildingId) || Data_Buildings[buildingId].type != Building_School) {
		w->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(w, 12);
	switch (w->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_125_Roaming:
			w->isGhost = 0;
			w->roamLength++;
			if (w->roamLength >= w->maxRoamLength) {
				w->state = FigureState_Dead;
			}
			FigureMovement_roamTicks(walkerId, 2);
			break;
	}
	if (w->actionState == FigureActionState_149_Corpse) {
		w->graphicId = GraphicId(ID_Graphic_Figure_SchoolChild) +
			WalkerActionCorpseGraphicOffset(w) + 96;
	} else {
		w->graphicId = GraphicId(ID_Graphic_Figure_SchoolChild) +
			WalkerActionDirection(w) + 8 * w->graphicOffset;
	}
}

void FigureAction_teacher(int walkerId)
{
	FigureAction_culture(walkerId, ID_Graphic_Figure_TeacherLibrarian);
}

void FigureAction_librarian(int walkerId)
{
	FigureAction_culture(walkerId, ID_Graphic_Figure_TeacherLibrarian);
}

void FigureAction_barber(int walkerId)
{
	FigureAction_culture(walkerId, ID_Graphic_Figure_Barber);
}

void FigureAction_bathhouseWorker(int walkerId)
{
	FigureAction_culture(walkerId, ID_Graphic_Figure_BathhouseWorker);
}

void FigureAction_doctor(int walkerId)
{
	FigureAction_culture(walkerId, ID_Graphic_Figure_DoctorSurgeon);
}

void FigureAction_surgeon(int walkerId)
{
	FigureAction_culture(walkerId, ID_Graphic_Figure_DoctorSurgeon);
}

void FigureAction_missionary(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = FigureTerrainUsage_Roads;
	w->useCrossCountry = 0;
	w->maxRoamLength = 192;
	int buildingId = w->buildingId;
	if (!BuildingIsInUse(buildingId) || Data_Buildings[buildingId].walkerId != walkerId) {
		w->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(w, 12);
	FigureAction_cultureCommon(walkerId, 1);
	WalkerActionUpdateGraphic(w, GraphicId(ID_Graphic_Figure_Missionary));
}

void FigureAction_patrician(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = FigureTerrainUsage_Roads;
	w->useCrossCountry = 0;
	w->maxRoamLength = 128;
	if (!BuildingIsInUse(w->buildingId)) {
		w->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(w, 12);
	FigureAction_cultureCommon(walkerId, 1);
	WalkerActionUpdateGraphic(w, GraphicId(ID_Graphic_Figure_Patrician));
}

void FigureAction_laborSeeker(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = FigureTerrainUsage_Roads;
	w->useCrossCountry = 0;
	w->maxRoamLength = 384;
	int buildingId = w->buildingId;
	if (!BuildingIsInUse(buildingId) || Data_Buildings[buildingId].walkerId2 != walkerId) {
		w->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(w, 12);
	FigureAction_cultureCommon(walkerId, 1);
	WalkerActionUpdateGraphic(w, GraphicId(ID_Graphic_Figure_LaborSeeker));
}

void FigureAction_marketTrader(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = FigureTerrainUsage_Roads;
	w->useCrossCountry = 0;
	w->maxRoamLength = 384;
	
	if (!BuildingIsInUse(w->buildingId) || Data_Buildings[w->buildingId].walkerId != walkerId) {
		w->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(w, 12);
	if (w->actionState == FigureActionState_125_Roaming) {
		// force return on out of stock
		int stock = Building_Market_getMaxFoodStock(w->buildingId) +
			Building_Market_getMaxGoodsStock(w->buildingId);
		if (w->roamLength >= 96 && stock <= 0) {
			w->roamLength = w->maxRoamLength;
		}
	}
	FigureAction_cultureCommon(walkerId, 1);
	WalkerActionUpdateGraphic(w, GraphicId(ID_Graphic_Figure_MarketLady));
}

