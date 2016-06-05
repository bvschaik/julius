#include "WalkerAction_private.h"

#include "Building.h"
#include "Terrain.h"
#include "Walker.h"

static void WalkerAction_cultureCommon(int walkerId, int numTicks)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	switch (w->actionState) {
		case FigureActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
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
			WalkerMovement_roamTicks(walkerId, numTicks);
			break;
		case FigureActionState_126_RoamerReturning:
			WalkerMovement_walkTicks(walkerId, numTicks);
			if (w->direction == DirFigure_8_AtDestination ||
				w->direction == DirFigure_9_Reroute || w->direction == DirFigure_10_Lost) {
				w->state = FigureState_Dead;
			}
			break;
	}
}

static void WalkerAction_culture(int walkerId, int graphicCategory)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = FigureTerrainUsage_Roads;
	w->useCrossCountry = 0;
	w->maxRoamLength = 384;
	int buildingId = w->buildingId;
	if (!BuildingIsInUse(buildingId) || Data_Buildings[buildingId].walkerId != walkerId) {
		w->state = FigureState_Dead;
	}
	WalkerActionIncreaseGraphicOffset(w, 12);
	WalkerAction_cultureCommon(walkerId, 1);
	if (w->actionState == FigureActionState_149_Corpse) {
		w->graphicId = GraphicId(graphicCategory) +
			WalkerActionCorpseGraphicOffset(w) + 96;
	} else {
		w->graphicId = GraphicId(graphicCategory) +
			WalkerActionDirection(w) + 8 * w->graphicOffset;
	}
}

void WalkerAction_priest(int walkerId)
{
	WalkerAction_culture(walkerId, ID_Graphic_Figure_Priest);
}

void WalkerAction_schoolChild(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = FigureTerrainUsage_Roads;
	w->useCrossCountry = 0;
	w->maxRoamLength = 96;
	int buildingId = w->buildingId;
	if (!BuildingIsInUse(buildingId) || Data_Buildings[buildingId].type != Building_School) {
		w->state = FigureState_Dead;
	}
	WalkerActionIncreaseGraphicOffset(w, 12);
	switch (w->actionState) {
		case FigureActionState_150_Attack:
			WalkerAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			WalkerAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_125_Roaming:
			w->isGhost = 0;
			w->roamLength++;
			if (w->roamLength >= w->maxRoamLength) {
				w->state = FigureState_Dead;
			}
			WalkerMovement_roamTicks(walkerId, 2);
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

void WalkerAction_teacher(int walkerId)
{
	WalkerAction_culture(walkerId, ID_Graphic_Figure_TeacherLibrarian);
}

void WalkerAction_librarian(int walkerId)
{
	WalkerAction_culture(walkerId, ID_Graphic_Figure_TeacherLibrarian);
}

void WalkerAction_barber(int walkerId)
{
	WalkerAction_culture(walkerId, ID_Graphic_Figure_Barber);
}

void WalkerAction_bathhouseWorker(int walkerId)
{
	WalkerAction_culture(walkerId, ID_Graphic_Figure_BathhouseWorker);
}

void WalkerAction_doctor(int walkerId)
{
	WalkerAction_culture(walkerId, ID_Graphic_Figure_DoctorSurgeon);
}

void WalkerAction_surgeon(int walkerId)
{
	WalkerAction_culture(walkerId, ID_Graphic_Figure_DoctorSurgeon);
}

void WalkerAction_missionary(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = FigureTerrainUsage_Roads;
	w->useCrossCountry = 0;
	w->maxRoamLength = 192;
	int buildingId = w->buildingId;
	if (!BuildingIsInUse(buildingId) || Data_Buildings[buildingId].walkerId != walkerId) {
		w->state = FigureState_Dead;
	}
	WalkerActionIncreaseGraphicOffset(w, 12);
	WalkerAction_cultureCommon(walkerId, 1);
	WalkerActionUpdateGraphic(w, GraphicId(ID_Graphic_Figure_Missionary));
}

void WalkerAction_patrician(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = FigureTerrainUsage_Roads;
	w->useCrossCountry = 0;
	w->maxRoamLength = 128;
	if (!BuildingIsInUse(w->buildingId)) {
		w->state = FigureState_Dead;
	}
	WalkerActionIncreaseGraphicOffset(w, 12);
	WalkerAction_cultureCommon(walkerId, 1);
	WalkerActionUpdateGraphic(w, GraphicId(ID_Graphic_Figure_Patrician));
}

void WalkerAction_laborSeeker(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = FigureTerrainUsage_Roads;
	w->useCrossCountry = 0;
	w->maxRoamLength = 384;
	int buildingId = w->buildingId;
	if (!BuildingIsInUse(buildingId) || Data_Buildings[buildingId].walkerId2 != walkerId) {
		w->state = FigureState_Dead;
	}
	WalkerActionIncreaseGraphicOffset(w, 12);
	WalkerAction_cultureCommon(walkerId, 1);
	WalkerActionUpdateGraphic(w, GraphicId(ID_Graphic_Figure_LaborSeeker));
}

void WalkerAction_marketTrader(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->terrainUsage = FigureTerrainUsage_Roads;
	w->useCrossCountry = 0;
	w->maxRoamLength = 384;
	
	if (!BuildingIsInUse(w->buildingId) || Data_Buildings[w->buildingId].walkerId != walkerId) {
		w->state = FigureState_Dead;
	}
	WalkerActionIncreaseGraphicOffset(w, 12);
	if (w->actionState == FigureActionState_125_Roaming) {
		// force return on out of stock
		int stock = Building_Market_getMaxFoodStock(w->buildingId) +
			Building_Market_getMaxGoodsStock(w->buildingId);
		if (w->roamLength >= 96 && stock <= 0) {
			w->roamLength = w->maxRoamLength;
		}
	}
	WalkerAction_cultureCommon(walkerId, 1);
	WalkerActionUpdateGraphic(w, GraphicId(ID_Graphic_Figure_MarketLady));
}

