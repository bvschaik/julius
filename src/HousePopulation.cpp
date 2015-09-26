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
	Data_BuildingList.large.size = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (BuildingIsInUse(i) && Data_Buildings[i].houseSize) {
			Data_BuildingList.large.items[Data_BuildingList.large.size++] = i;
		}
	}
}

void HousePopulation_updateRoom()
{
	Data_CityInfo.populationMaxSupported = 0;
	Data_CityInfo.populationRoomInHouses = 0;

	fillBuildingListHouses();
	for (int i = 0; i < Data_BuildingList.large.size; i++) {
		struct Data_Building *b = &Data_Buildings[Data_BuildingList.large.items[i]];
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
				PlayerMessage_post(1, Message_111_Emigration, 0, 0);
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
		PlayerMessage_post(1, Message_2_Population500, 0, 0);
		Data_Message.populationMessagesShown.pop500 = 1;
	}
	if (Data_CityInfo.population >= 1000 && !Data_Message.populationMessagesShown.pop1000) {
		PlayerMessage_post(1, Message_3_Population1000, 0, 0);
		Data_Message.populationMessagesShown.pop1000 = 1;
	}
	if (Data_CityInfo.population >= 2000 && !Data_Message.populationMessagesShown.pop2000) {
		PlayerMessage_post(1, Message_4_Population2000, 0, 0);
		Data_Message.populationMessagesShown.pop2000 = 1;
	}
	if (Data_CityInfo.population >= 3000 && !Data_Message.populationMessagesShown.pop3000) {
		PlayerMessage_post(1, Message_5_Population3000, 0, 0);
		Data_Message.populationMessagesShown.pop3000 = 1;
	}
	if (Data_CityInfo.population >= 5000 && !Data_Message.populationMessagesShown.pop5000) {
		PlayerMessage_post(1, Message_6_Population5000, 0, 0);
		Data_Message.populationMessagesShown.pop5000 = 1;
	}
	if (Data_CityInfo.population >= 10000 && !Data_Message.populationMessagesShown.pop10000) {
		PlayerMessage_post(1, Message_7_Population10000, 0, 0);
		Data_Message.populationMessagesShown.pop10000 = 1;
	}
	if (Data_CityInfo.population >= 15000 && !Data_Message.populationMessagesShown.pop15000) {
		PlayerMessage_post(1, Message_8_Population15000, 0, 0);
		Data_Message.populationMessagesShown.pop15000 = 1;
	}
	if (Data_CityInfo.population >= 20000 && !Data_Message.populationMessagesShown.pop20000) {
		PlayerMessage_post(1, Message_9_Population20000, 0, 0);
		Data_Message.populationMessagesShown.pop20000 = 1;
	}
	if (Data_CityInfo.population >= 25000 && !Data_Message.populationMessagesShown.pop25000) {
		PlayerMessage_post(1, Message_10_Population25000, 0, 0);
		Data_Message.populationMessagesShown.pop25000 = 1;
	}
}

static void calculateWorkers()
{
	int numPlebs = 0;
	int numPatricians = 0;
	for (int i = 0; i < Data_BuildingList.large.size; i++) {
		int buildingId = Data_BuildingList.large.items[i];
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
	for (int i = 0; i < Data_BuildingList.large.size; i++) {
		int buildingId = Data_BuildingList.large.items[i];
		if (Data_Buildings[buildingId].immigrantWalkerId &&
			Data_Walkers[Data_Buildings[buildingId].immigrantWalkerId].state != WalkerState_Alive) {
			Data_Buildings[buildingId].immigrantWalkerId = 0;
		}
	}
	// houses with plenty of room
	for (int i = 0; i < Data_BuildingList.large.size && toImmigrate > 0; i++) {
		int buildingId = Data_BuildingList.large.items[i];
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
	for (int i = 0; i < Data_BuildingList.large.size && toImmigrate > 0; i++) {
		int buildingId = Data_BuildingList.large.items[i];
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
	for (int level = HouseLevel_SmallTent; level < HouseLevel_LargeInsula && toEmigrate > 0; level++) {
		for (int i = 0; i < Data_BuildingList.large.size && toEmigrate > 0; i++) {
			int buildingId = Data_BuildingList.large.items[i];
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
	Data_Buildings[buildingId].immigrantWalkerId = walkerId;
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
	for (int i = 1; i <= Data_Buildings_Extra.highestBuildingIdInUse; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (BuildingIsInUse(i) && b->houseSize && b->distanceFromEntry > 0 && b->housePopulationRoom > 0) {
			if (!b->immigrantWalkerId) {
				int dist = Calc_distanceMaximum(x, y, b->x, b->y);
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
		struct Data_Building *b = &Data_Buildings[buildingId];
		if (BuildingIsInUse(buildingId) && b->houseSize && b->distanceFromEntry > 0 && b->housePopulation > 0) {
			Data_CityInfo.populationLastTargetHouseAdd = buildingId;
			int maxPeople = Data_Model_Houses[b->subtype.houseLevel].maxPeople;
			if (b->houseIsMerged) {
				maxPeople *= 4;
			}
			if (b->housePopulation < maxPeople) {
				++added;
				++b->housePopulation;
				b->housePopulationRoom = maxPeople - b->housePopulation;
			}
		}
	}
	return added;
}

int HousePopulation_removePeople(int amount)
{
	int removed = 0;
	int buildingId = Data_CityInfo.populationLastTargetHouseRemove;
	for (int i = 1; i < 4 * MAX_BUILDINGS && removed < amount; i++) {
		if (++buildingId >= MAX_BUILDINGS) {
			buildingId = 1;
		}
		struct Data_Building *b = &Data_Buildings[buildingId];
		if (BuildingIsInUse(buildingId) && b->houseSize) {
			Data_CityInfo.populationLastTargetHouseRemove = buildingId;
			if (b->housePopulation > 0) {
				++removed;
				--Data_Buildings[buildingId].housePopulation;
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
		struct Data_Building *b = &Data_Buildings[i];
		if (b->state == BuildingState_Unused ||
			b->state == BuildingState_Undo ||
			b->state == BuildingState_DeletedByGame ||
			b->state == BuildingState_DeletedByPlayer) {
			continue;
		}
		if (b->houseSize) {
			int pop = b->housePopulation;
			total += pop;
			if (b->subtype.houseLevel <= HouseLevel_LargeTent) {
				Data_CityInfo.populationPeopleInTents += pop;
			}
			if (b->subtype.houseLevel <= HouseLevel_LargeShack) {
				Data_CityInfo.populationPeopleInTentsShacks += pop;
			}
			if (b->subtype.houseLevel >= HouseLevel_LargeInsula) {
				Data_CityInfo.populationPeopleInLargeInsulaAndAbove += pop;
			}
			if (b->subtype.houseLevel >= HouseLevel_SmallVilla) {
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
	for (int i = 0; i < Data_BuildingList.large.size; i++) {
		int buildingId = Data_BuildingList.large.items[i];
		struct Data_Building *b = &Data_Buildings[buildingId];
		if (b->housePopulationRoom < 0) {
			int numPeopleToEvict = -b->housePopulationRoom;
			HousePopulation_createHomeless(b->x, b->y, numPeopleToEvict);
			if (numPeopleToEvict < b->housePopulation) {
				b->housePopulation -= numPeopleToEvict;
			} else {
				// house has been removed
				b->state = BuildingState_Undo;
			}
		}
	}
}
