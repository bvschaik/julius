#include "HouseEvolution.h"

#include "TerrainGraphics.h"

#include "Data/CityInfo.h"

#include "building/building.h"
#include "building/house.h"
#include "building/house_evolution.h"
#include "building/model.h"
#include "city/culture.h"
#include "game/resource.h"
#include "game/time.h"
#include "map/routing_terrain.h"

static void resetCityInfoServiceRequiredCounters();
static void consumeResources(building *b);

void HouseEvolution_Tick_evolveAndConsumeResources()
{
	resetCityInfoServiceRequiredCounters();
	int hasExpanded = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
		if (BuildingIsInUse(b) && BuildingIsHouse(b->type)) {
			building_house_check_for_corruption(b);
			hasExpanded |= building_house_process_evolve(b);
			if (game_time_day() == 0 || game_time_day() == 7) {
				consumeResources(b);
			}
		}
	}
	if (hasExpanded) {
		map_routing_update_land();
	}
}

static void consumeResources(building *b)
{
    const model_house *model = model_get_house(b->subtype.houseLevel);
	int pottery = model->pottery;
	int furniture = model->furniture;
	int oil = model->oil;
	int wine = model->wine;

	if (pottery > 0) {
		if (pottery > b->data.house.inventory[INVENTORY_POTTERY]) {
			b->data.house.inventory[INVENTORY_POTTERY] = 0;
		} else {
			b->data.house.inventory[INVENTORY_POTTERY] -= pottery;
		}
	}
	if (furniture > 0) {
		if (furniture > b->data.house.inventory[INVENTORY_FURNITURE]) {
			b->data.house.inventory[INVENTORY_FURNITURE] = 0;
		} else {
			b->data.house.inventory[INVENTORY_FURNITURE] -= furniture;
		}
	}
	if (oil > 0) {
		if (oil > b->data.house.inventory[INVENTORY_OIL]) {
			b->data.house.inventory[INVENTORY_OIL] = 0;
		} else {
			b->data.house.inventory[INVENTORY_OIL] -= oil;
		}
	}
	if (wine > 0) {
		if (wine > b->data.house.inventory[INVENTORY_WINE]) {
			b->data.house.inventory[INVENTORY_WINE] = 0;
		} else {
			b->data.house.inventory[INVENTORY_WINE] -= wine;
		}
	}
}

static void resetCityInfoServiceRequiredCounters()
{
	Data_CityInfo.housesRequiringFountainToEvolve = 0;
	Data_CityInfo.housesRequiringWellToEvolve = 0;
	Data_CityInfo.housesRequiringEntertainmentToEvolve = 0;
	Data_CityInfo.housesRequiringMoreEntertainmentToEvolve = 0;
	Data_CityInfo.housesRequiringEducationToEvolve = 0;
	Data_CityInfo.housesRequiringMoreEducationToEvolve = 0;
	Data_CityInfo.housesRequiringReligionToEvolve = 0;
	Data_CityInfo.housesRequiringMoreReligionToEvolve = 0;
	Data_CityInfo.housesRequiringEvenMoreReligionToEvolve = 0;
	Data_CityInfo.housesRequiringBarberToEvolve = 0;
	Data_CityInfo.housesRequiringBathhouseToEvolve = 0;
	Data_CityInfo.housesRequiringClinicToEvolve = 0;
	Data_CityInfo.housesRequiringHospitalToEvolve = 0;
	Data_CityInfo.housesRequiringFoodToEvolve = 0;

	Data_CityInfo.housesRequiringSchool = 0;
	Data_CityInfo.housesRequiringLibrary = 0;
	Data_CityInfo.housesRequiringBarber = 0;
	Data_CityInfo.housesRequiringBathhouse = 0;
	Data_CityInfo.housesRequiringClinic = 0;
	Data_CityInfo.housesRequiringReligion = 0;
}

#define DECAY(svc) \
	if (b->data.house.svc > 0) \
		--b->data.house.svc; \
	else b->data.house.svc = 0

void HouseEvolution_Tick_decayCultureService()
{
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
		if (!BuildingIsInUse(b) || !b->houseSize) {
			continue;
		}
		DECAY(theater);
		DECAY(amphitheaterActor);
		DECAY(amphitheaterGladiator);
		DECAY(colosseumGladiator);
		DECAY(colosseumLion);
		DECAY(hippodrome);
		DECAY(school);
		DECAY(library);
		DECAY(academy);
		DECAY(barber);
		DECAY(clinic);
		DECAY(bathhouse);
		DECAY(hospital);
		DECAY(templeCeres);
		DECAY(templeNeptune);
		DECAY(templeMercury);
		DECAY(templeMars);
		DECAY(templeVenus);
	}
}

void HouseEvolution_Tick_calculateCultureServiceAggregates()
{
    int baseEntertainment = city_culture_coverage_average_entertainment() / 5;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
		if (!BuildingIsInUse(b) || !b->houseSize) {
			continue;
		}

		b->data.house.entertainment = 0;
		b->data.house.education = 0;
		b->data.house.health = 0;
		b->data.house.numGods = 0;

		// entertainment
		b->data.house.entertainment = baseEntertainment;
		if (b->data.house.theater) {
			b->data.house.entertainment += 10;
		}
		if (b->data.house.amphitheaterActor) {
			if (b->data.house.amphitheaterGladiator) {
				b->data.house.entertainment += 15;
			} else {
				b->data.house.entertainment += 10;
			}
		}
		if (b->data.house.colosseumGladiator) {
			if (b->data.house.colosseumLion) {
				b->data.house.entertainment += 25;
			} else {
				b->data.house.entertainment += 15;
			}
		}
		if (b->data.house.hippodrome) {
			b->data.house.entertainment += 30;
		}

		// education
		if (b->data.house.school || b->data.house.library) {
			b->data.house.education = 1;
			if (b->data.house.school && b->data.house.library) {
				b->data.house.education = 2;
				if (b->data.house.academy) {
					b->data.house.education = 3;
				}
			}
		}

		// religion
		if (b->data.house.templeCeres) {
			++b->data.house.numGods;
		}
		if (b->data.house.templeNeptune) {
			++b->data.house.numGods;
		}
		if (b->data.house.templeMercury) {
			++b->data.house.numGods;
		}
		if (b->data.house.templeMars) {
			++b->data.house.numGods;
		}
		if (b->data.house.templeVenus) {
			++b->data.house.numGods;
		}

		// health
		if (b->data.house.clinic) {
			++b->data.house.health;
		}
		if (b->data.house.hospital) {
			++b->data.house.health;
		}
	}
}
