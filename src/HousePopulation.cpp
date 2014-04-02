#include "HousePopulation.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Model.h"

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
			Data_Buildings[buildingId].isReachableFromRome > 0 &&
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
