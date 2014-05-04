#include "HousePopulation.h"

#include "BuildingHouse.h"
#include "Calc.h"
#include "CityInfo.h"
#include "PlayerMessage.h"
#include "Walker.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Message.h"
#include "Data/Model.h"
#include "Data/Walker.h"

static void calculateWorkers();
static void createImmigrants(int numPeople);
static void createEmigrants(int numPeople);
static void createImmigrantForBuilding(int buildingId, int numPeople);
static void createEmigrantForBuilding(int buildingId, int numPeople);

static void fillBuildingListHouses()
{
	Data_BuildingList.size = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse == 1 && Data_Buildings[i].houseSize) {
			Data_BuildingList.buildingIds[Data_BuildingList.size++] = i;
		}
	}
}

void HousePopulation_updateRoom()
{
	Data_CityInfo.populationMaxSupported = 0;
	Data_CityInfo.populationRoomInHouses = 0;

	fillBuildingListHouses();
	for (int i = 0; i < Data_BuildingList.size; i++) {
		struct Data_Building *b = &Data_Buildings[Data_BuildingList.buildingIds[i]];
		b->housePopulationRoom = 0;
		if (b->distanceFromEntry > 0) {
			int maxPop = Data_Model_Houses[b->subtype.houseLevel].maxPeople;
			if (b->houseIsMerged) {
				maxPop *= 4;
			}
			Data_CityInfo.populationMaxSupported += maxPop;
			Data_CityInfo.populationRoomInHouses += maxPop - b->housePopulation;
			b->housePopulationRoom = maxPop - b->housePopulation;
			if (b->housePopulation > b->houseMaxPopulationSeen) {
				b->houseMaxPopulationSeen = b->housePopulation;
			}
		} else if (b->housePopulation) {
			// not connected to Rome, mark people for eviction
			b->housePopulationRoom = -b->housePopulation;
		}
	}
}

void HousePopulation_updateMigration()
{
	CityInfo_Population_calculateMigrationSentiment();
	Data_CityInfo.populationImmigratedToday = 0;
	Data_CityInfo.populationEmigratedToday = 0;
	Data_CityInfo.populationRefusedImmigrantsNoRoom = 0;

	if (Data_CityInfo.populationImmigrationAmountPerBatch > 0) {
		if (Data_CityInfo.populationImmigrationAmountPerBatch >= 4) {
			createImmigrants(Data_CityInfo.populationImmigrationAmountPerBatch);
		} else if (Data_CityInfo.populationImmigrationAmountPerBatch +
				Data_CityInfo.populationImmigrationQueueSize >= 4) {
			createImmigrants(
				Data_CityInfo.populationImmigrationAmountPerBatch +
				Data_CityInfo.populationImmigrationQueueSize);
			Data_CityInfo.populationImmigrationQueueSize = 0;
		} else {
			// queue them for next round
			Data_CityInfo.populationImmigrationQueueSize +=
				Data_CityInfo.populationImmigrationAmountPerBatch;
		}
	}
	if (Data_CityInfo.populationEmigrationAmountPerBatch > 0) {
		if (Data_CityInfo.populationEmigrationAmountPerBatch >= 4) {
			createEmigrants(Data_CityInfo.populationEmigrationAmountPerBatch);
		} else if (Data_CityInfo.populationEmigrationAmountPerBatch +
				Data_CityInfo.populationEmigrationQueueSize >= 4) {
			createEmigrants(
				Data_CityInfo.populationEmigrationAmountPerBatch +
				Data_CityInfo.populationEmigrationQueueSize);
			Data_CityInfo.populationEmigrationQueueSize = 0;
			if (!Data_CityInfo.messageShownEmigration) {
				Data_CityInfo.messageShownEmigration = 1;
				PlayerMessage_post(1, 111, 0, 0);
			}
		} else {
			// queue them for next round
			Data_CityInfo.populationEmigrationQueueSize +=
				Data_CityInfo.populationEmigrationAmountPerBatch;
		}
	}
	Data_CityInfo.populationImmigrationAmountPerBatch = 0;
	Data_CityInfo.populationEmigrationAmountPerBatch = 0;

	CityInfo_Population_yearlyUpdate();
	calculateWorkers();
	// population messages
	if (Data_CityInfo.population >= 500 && !Data_Message.populationMessagesShown.pop500) {
		PlayerMessage_post(1, 2, 0, 0);
		Data_Message.populationMessagesShown.pop500 = 1;
	}
	if (Data_CityInfo.population >= 1000 && !Data_Message.populationMessagesShown.pop1000) {
		PlayerMessage_post(1, 3, 0, 0);
		Data_Message.populationMessagesShown.pop1000 = 1;
	}
	if (Data_CityInfo.population >= 2000 && !Data_Message.populationMessagesShown.pop2000) {
		PlayerMessage_post(1, 4, 0, 0);
		Data_Message.populationMessagesShown.pop2000 = 1;
	}
	if (Data_CityInfo.population >= 3000 && !Data_Message.populationMessagesShown.pop3000) {
		PlayerMessage_post(1, 5, 0, 0);
		Data_Message.populationMessagesShown.pop3000 = 1;
	}
	if (Data_CityInfo.population >= 5000 && !Data_Message.populationMessagesShown.pop5000) {
		PlayerMessage_post(1, 6, 0, 0);
		Data_Message.populationMessagesShown.pop5000 = 1;
	}
	if (Data_CityInfo.population >= 10000 && !Data_Message.populationMessagesShown.pop10000) {
		PlayerMessage_post(1, 7, 0, 0);
		Data_Message.populationMessagesShown.pop10000 = 1;
	}
	if (Data_CityInfo.population >= 15000 && !Data_Message.populationMessagesShown.pop15000) {
		PlayerMessage_post(1, 8, 0, 0);
		Data_Message.populationMessagesShown.pop15000 = 1;
	}
	if (Data_CityInfo.population >= 20000 && !Data_Message.populationMessagesShown.pop20000) {
		PlayerMessage_post(1, 9, 0, 0);
		Data_Message.populationMessagesShown.pop20000 = 1;
	}
	if (Data_CityInfo.population >= 25000 && !Data_Message.populationMessagesShown.pop25000) {
		PlayerMessage_post(1, 10, 0, 0);
		Data_Message.populationMessagesShown.pop25000 = 1;
	}
}

