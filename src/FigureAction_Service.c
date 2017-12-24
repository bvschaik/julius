#include "FigureAction_private.h"

#include "Security.h"
#include "Terrain.h"

#include "Data/CityInfo.h"

#include "building/building.h"
#include "building/list.h"
#include "building/maintenance.h"
#include "core/calc.h"
#include "figure/enemy_army.h"
#include "figure/route.h"
#include "figure/type.h"
#include "map/building.h"
#include "sound/effect.h"

void FigureAction_taxCollector(figure *f)
{
	building *b = building_get(f->buildingId);
	
	f->terrainUsage = FigureTerrainUsage_Roads;
	f->useCrossCountry = 0;
	f->maxRoamLength = 512;
	if (!BuildingIsInUse(b) || b->figureId != f->id) {
		f->state = FigureState_Dead;
	}
	FigureActionIncreaseGraphicOffset(f, 12);
	
	switch (f->actionState) {
		case FigureActionState_150_Attack:
			FigureAction_Common_handleAttack(f);
			break;
		case FigureActionState_149_Corpse:
			FigureAction_Common_handleCorpse(f);
			break;
		case FigureActionState_40_TaxCollectorCreated:
			f->isGhost = 1;
			f->graphicOffset = 0;
			f->waitTicks--;
			if (f->waitTicks <= 0) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					f->actionState = FigureActionState_41_TaxCollectorEnteringExiting;
					FigureAction_Common_setCrossCountryDestination(f, xRoad, yRoad);
					f->roamLength = 0;
				} else {
					f->state = FigureState_Dead;
				}
			}
			break;
		case FigureActionState_41_TaxCollectorEnteringExiting:
			f->useCrossCountry = 1;
			f->isGhost = 1;
			if (FigureMovement_crossCountryWalkTicks(f, 1) == 1) {
				if (map_building_at(f->gridOffset) == f->buildingId) {
					// returned to own building
					f->state = FigureState_Dead;
				} else {
					f->actionState = FigureActionState_42_TaxCollectorRoaming;
					FigureMovement_initRoaming(f);
					f->roamLength = 0;
				}
			}
			break;
		case FigureActionState_42_TaxCollectorRoaming:
			f->isGhost = 0;
			f->roamLength++;
			if (f->roamLength >= f->maxRoamLength) {
				int xRoad, yRoad;
				if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
					f->actionState = FigureActionState_43_TaxCollectorReturning;
					f->destinationX = xRoad;
					f->destinationY = yRoad;
				} else {
					f->state = FigureState_Dead;
				}
			}
			FigureMovement_roamTicks(f, 1);
			break;
		case FigureActionState_43_TaxCollectorReturning:
			FigureMovement_walkTicks(f, 1);
			if (f->direction == DIR_FIGURE_AT_DESTINATION) {
				f->actionState = FigureActionState_41_TaxCollectorEnteringExiting;
				FigureAction_Common_setCrossCountryDestination(f, b->x, b->y);
				f->roamLength = 0;
			} else if (f->direction == DIR_FIGURE_REROUTE || f->direction == DIR_FIGURE_LOST) {
				f->state = FigureState_Dead;
			}
			break;
	}
	FigureActionUpdateGraphic(f, image_group(GROUP_FIGURE_TAX_COLLECTOR));
}
