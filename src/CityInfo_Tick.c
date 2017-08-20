#include "CityInfo.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Building.h"
#include "Data/Figure.h"

#define CLEAR(t) Data_CityInfo_Buildings.t.working = Data_CityInfo_Buildings.t.total = 0
#define UPDATE(t) \
	++Data_CityInfo_Buildings.t.total;\
	if (Data_Buildings[i].numWorkers > 0) {\
		++Data_CityInfo_Buildings.t.working;\
	}
#define UPDATE_RES(r) \
	++Data_CityInfo_Buildings.industry.total[r];\
	if (Data_Buildings[i].numWorkers > 0) {\
		++Data_CityInfo_Buildings.industry.working[r];\
	}

void CityInfo_Tick_countBuildingTypes()
{
	CLEAR(amphitheater);
	CLEAR(theater);
	CLEAR(hippodrome);
	CLEAR(colosseum);
	CLEAR(library);
	CLEAR(school);
	CLEAR(academy);
	CLEAR(barber);
	CLEAR(clinic);
	CLEAR(bathhouse);
	CLEAR(hospital);
	CLEAR(forum);
	CLEAR(forumUpgraded);
	CLEAR(senate);
	CLEAR(senateUpgraded);
	CLEAR(actorColony);
	CLEAR(gladiatorSchool);
	CLEAR(lionHouse);
	CLEAR(chariotMaker);
	CLEAR(market);
	CLEAR(reservoir);
	CLEAR(fountain);
	CLEAR(militaryAcademy);
	CLEAR(barracks);
	for (int i = 0; i < Resource_Max; i++) {
		Data_CityInfo_Buildings.industry.working[i] = 0;
		Data_CityInfo_Buildings.industry.total[i] = 0;
	}
	Data_CityInfo.numWorkingWharfs = 0;
	Data_CityInfo.shipyardBoatsRequested = 0;
	for (int i = 0; i < 8; i++) {
		Data_CityInfo.workingDockBuildingIds[i] = 0;
	}
	Data_CityInfo.numWorkingDocks = 0;
	Data_CityInfo.numHospitalWorkers = 0;
	CLEAR(smallTempleCeres);
	CLEAR(smallTempleNeptune);
	CLEAR(smallTempleMercury);
	CLEAR(smallTempleMars);
	CLEAR(smallTempleVenus);
	CLEAR(largeTempleCeres);
	CLEAR(largeTempleNeptune);
	CLEAR(largeTempleMercury);
	CLEAR(largeTempleMars);
	CLEAR(largeTempleVenus);
	CLEAR(oracle);

	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!BuildingIsInUse(i) || Data_Buildings[i].houseSize) {
			continue;
		}
		int isEntertainmentVenue = 0;
		switch (Data_Buildings[i].type) {
			// entertainment venues
			case BUILDING_THEATER:
				isEntertainmentVenue = 1;
				UPDATE(theater);
				break;
			case BUILDING_AMPHITHEATER:
				isEntertainmentVenue = 1;
				UPDATE(amphitheater);
				break;
			case BUILDING_COLOSSEUM:
				isEntertainmentVenue = 1;
				UPDATE(colosseum);
				break;
			case BUILDING_HIPPODROME:
				isEntertainmentVenue = 1;
				UPDATE(hippodrome);
				break;
			// education
			case BUILDING_SCHOOL:
				UPDATE(school);
				break;
			case BUILDING_LIBRARY:
				UPDATE(library);
				break;
			case BUILDING_ACADEMY:
				UPDATE(academy);
				break;
			// health
			case BUILDING_BARBER:
				UPDATE(barber);
				break;
			case BUILDING_BATHHOUSE:
				UPDATE(bathhouse);
				break;
			case BUILDING_DOCTOR:
				UPDATE(clinic);
				break;
			case BUILDING_HOSPITAL:
				UPDATE(hospital);
				Data_CityInfo.numHospitalWorkers += Data_Buildings[i].numWorkers;
				break;
			// government
			case BUILDING_FORUM:
				UPDATE(forum);
				break;
			case BUILDING_FORUM_UPGRADED:
				UPDATE(forumUpgraded);
				break;
			case BUILDING_SENATE:
				UPDATE(senate);
				break;
			case BUILDING_SENATE_UPGRADED:
				UPDATE(senateUpgraded);
				break;
			// entertainment schools
			case BUILDING_ACTOR_COLONY:
				UPDATE(actorColony);
				break;
			case BUILDING_GLADIATOR_SCHOOL:
				UPDATE(gladiatorSchool);
				break;
			case BUILDING_LION_HOUSE:
				UPDATE(lionHouse);
				break;
			case BUILDING_CHARIOT_MAKER:
				UPDATE(chariotMaker);
				break;
			// distribution
			case BUILDING_MARKET:
				UPDATE(market);
				break;
			// military
			case BUILDING_MILITARY_ACADEMY:
				UPDATE(militaryAcademy);
				break;
			case BUILDING_BARRACKS:
				Data_CityInfo.buildingBarracksBuildingId = i;
				UPDATE(barracks);
				break;
			// religion
			case BUILDING_SMALL_TEMPLE_CERES:
				UPDATE(smallTempleCeres);
				break;
			case BUILDING_SMALL_TEMPLE_NEPTUNE:
				UPDATE(smallTempleNeptune);
				break;
			case BUILDING_SMALL_TEMPLE_MERCURY:
				UPDATE(smallTempleMercury);
				break;
			case BUILDING_SMALL_TEMPLE_MARS:
				UPDATE(smallTempleMars);
				break;
			case BUILDING_SMALL_TEMPLE_VENUS:
				UPDATE(smallTempleVenus);
				break;
			case BUILDING_LARGE_TEMPLE_CERES:
				UPDATE(largeTempleCeres);
				break;
			case BUILDING_LARGE_TEMPLE_NEPTUNE:
				UPDATE(largeTempleNeptune);
				break;
			case BUILDING_LARGE_TEMPLE_MERCURY:
				UPDATE(largeTempleMercury);
				break;
			case BUILDING_LARGE_TEMPLE_MARS:
				UPDATE(largeTempleMars);
				break;
			case BUILDING_LARGE_TEMPLE_VENUS:
				UPDATE(largeTempleVenus);
				break;
			case BUILDING_ORACLE:
				++Data_CityInfo_Buildings.oracle.total;
				break;
			// water
			case BUILDING_RESERVOIR:
				++Data_CityInfo_Buildings.reservoir.total;
				if (Data_Buildings[i].hasWaterAccess) {
					++Data_CityInfo_Buildings.reservoir.working;
				}
				break;
			case BUILDING_FOUNTAIN:
				++Data_CityInfo_Buildings.fountain.total;
				if (Data_Buildings[i].hasWaterAccess) {
					++Data_CityInfo_Buildings.fountain.working;
				}
				break;
			// industry
			case BUILDING_WHEAT_FARM:
				UPDATE_RES(Resource_Wheat);
				break;
			case BUILDING_VEGETABLE_FARM:
				UPDATE_RES(Resource_Vegetables);
				break;
			case BUILDING_FRUIT_FARM:
				UPDATE_RES(Resource_Fruit);
				break;
			case BUILDING_OLIVE_FARM:
				UPDATE_RES(Resource_Olives);
				break;
			case BUILDING_VINES_FARM:
				UPDATE_RES(Resource_Vines);
				break;
			case BUILDING_PIG_FARM:
				UPDATE_RES(Resource_Meat);
				break;
			case BUILDING_MARBLE_QUARRY:
				UPDATE_RES(Resource_Marble);
				break;
			case BUILDING_IRON_MINE:
				UPDATE_RES(Resource_Iron);
				break;
			case BUILDING_TIMBER_YARD:
				UPDATE_RES(Resource_Timber);
				break;
			case BUILDING_CLAY_PIT:
				UPDATE_RES(Resource_Clay);
				break;
			case BUILDING_WINE_WORKSHOP:
				UPDATE_RES(Resource_Wine);
				break;
			case BUILDING_OIL_WORKSHOP:
				UPDATE_RES(Resource_Oil);
				break;
			case BUILDING_WEAPONS_WORKSHOP:
				UPDATE_RES(Resource_Weapons);
				break;
			case BUILDING_FURNITURE_WORKSHOP:
				UPDATE_RES(Resource_Furniture);
				break;
			case BUILDING_POTTERY_WORKSHOP:
				UPDATE_RES(Resource_Pottery);
				break;
			// water-side
			case BUILDING_WHARF:
				if (Data_Buildings[i].numWorkers > 0) {
					++Data_CityInfo.numWorkingWharfs;
					if (!Data_Buildings[i].data.other.boatFigureId) {
						++Data_CityInfo.shipyardBoatsRequested;
					}
				}
				break;
			case BUILDING_DOCK:
				if (Data_Buildings[i].numWorkers > 0 && Data_Buildings[i].hasWaterAccess) {
					if (Data_CityInfo.numWorkingDocks < 10) {
						Data_CityInfo.workingDockBuildingIds[Data_CityInfo.numWorkingDocks] = i;
					}
					++Data_CityInfo.numWorkingDocks;
				}
				break;
		}
		if (Data_Buildings[i].immigrantFigureId) {
			int immigrantId = Data_Buildings[i].immigrantFigureId;
			if (Data_Figures[immigrantId].state != FigureState_Alive || Data_Figures[immigrantId].destinationBuildingId != i) {
				Data_Buildings[i].immigrantFigureId = 0;
			}
		}
		if (isEntertainmentVenue) {
			// update number of shows
			int shows = 0;
			if (Data_Buildings[i].data.entertainment.days1 > 0) {
				--Data_Buildings[i].data.entertainment.days1;
				++shows;
			}
			if (Data_Buildings[i].data.entertainment.days2 > 0) {
				--Data_Buildings[i].data.entertainment.days2;
				++shows;
			}
			Data_Buildings[i].data.entertainment.numShows = shows;
		}
	}
	if (Data_CityInfo_Buildings.hippodrome.total > 1) {
		Data_CityInfo_Buildings.hippodrome.total = 1;
	}
	if (Data_CityInfo_Buildings.hippodrome.working > 1) {
		Data_CityInfo_Buildings.hippodrome.working = 1;
	}
}