static void calculateWorkers()
{
	int numPlebs = 0;
	int numPatricians = 0;
	for (int i = 0; i < Data_BuildingList.size; i++) {
		int buildingId = Data_BuildingList.buildingIds[i];
		if (Data_Buildings[buildingId].housePopulation > 0) {
			if (Data_Buildings[buildingId].subtype.houseLevel >= HouseLevel_SmallVilla) {
				numPatricians += Data_Buildings[buildingId].housePopulation;
			} else {
				numPlebs += Data_Buildings[buildingId].housePopulation;
			}
		}
	}
	Data_CityInfo.populationPercentagePlebs = Calc_getPercentage(numPlebs, numPlebs + numPatricians);
	int workingAge = CityInfo_Population_getPeopleOfWorkingAge();
	Data_CityInfo.populationWorkingAge = Calc_adjustWithPercentage(workingAge, 60);
	Data_CityInfo.workersAvailable = Calc_adjustWithPercentage(
		Data_CityInfo.populationWorkingAge, Data_CityInfo.populationPercentagePlebs);
}

static void createImmigrants(int numPeople)
{
	int toImmigrate = numPeople;
	// clean up any dead immigrants
	for (int i = 0; i < Data_BuildingList.size; i++) {
		int buildingId = Data_BuildingList.buildingIds[i];
		if (Data_Buildings[buildingId].immigrantWalkerId &&
			Data_Walkers[Data_Buildings[buildingId].immigrantWalkerId].state != WalkerState_Alive) {
			Data_Buildings[buildingId].immigrantWalkerId = 0;
		}
	}
	// houses with plenty of room
	for (int i = 0; i < Data_BuildingList.size && toImmigrate > 0; i++) {
		int buildingId = Data_BuildingList.buildingIds[i];
		struct Data_Building *b = &Data_Buildings[buildingId];
		if (b->distanceFromEntry > 0 && b->housePopulationRoom >= 8 && !b->immigrantWalkerId) {
			if (toImmigrate <= 4) {
				createImmigrantForBuilding(buildingId, toImmigrate);
				toImmigrate = 0;
			} else {
				createImmigrantForBuilding(buildingId, 4);
				toImmigrate -= 4;
			}
		}
	}
	// houses with less room
	for (int i = 0; i < Data_BuildingList.size && toImmigrate > 0; i++) {
		int buildingId = Data_BuildingList.buildingIds[i];
		struct Data_Building *b = &Data_Buildings[buildingId];
		if (b->distanceFromEntry > 0 && b->housePopulationRoom > 0 && !b->immigrantWalkerId) {
			if (toImmigrate <= b->housePopulationRoom) {
				createImmigrantForBuilding(buildingId, toImmigrate);
				toImmigrate = 0;
			} else {
				createImmigrantForBuilding(buildingId, b->housePopulationRoom);
				toImmigrate -= b->housePopulationRoom;
			}
		}
	}
	Data_CityInfo.populationImmigratedToday += numPeople - toImmigrate;
	Data_CityInfo.populationNewcomersThisMonth += Data_CityInfo.populationImmigratedToday;
	if (toImmigrate && toImmigrate == numPeople) {
		Data_CityInfo.populationRefusedImmigrantsNoRoom += toImmigrate;
	}
}

