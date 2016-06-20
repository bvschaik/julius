#include "FigureAction_private.h"

#include "Building.h"
#include "Figure.h"
#include "Terrain.h"

static void FigureAction_cultureCommon(int walkerId, int numTicks)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_125_Roaming:
			f->isGhost = 0;
			f->roamLength++;
			if (f->roamLength >= f->maxRoamLength) {
				int x, y;
				if (Terrain_getClosestRoadWithinRadius(
						Data_Buildings[f->buildingId].x,
						Data_Buildings[f->buildingId].y,
						Data_Buildings[f->buildingId].size,
						2, &x, &y)) {
					f->actionState = FigureActionState_126_RoamerReturning;
					f->destinationX = x;
					f->destinationY = y;
					FigureRoute_remove(walkerId);
					f->roamLength = 0;
				} else {
					f->state = FigureState_Dead;
				}
			}
			FigureMovement_roamTicks(walkerId, numTicks);
			break;
		case FigureActionState_126_RoamerReturning:
			FigureMovement_walkTicks(walkerId, numTicks);
			if (f->direction == DirFigure_8_AtDestination ||
				f->direction == DirFigure_9_Reroute || f->direction == DirFigure_10_Lost) {
				f->state = FigureState_Dead;
			}
			break;
	}
}

static void FigureAction_culture(int walkerId, int graphicCategory)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->terrainUsage = FigureTerrainUsage_Roads;
	f->useCrossCountry = 0;
	f->maxRoamLength = 384;
	int buildingId = f->buildingId;
	if (!BuildingIsInUse(buildingId) || Data_Buildings[buildingId].walkerId != walkerId) {
		f->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(f, 12);
	FigureAction_cultureCommon(walkerId, 1);
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = GraphicId(graphicCategory) +
			WalkerActionCorpseGraphicOffset(f) + 96;
	} else {
		f->graphicId = GraphicId(graphicCategory) +
			WalkerActionDirection(f) + 8 * f->graphicOffset;
	}
}

void FigureAction_priest(int walkerId)
{
	FigureAction_culture(walkerId, ID_Graphic_Figure_Priest);
}

void FigureAction_schoolChild(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->terrainUsage = FigureTerrainUsage_Roads;
	f->useCrossCountry = 0;
	f->maxRoamLength = 96;
	int buildingId = f->buildingId;
	if (!BuildingIsInUse(buildingId) || Data_Buildings[buildingId].type != Building_School) {
		f->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(f, 12);
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(walkerId);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(walkerId);
			break;
		case FigureActionState_125_Roaming:
			f->isGhost = 0;
			f->roamLength++;
			if (f->roamLength >= f->maxRoamLength) {
				f->state = FigureState_Dead;
			}
			FigureMovement_roamTicks(walkerId, 2);
			break;
	}
	if (f->actionState == FigureActionState_149_Corpse) {
		f->graphicId = GraphicId(ID_Graphic_Figure_SchoolChild) +
			WalkerActionCorpseGraphicOffset(f) + 96;
	} else {
		f->graphicId = GraphicId(ID_Graphic_Figure_SchoolChild) +
			WalkerActionDirection(f) + 8 * f->graphicOffset;
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
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->terrainUsage = FigureTerrainUsage_Roads;
	f->useCrossCountry = 0;
	f->maxRoamLength = 192;
	int buildingId = f->buildingId;
	if (!BuildingIsInUse(buildingId) || Data_Buildings[buildingId].walkerId != walkerId) {
		f->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(f, 12);
	FigureAction_cultureCommon(walkerId, 1);
	WalkerActionUpdateGraphic(f, GraphicId(ID_Graphic_Figure_Missionary));
}

void FigureAction_patrician(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->terrainUsage = FigureTerrainUsage_Roads;
	f->useCrossCountry = 0;
	f->maxRoamLength = 128;
	if (!BuildingIsInUse(f->buildingId)) {
		f->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(f, 12);
	FigureAction_cultureCommon(walkerId, 1);
	WalkerActionUpdateGraphic(f, GraphicId(ID_Graphic_Figure_Patrician));
}

void FigureAction_laborSeeker(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->terrainUsage = FigureTerrainUsage_Roads;
	f->useCrossCountry = 0;
	f->maxRoamLength = 384;
	int buildingId = f->buildingId;
	if (!BuildingIsInUse(buildingId) || Data_Buildings[buildingId].walkerId2 != walkerId) {
		f->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(f, 12);
	FigureAction_cultureCommon(walkerId, 1);
	WalkerActionUpdateGraphic(f, GraphicId(ID_Graphic_Figure_LaborSeeker));
}

void FigureAction_marketTrader(int walkerId)
{
	struct Data_Walker *f = &Data_Walkers[walkerId];
	f->terrainUsage = FigureTerrainUsage_Roads;
	f->useCrossCountry = 0;
	f->maxRoamLength = 384;
	
	if (!BuildingIsInUse(f->buildingId) || Data_Buildings[f->buildingId].walkerId != walkerId) {
		f->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(f, 12);
	if (f->actionState == FigureActionState_125_Roaming) {
		// force return on out of stock
		int stock = Building_Market_getMaxFoodStock(f->buildingId) +
			Building_Market_getMaxGoodsStock(f->buildingId);
		if (f->roamLength >= 96 && stock <= 0) {
			f->roamLength = f->maxRoamLength;
		}
	}
	FigureAction_cultureCommon(walkerId, 1);
	WalkerActionUpdateGraphic(f, GraphicId(ID_Graphic_Figure_MarketLady));
}