void CityInfo_Tick_distributeTreasuryOverForumsAndSenates()
{
	int units =
		5 * Data_CityInfo_Buildings.senate.working +
		1 * Data_CityInfo_Buildings.forum.working +
		8 * Data_CityInfo_Buildings.senateUpgraded.working +
		2 * Data_CityInfo_Buildings.forumUpgraded.working;
	int amountPerUnit;
	int remainder;
	if (Data_CityInfo.treasury > 0 && units > 0) {
		amountPerUnit = Data_CityInfo.treasury / units;
		remainder = Data_CityInfo.treasury - units * amountPerUnit;
	} else {
		amountPerUnit = 0;
		remainder = 0;
	}

	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!BuildingIsInUse(i) || Data_Buildings[i].houseSize) {
			continue;
		}
		Data_Buildings[i].taxIncomeOrStorage = 0;
		if (Data_Buildings[i].numWorkers <= 0) {
			continue;
		}
		switch (Data_Buildings[i].type) {
			// ordered based on importance: most important gets the remainder
			case BUILDING_SENATE_UPGRADED:
				Data_Buildings[i].taxIncomeOrStorage = 8 * amountPerUnit + remainder;
				remainder = 0;
				break;
			case BUILDING_SENATE:
				if (remainder && !Data_CityInfo_Buildings.senateUpgraded.working) {
					Data_Buildings[i].taxIncomeOrStorage = 5 * amountPerUnit + remainder;
					remainder = 0;
				} else {
					Data_Buildings[i].taxIncomeOrStorage = 5 * amountPerUnit;
				}
				break;
			case BUILDING_FORUM_UPGRADED:
				if (remainder && !(
					Data_CityInfo_Buildings.senateUpgraded.working ||
					Data_CityInfo_Buildings.senate.working)) {
					Data_Buildings[i].taxIncomeOrStorage = 2 * amountPerUnit + remainder;
					remainder = 0;
				} else {
					Data_Buildings[i].taxIncomeOrStorage = 2 * amountPerUnit;
				}
				break;
			case BUILDING_FORUM:
				if (remainder && !(
					Data_CityInfo_Buildings.senateUpgraded.working ||
					Data_CityInfo_Buildings.senate.working ||
					Data_CityInfo_Buildings.forumUpgraded.working)) {
					Data_Buildings[i].taxIncomeOrStorage = amountPerUnit + remainder;
					remainder = 0;
				} else {
					Data_Buildings[i].taxIncomeOrStorage = amountPerUnit;
				}
				break;
		}
	}
}