static void createEmigrants(int numPeople)
{
	int toEmigrate = numPeople;
	for (int level = 0; level < 10 && toEmigrate > 0; level++) {
		for (int i = 0; i < Data_BuildingList.size && toEmigrate > 0; i++) {
			int buildingId = Data_BuildingList.buildingIds[i];
			if (Data_Buildings[buildingId].housePopulation > 0 &&
				Data_Buildings[buildingId].subtype.houseLevel == level) {
				int currentPeople;
				if (Data_Buildings[buildingId].housePopulation >= 4) {
					currentPeople = 4;
				} else {
					currentPeople = Data_Buildings[buildingId].housePopulation;
				}
				if (toEmigrate <= currentPeople) {
					createEmigrantForBuilding(buildingId, toEmigrate);
					toEmigrate = 0;
				} else {
					createEmigrantForBuilding(buildingId, currentPeople);
					toEmigrate -= currentPeople;
				}
			}
		}
	}
	Data_CityInfo.populationEmigratedToday += numPeople - toEmigrate;
}

static void createImmigrantForBuilding(int buildingId, int numPeople)
{
	int walkerId = Walker_create(Walker_Immigrant,
		Data_CityInfo.entryPointX, Data_CityInfo.entryPointY, 0);
	Data_Walkers[walkerId].actionState = WalkerActionState_1_ImmigrantCreated;
	Data_Walkers[walkerId].immigrantBuildingId = buildingId;
	Data_Walkers[walkerId].waitTicks =
		10 + (Data_Buildings[buildingId].houseGenerationDelay & 0x7f);
	Data_Walkers[walkerId].migrantNumPeople = numPeople;
}

static void createEmigrantForBuilding(int buildingId, int numPeople)
{
	CityInfo_Population_addPeople(-numPeople);
	if (numPeople < Data_Buildings[buildingId].housePopulation) {
		Data_Buildings[buildingId].housePopulation -= numPeople;
	} else {
		Data_Buildings[buildingId].housePopulation = 0;
		BuildingHouse_changeToVacantLot(buildingId);
	}
	int walkerId = Walker_create(Walker_Emigrant,
		Data_Buildings[buildingId].x, Data_Buildings[buildingId].y, 0);
	Data_Walkers[walkerId].actionState = WalkerActionState_4_EmigrantCreated;
	Data_Walkers[walkerId].waitTicks = 0;
	Data_Walkers[walkerId].migrantNumPeople = numPeople;
}

int HousePopulation_getClosestHouseWithRoom(int x, int y)
{
	int minDist = 1000;
	int minBuildingId = 0;
	for (int i = 1; i < Data_Buildings_Extra.highestBuildingIdInUse; i++) {
		if (Data_Buildings[i].inUse == 1 && Data_Buildings[i].houseSize &&
			Data_Buildings[i].distanceFromEntry > 0 && Data_Buildings[i].housePopulationRoom > 0) {
			if (!Data_Buildings[i].immigrantWalkerId) {
				int dist = Calc_distanceMaximum(x, y,
					Data_Buildings[i].x, Data_Buildings[i].y);
				if (dist < minDist) {
					minDist = dist;
					minBuildingId = i;
				}
			}
		}
	}
	return minBuildingId;
}

