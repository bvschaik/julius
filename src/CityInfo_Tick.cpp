#include "CityInfo.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Building.h"
#include "Data/Walker.h"

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
	for (int i = 0; i < 16; i++) {
		Data_CityInfo_Buildings.industry.working[i] = 0;
		Data_CityInfo_Buildings.industry.total[i] = 0;
	}
	Data_CityInfo.numWorkingWharfs = 0;
	Data_CityInfo.shipyardBoatsRequested = 0;
	Data_CityInfo.numWorkingDocks = 0;
	Data_CityInfo.numHospitalWorkers = 0;
	for (int i = 0; i < 8; i++) {
		Data_CityInfo.workingDockBuildingIds[i] = 0;
	}
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
		if (Data_Buildings[i].inUse != 1 || Data_Buildings[i].houseSize) {
			continue;
		}
		int isEntertainmentVenue;
		switch (Data_Buildings[i].type) {
			// entertainment venues
			case Building_Theater:
				isEntertainmentVenue = 1;
				UPDATE(theater);
				break;
			case Building_Amphitheater:
				isEntertainmentVenue = 1;
				UPDATE(amphitheater);
				break;
			case Building_Colosseum:
				isEntertainmentVenue = 1;
				UPDATE(colosseum);
				break;
			case Building_Hippodrome:
				isEntertainmentVenue = 1;
				UPDATE(hippodrome);
				break;
			// education
			case Building_School:
				UPDATE(school);
				break;
			case Building_Library:
				UPDATE(library);
				break;
			case Building_Academy:
				UPDATE(academy);
				break;
			// health
			case Building_Barber:
				UPDATE(barber);
				break;
			case Building_Bathhouse:
				UPDATE(bathhouse);
				break;
			case Building_Doctor:
				UPDATE(clinic);
				break;
			case Building_Hospital:
				UPDATE(hospital);
				Data_CityInfo.numHospitalWorkers += Data_Buildings[i].numWorkers;
				break;
			// government
			case Building_Forum:
				UPDATE(forum);
				break;
			case Building_ForumUpgraded:
				UPDATE(forumUpgraded);
				break;
			case Building_Senate:
				UPDATE(senate);
				break;
			case Building_SenateUpgraded:
				UPDATE(senateUpgraded);
				break;
			// entertainment schools
			case Building_ActorColony:
				UPDATE(actorColony);
				break;
			case Building_GladiatorSchool:
				UPDATE(gladiatorSchool);
				break;
			case Building_LionHouse:
				UPDATE(lionHouse);
				break;
			case Building_ChariotMaker:
				UPDATE(chariotMaker);
				break;
			// distribution
			case Building_Market:
				UPDATE(market);
				break;
			// military
			case Building_MilitaryAcademy:
				UPDATE(militaryAcademy);
				break;
			case Building_Barracks:
				UPDATE(barracks);
				break;
			// religion
			case Building_SmallTempleCeres:
				UPDATE(smallTempleCeres);
				break;
			case Building_SmallTempleNeptune:
				UPDATE(smallTempleNeptune);
				break;
			case Building_SmallTempleMercury:
				UPDATE(smallTempleMercury);
				break;
			case Building_SmallTempleMars:
				UPDATE(smallTempleMars);
				break;
			case Building_SmallTempleVenus:
				UPDATE(smallTempleVenus);
				break;
			case Building_LargeTempleCeres:
				UPDATE(largeTempleCeres);
				break;
			case Building_LargeTempleNeptune:
				UPDATE(largeTempleNeptune);
				break;
			case Building_LargeTempleMercury:
				UPDATE(largeTempleMercury);
				break;
			case Building_LargeTempleMars:
				UPDATE(largeTempleMars);
				break;
			case Building_LargeTempleVenus:
				UPDATE(largeTempleVenus);
				break;
			case Building_Oracle:
				++Data_CityInfo_Buildings.oracle.total;
				break;
			// water
			case Building_Reservoir:
				++Data_CityInfo_Buildings.reservoir.total;
				if (Data_Buildings[i].hasWaterAccess) {
					++Data_CityInfo_Buildings.reservoir.working;
				}
				break;
			case Building_Fountain:
				++Data_CityInfo_Buildings.fountain.total;
				if (Data_Buildings[i].hasWaterAccess) {
					++Data_CityInfo_Buildings.fountain.working;
				}
				break;
			// industry
			case Building_WheatFarm:
				UPDATE_RES(Resource_Wheat);
				break;
			case Building_VegetableFarm:
				UPDATE_RES(Resource_Vegetables);
				break;
			case Building_FruitFarm:
				UPDATE_RES(Resource_Fruit);
				break;
			case Building_OliveFarm:
				UPDATE_RES(Resource_Olives);
				break;
			case Building_VinesFarm:
				UPDATE_RES(Resource_Vines);
				break;
			case Building_PigFarm:
				UPDATE_RES(Resource_Meat);
				break;
			case Building_MarbleQuarry:
				UPDATE_RES(Resource_Marble);
				break;
			case Building_IronMine:
				UPDATE_RES(Resource_Iron);
				break;
			case Building_TimberYard:
				UPDATE_RES(Resource_Timber);
				break;
			case Building_ClayPit:
				UPDATE_RES(Resource_Clay);
				break;
			case Building_WineWorkshop:
				UPDATE_RES(Resource_Wine);
				break;
			case Building_OilWorkshop:
				UPDATE_RES(Resource_Oil);
				break;
			case Building_WeaponsWorkshop:
				UPDATE_RES(Resource_Weapons);
				break;
			case Building_FurnitureWorkshop:
				UPDATE_RES(Resource_Furniture);
				break;
			case Building_PotteryWorkshop:
				UPDATE_RES(Resource_Pottery);
				break;
			// water-side
			case Building_Wharf:
				if (Data_Buildings[i].numWorkers > 0) {
					++Data_CityInfo.numWorkingWharfs;
					if (!Data_Buildings[i].data.other.boatWalkerId) {
						++Data_CityInfo.shipyardBoatsRequested;
					}
				}
				break;
			case Building_Dock:
				if (Data_Buildings[i].numWorkers > 0 && Data_Buildings[i].hasWaterAccess) {
					if (Data_CityInfo.numWorkingDocks < 10) {
						Data_CityInfo.workingDockBuildingIds[Data_CityInfo.numWorkingDocks] = i;
					}
					++Data_CityInfo.numWorkingDocks;
				}
				break;
		}
		if (Data_Buildings[i].immigrantWalkerId) {
			int immigrantId = Data_Buildings[i].immigrantWalkerId;
			if (Data_Walkers[immigrantId].state != 1 || Data_Walkers[immigrantId].destinationBuildingId != i) {
				Data_Buildings[i].immigrantWalkerId = 0;
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
