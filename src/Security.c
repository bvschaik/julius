#include "Security.h"

#include "Building.h"
#include "CityInfo.h"
#include "Figure.h"
#include "FigureAction.h"
#include "Formation.h"
#include "SidebarMenu.h"
#include "Terrain.h"
#include "TerrainGraphics.h"

#include "Data/CityInfo.h"
#include "Data/State.h"

#include "building/building.h"
#include "building/list.h"
#include "city/finance.h"
#include "city/message.h"
#include "core/calc.h"
#include "core/random.h"
#include "figure/figure.h"
#include "figure/type.h"
#include "game/tutorial.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/random.h"
#include "map/routing_terrain.h"
#include "scenario/property.h"
#include "sound/effect.h"

static void generateRioter(int buildingId)
{
	int xRoad, yRoad;
	struct Data_Building *b = building_get(buildingId);
	if (!Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 4, &xRoad, &yRoad)) {
		return;
	}
	Data_CityInfo.numCriminalsThisMonth++;
	int peopleInMob;
	if (Data_CityInfo.population <= 150) {
		peopleInMob = 1;
	} else if (Data_CityInfo.population <= 300) {
		peopleInMob = 2;
	} else if (Data_CityInfo.population <= 800) {
		peopleInMob = 3;
	} else if (Data_CityInfo.population <= 1200) {
		peopleInMob = 4;
	} else if (Data_CityInfo.population <= 2000) {
		peopleInMob = 5;
	} else {
		peopleInMob = 6;
	}
	int targetX, targetY;
	int targetBuildingId = Formation_Rioter_getTargetBuilding(&targetX, &targetY);
	for (int i = 0; i < peopleInMob; i++) {
		figure *f = figure_create(FIGURE_RIOTER, xRoad, yRoad, DIR_4_BOTTOM);
		f->actionState = FigureActionState_120_RioterCreated;
		f->roamLength = 0;
		f->waitTicks = 10 + 4 * i;
		if (targetBuildingId) {
			f->destinationX = targetX;
			f->destinationY = targetY;
			f->destinationBuildingId = targetBuildingId;
		} else {
			f->state = FigureState_Dead;
		}
	}
	Building_collapseOnFire(buildingId, 0);
	Data_CityInfo.ratingPeaceNumRiotersThisYear++;
	Data_CityInfo.riotCause = Data_CityInfo.populationEmigrationCause;
	CityInfo_Population_changeHappiness(20);
    tutorial_on_crime();
    city_message_apply_sound_interval(MESSAGE_CAT_RIOT);
	city_message_post_with_popup_delay(MESSAGE_CAT_RIOT, MESSAGE_RIOT, b->type, map_grid_offset(xRoad, yRoad));
}

static void generateMugger(int buildingId)
{
	Data_CityInfo.numCriminalsThisMonth++;
	struct Data_Building *b = building_get(buildingId);
	if (b->houseCriminalActive < 2) {
		b->houseCriminalActive = 2;
		int xRoad, yRoad;
		if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
			figure *f = figure_create(FIGURE_CRIMINAL, xRoad, yRoad, DIR_4_BOTTOM);
			f->waitTicks = 10 + (b->houseGenerationDelay & 0xf);
			Data_CityInfo.ratingPeaceNumCriminalsThisYear++;
			if (Data_CityInfo.financeTaxesThisYear > 20) {
				int moneyStolen = Data_CityInfo.financeTaxesThisYear / 4;
				if (moneyStolen > 400) {
					moneyStolen = 400 - random_byte() / 2;
				}
				city_message_post(1, MESSAGE_THEFT, moneyStolen, f->gridOffset);
                city_finance_process_stolen(moneyStolen);
			}
		}
	}
}

static void generateProtester(int buildingId)
{
	Data_CityInfo.numProtestersThisMonth++;
	struct Data_Building *b = building_get(buildingId);
	if (b->houseCriminalActive < 1) {
		b->houseCriminalActive = 1;
		int xRoad, yRoad;
		if (Terrain_getClosestRoadWithinRadius(b->x, b->y, b->size, 2, &xRoad, &yRoad)) {
			figure *f = figure_create(FIGURE_PROTESTER, xRoad, yRoad, DIR_4_BOTTOM);
			f->waitTicks = 10 + (b->houseGenerationDelay & 0xf);
			Data_CityInfo.ratingPeaceNumCriminalsThisYear++;
		}
	}
}

void Security_Tick_generateCriminal()
{
	int minBuildingId = 0;
	int minHappiness = 50;
	for (int i = 1; i <= Data_Buildings_Extra.highestBuildingIdInUse; i++) {
		struct Data_Building *b = building_get(i);
		if (BuildingIsInUse(i) && b->houseSize) {
			if (b->sentiment.houseHappiness >= 50) {
				b->houseCriminalActive = 0;
			} else if (b->sentiment.houseHappiness < minHappiness) {
				minHappiness = b->sentiment.houseHappiness;
				minBuildingId = i;
			}
		}
	}
	if (minBuildingId) {
		if (scenario_is_tutorial_1() || scenario_is_tutorial_2()) {
			return;
		}
		if (Data_CityInfo.citySentiment < 30) {
			if (random_byte() >= Data_CityInfo.citySentiment + 50) {
				if (minHappiness <= 10) {
					generateRioter(minBuildingId);
				} else if (minHappiness < 30) {
					generateMugger(minBuildingId);
				} else if (minHappiness < 50) {
					generateProtester(minBuildingId);
				}
			}
		} else if (Data_CityInfo.citySentiment < 60) {
			if (random_byte() >= Data_CityInfo.citySentiment + 40) {
				if (minHappiness < 30) {
					generateMugger(minBuildingId);
				} else if (minHappiness < 50) {
					generateProtester(minBuildingId);
				}
			}
		} else {
			if (random_byte() >= Data_CityInfo.citySentiment + 20) {
				if (minHappiness < 50) {
					generateProtester(minBuildingId);
				}
			}
		}
	}
}
