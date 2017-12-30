#include "CityInfo.h"

#include "Building.h"

#include "Data/CityInfo.h"

#include "building/house_population.h"
#include "building/model.h"
#include "city/constants.h"
#include "city/message.h"
#include "city/population.h"
#include "core/calc.h"
#include "core/random.h"
#include "game/difficulty.h"
#include "game/tutorial.h"
#include "scenario/property.h"

static void healthCauseDisease(int totalPeople);

void CityInfo_Population_updateHealthRate()
{
	if (Data_CityInfo.population < 200 || scenario_is_tutorial_1() || scenario_is_tutorial_2()) {
		Data_CityInfo.healthRate = 50;
		Data_CityInfo.healthRateTarget = 50;
		return;
	}
	int totalPopulation = 0;
	int healthyPopulation = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		building *b = building_get(i);
		if (!BuildingIsInUse(b) || !b->houseSize || !b->housePopulation) {
			continue;
		}
		totalPopulation += b->housePopulation;
		if (b->subtype.houseLevel <= HOUSE_LARGE_TENT) {
			if (b->data.house.clinic) {
				healthyPopulation += b->housePopulation;
			} else {
				healthyPopulation += b->housePopulation / 4;
			}
		} else if (b->data.house.clinic) {
			if (b->houseDaysWithoutFood <= 0) {
				healthyPopulation += b->housePopulation;
			} else {
				healthyPopulation += b->housePopulation / 4;
			}
		} else if (b->houseDaysWithoutFood <= 0) {
			healthyPopulation += b->housePopulation / 4;
		}
	}
	Data_CityInfo.healthRateTarget = calc_percentage(healthyPopulation, totalPopulation);
	if (Data_CityInfo.healthRate < Data_CityInfo.healthRateTarget) {
		Data_CityInfo.healthRate += 2;
		if (Data_CityInfo.healthRate > Data_CityInfo.healthRateTarget) {
			Data_CityInfo.healthRate = Data_CityInfo.healthRateTarget;
		}
	} else if (Data_CityInfo.healthRate > Data_CityInfo.healthRateTarget) {
		Data_CityInfo.healthRate -= 2;
		if (Data_CityInfo.healthRate < Data_CityInfo.healthRateTarget) {
			Data_CityInfo.healthRate = Data_CityInfo.healthRateTarget;
		}
	}
	Data_CityInfo.healthRate = calc_bound(Data_CityInfo.healthRate, 0, 100);

	healthCauseDisease(totalPopulation);
}

static void healthCauseDisease(int totalPeople)
{
	if (Data_CityInfo.healthRate >= 40) {
		return;
	}
	int chanceValue = random_byte() & 0x3f;
	if (Data_CityInfo.godCurseVenusActive) {
		// force plague
		chanceValue = 0;
		Data_CityInfo.godCurseVenusActive = 0;
	}
	if (chanceValue > 40 - Data_CityInfo.healthRate) {
		return;
	}

	int sickPeople = calc_adjust_with_percentage(totalPeople, 7 + (random_byte() & 3));
	if (sickPeople <= 0) {
		return;
	}
	CityInfo_Population_changeHealthRate(10);
	int peopleToKill = sickPeople - Data_CityInfo.numHospitalWorkers;
	if (peopleToKill <= 0) {
		city_message_post(1, MESSAGE_HEALTH_ILLNESS, 0, 0);
		return;
	}
	if (Data_CityInfo.numHospitalWorkers > 0) {
		city_message_post(1, MESSAGE_HEALTH_DISEASE, 0, 0);
	} else {
		city_message_post(1, MESSAGE_HEALTH_PESTILENCE, 0, 0);
	}
	tutorial_on_disease();
	// kill people who don't have access to a doctor
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		building *b = building_get(i);
		if (BuildingIsInUse(b) && b->houseSize && b->housePopulation) {
			if (!b->data.house.clinic) {
				peopleToKill -= b->housePopulation;
				Building_collapseOnFire(i, 1);
				if (peopleToKill <= 0) {
					return;
				}
			}
		}
	}
	// kill people in tents
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		building *b = building_get(i);
		if (BuildingIsInUse(b) && b->houseSize && b->housePopulation) {
			if (b->subtype.houseLevel <= HOUSE_LARGE_TENT) {
				peopleToKill -= b->housePopulation;
				Building_collapseOnFire(i, 1);
				if (peopleToKill <= 0) {
					return;
				}
			}
		}
	}
	// kill anyone
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		building *b = building_get(i);
		if (BuildingIsInUse(b) && b->houseSize && b->housePopulation) {
			peopleToKill -= b->housePopulation;
			Building_collapseOnFire(i, 1);
			if (peopleToKill <= 0) {
				return;
			}
		}
	}
}

void CityInfo_Population_changeHealthRate(int amount)
{
	Data_CityInfo.healthRate = calc_bound(Data_CityInfo.healthRate + amount, 0, 100);
}