int HousePopulation_addPeople(int amount)
{
	int added = 0;
	int buildingId = Data_CityInfo.populationLastTargetHouseAdd;
	for (int i = 1; i < MAX_BUILDINGS && added < amount; i++) {
		if (++buildingId >= MAX_BUILDINGS) {
			buildingId = 1;
		}
		if (Data_Buildings[buildingId].inUse == 1 &&
			Data_Buildings[buildingId].houseSize &&
			Data_Buildings[buildingId].distanceFromEntry > 0 &&
			Data_Buildings[buildingId].housePopulation > 0) {
			Data_CityInfo.populationLastTargetHouseAdd = buildingId;
			int maxPeople = Data_Model_Houses[Data_Buildings[buildingId].subtype.houseLevel].maxPeople;
			if (Data_Buildings[buildingId].houseIsMerged) {
				maxPeople *= 4;
			}
			if (Data_Buildings[buildingId].housePopulation < maxPeople) {
				++added;
				++Data_Buildings[buildingId].housePopulation;
				Data_Buildings[buildingId].housePopulationRoom =
					maxPeople - Data_Buildings[buildingId].housePopulation;
			}
		}
	}
	return added;
}

int HousePopulation_removePeople(int amount)
{
	int removed = 0;
	int buildingId = Data_CityInfo.populationLastTargetHouseRemove;
	for (int i = 1; i < MAX_BUILDINGS && removed < amount; i++) {
		if (++buildingId >= MAX_BUILDINGS) {
			buildingId = 1;
		}
		if (Data_Buildings[buildingId].inUse == 1 &&
			Data_Buildings[buildingId].houseSize) {
			Data_CityInfo.populationLastTargetHouseRemove = buildingId;
			if (Data_Buildings[buildingId].housePopulation > 0) {
				++removed;
				--Data_Buildings[buildingId].housePopulation;
				// BUGFIX ghost bug - need to update room for people!
			}
		}
	}
	return removed;
}

int HousePopulation_calculatePeoplePerType()
{
	Data_CityInfo.populationPeopleInTentsShacks = 0;
	Data_CityInfo.populationPeopleInVillasPalaces = 0;
	Data_CityInfo.populationPeopleInTents = 0;
	Data_CityInfo.populationPeopleInLargeInsulaAndAbove = 0;
	int total = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse == 0 || Data_Buildings[i].inUse == 2 ||
			Data_Buildings[i].inUse == 5 || Data_Buildings[i].inUse == 6) {
			continue;
		}
		if (Data_Buildings[i].houseSize) {
			int pop = Data_Buildings[i].housePopulation;
			total += pop;
			if (Data_Buildings[i].subtype.houseLevel <= HouseLevel_LargeTent) {
				Data_CityInfo.populationPeopleInTents += pop;
			}
			if (Data_Buildings[i].subtype.houseLevel <= HouseLevel_LargeShack) {
				Data_CityInfo.populationPeopleInTentsShacks += pop;
			}
			if (Data_Buildings[i].subtype.houseLevel >= HouseLevel_LargeInsula) {
				Data_CityInfo.populationPeopleInLargeInsulaAndAbove += pop;
			}
			if (Data_Buildings[i].subtype.houseLevel >= HouseLevel_SmallVilla) {
				Data_CityInfo.populationPeopleInVillasPalaces += pop;
			}
		}
	}
	return total;
}

void HousePopulation_createHomeless(int x, int y, int numPeople)
{
	int walkerId = Walker_create(Walker_Homeless, x, y, 0);
	Data_Walkers[walkerId].actionState = WalkerActionState_7_HomelessCreated;
	Data_Walkers[walkerId].waitTicks = 0;
	Data_Walkers[walkerId].migrantNumPeople = numPeople;
	CityInfo_Population_removePeopleHomeless(numPeople);
}

void HousePopulation_evictOvercrowded()
{
	for (int i = 0; i < Data_BuildingList.size; i++) {
		int buildingId = Data_BuildingList.buildingIds[i];
		struct Data_Building *b = &Data_Buildings[buildingId];
		if (b->housePopulationRoom < 0) {
			int numPeople = -b->housePopulationRoom;
			HousePopulation_createHomeless(b->x, b->y, numPeople);
			if (numPeople < b->housePopulation) {
				b->housePopulation -= numPeople;
			} else {
				// house has been removed
				b->inUse = 2;
			}
		}
	}
}
