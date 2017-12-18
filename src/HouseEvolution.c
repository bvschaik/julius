#include "BuildingHouse.h"

#include "TerrainGraphics.h"

#include "Data/CityInfo.h"

#include "building/building.h"
#include "building/model.h"
#include "city/culture.h"
#include "game/resource.h"
#include "game/time.h"
#include "map/routing_terrain.h"

static int checkEvolveDesirability(building *b);
static int hasRequiredGoodsAndServices(building *b, int forUpgrade);
static void resetCityInfoServiceRequiredCounters();
static void consumeResources(int buildingId);

enum {
	Evolve = 1,
	None = 0,
	Devolve = -1
};

static int check_requirements(building *b)
{
    int status = checkEvolveDesirability(b);
    if (!hasRequiredGoodsAndServices(b, 0)) {
        status = Devolve;
    } else if (status == Evolve) {
        status = hasRequiredGoodsAndServices(b, 1);
    }
    return status;
}

static int has_devolve_delay(building *b, int status)
{
    if (status == Devolve && b->data.house.devolveDelay < 2) {
        b->data.house.devolveDelay++;
        return 1;
    } else {
        b->data.house.devolveDelay = 0;
        return 0;
    }
}

static void evolveSmallTent(int buildingId, building *b, int *hasExpanded)
{
	if (b->housePopulation > 0) {
		BuildingHouse_checkMerge(b);
		int status = check_requirements(b);
		if (status == Evolve) {
			BuildingHouse_changeTo(b, BUILDING_HOUSE_LARGE_TENT);
		}
	}
}

static void evolveLargeTent(int buildingId, building *b, int *hasExpanded)
{
	if (b->housePopulation > 0) {
		BuildingHouse_checkMerge(b);
		int status = check_requirements(b);
        if (has_devolve_delay(b, status)) {
            return;
        }
		if (status == Evolve) {
			BuildingHouse_changeTo(b, BUILDING_HOUSE_SMALL_SHACK);
		} else if (status == Devolve) {
			BuildingHouse_changeTo(b, BUILDING_HOUSE_SMALL_TENT);
		}
	}
}

static void evolveSmallShack(int buildingId, building *b, int *hasExpanded)
{
	BuildingHouse_checkMerge(b);
	int status = check_requirements(b);
    if (has_devolve_delay(b, status)) {
        return;
    }
	if (status == Evolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_LARGE_SHACK);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_LARGE_TENT);
	}
}

static void evolveLargeShack(int buildingId, building *b, int *hasExpanded)
{
	BuildingHouse_checkMerge(b);
	int status = check_requirements(b);
    if (has_devolve_delay(b, status)) {
        return;
    }
	if (status == Evolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_SMALL_HOVEL);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_SMALL_SHACK);
	}
}

static void evolveSmallHovel(int buildingId, building *b, int *hasExpanded)
{
	BuildingHouse_checkMerge(b);
	int status = check_requirements(b);
    if (has_devolve_delay(b, status)) {
        return;
    }
	if (status == Evolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_LARGE_HOVEL);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_LARGE_SHACK);
	}
}

static void evolveLargeHovel(int buildingId, building *b, int *hasExpanded)
{
	BuildingHouse_checkMerge(b);
	int status = check_requirements(b);
    if (has_devolve_delay(b, status)) {
        return;
    }
	if (status == Evolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_SMALL_CASA);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_SMALL_HOVEL);
	}
}

static void evolveSmallCasa(int buildingId, building *b, int *hasExpanded)
{
	BuildingHouse_checkMerge(b);
	int status = check_requirements(b);
    if (has_devolve_delay(b, status)) {
        return;
    }
	if (status == Evolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_LARGE_CASA);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_LARGE_HOVEL);
	}
}

static void evolveLargeCasa(int buildingId, building *b, int *hasExpanded)
{
	BuildingHouse_checkMerge(b);
	int status = check_requirements(b);
    if (has_devolve_delay(b, status)) {
        return;
    }
	if (status == Evolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_SMALL_INSULA);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_SMALL_CASA);
	}
}

static void evolveSmallInsula(int buildingId, building *b, int *hasExpanded)
{
	BuildingHouse_checkMerge(b);
	int status = check_requirements(b);
    if (has_devolve_delay(b, status)) {
        return;
    }
	if (status == Evolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_MEDIUM_INSULA);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_LARGE_CASA);
	}
}

static void evolveMediumInsula(int buildingId, building *b, int *hasExpanded)
{
	BuildingHouse_checkMerge(b);
	int status = check_requirements(b);
    if (has_devolve_delay(b, status)) {
        return;
    }
	if (status == Evolve) {
		if (BuildingHouse_canExpand(b, 4)) {
			b->houseIsMerged = 0;
			BuildingHouse_expandToLargeInsula(b);
			*hasExpanded = 1;
			TerrainGraphics_updateAllGardens();
		}
	} else if (status == Devolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_SMALL_INSULA);
	}
}

static void evolveLargeInsula(int buildingId, building *b, int *hasExpanded)
{
	int status = check_requirements(b);
    if (has_devolve_delay(b, status)) {
        return;
    }
	if (status == Evolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_GRAND_INSULA);
	} else if (status == Devolve) {
		BuildingHouse_devolveFromLargeInsula(b);
	}
}

static void evolveGrandInsula(int buildingId, building *b, int *hasExpanded)
{
	int status = check_requirements(b);
    if (has_devolve_delay(b, status)) {
        return;
    }
	if (status == Evolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_SMALL_VILLA);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_LARGE_INSULA);
	}
}

static void evolveSmallVilla(int buildingId, building *b, int *hasExpanded)
{
	int status = check_requirements(b);
    if (has_devolve_delay(b, status)) {
        return;
    }
	if (status == Evolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_MEDIUM_VILLA);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_GRAND_INSULA);
	}
}

static void evolveMediumVilla(int buildingId, building *b, int *hasExpanded)
{
	int status = check_requirements(b);
    if (has_devolve_delay(b, status)) {
        return;
    }
	if (status == Evolve) {
		if (BuildingHouse_canExpand(b, 9)) {
			BuildingHouse_expandToLargeVilla(b);
			*hasExpanded = 1;
			TerrainGraphics_updateAllGardens();
		}
	} else if (status == Devolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_SMALL_VILLA);
	}
}

static void evolveLargeVilla(int buildingId, building *b, int *hasExpanded)
{
	int status = check_requirements(b);
    if (has_devolve_delay(b, status)) {
        return;
    }
	if (status == Evolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_GRAND_VILLA);
	} else if (status == Devolve) {
		BuildingHouse_devolveFromLargeVilla(b);
	}
}

static void evolveGrandVilla(int buildingId, building *b, int *hasExpanded)
{
	int status = check_requirements(b);
    if (has_devolve_delay(b, status)) {
        return;
    }
	if (status == Evolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_SMALL_PALACE);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_LARGE_VILLA);
	}
}

static void evolveSmallPalace(int buildingId, building *b, int *hasExpanded)
{
	int status = check_requirements(b);
    if (has_devolve_delay(b, status)) {
        return;
    }
	if (status == Evolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_MEDIUM_PALACE);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_GRAND_VILLA);
	}
}

static void evolveMediumPalace(int buildingId, building *b, int *hasExpanded)
{
	int status = check_requirements(b);
    if (has_devolve_delay(b, status)) {
        return;
    }
	if (status == Evolve) {
		if (BuildingHouse_canExpand(b, 16)) {
			BuildingHouse_expandToLargePalace(b);
			*hasExpanded = 1;
			TerrainGraphics_updateAllGardens();
		}
	} else if (status == Devolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_SMALL_PALACE);
	}
}

static void evolveLargePalace(int buildingId, building *b, int *hasExpanded)
{
	int status = check_requirements(b);
    if (has_devolve_delay(b, status)) {
        return;
    }
	if (status == Evolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_LUXURY_PALACE);
	} else if (status == Devolve) {
		BuildingHouse_devolveFromLargePalace(b);
	}
}

static void evolveLuxuryPalace(int buildingId, building *b, int *hasExpanded)
{
	int status = checkEvolveDesirability(b);
	if (!hasRequiredGoodsAndServices(b, 0)) {
		status = Devolve;
	}
    if (has_devolve_delay(b, status)) {
        return;
    }
	if (status == Devolve) {
		BuildingHouse_changeTo(b, BUILDING_HOUSE_LARGE_PALACE);
	}
}

static void (*const callbacks[])(int, building *, int*) = {
	evolveSmallTent, evolveLargeTent, evolveSmallShack, evolveLargeShack,
	evolveSmallHovel, evolveLargeHovel, evolveSmallCasa, evolveLargeCasa,
	evolveSmallInsula, evolveMediumInsula, evolveLargeInsula, evolveGrandInsula,
	evolveSmallVilla, evolveMediumVilla, evolveLargeVilla, evolveGrandVilla,
	evolveSmallPalace, evolveMediumPalace, evolveLargePalace, evolveLuxuryPalace
};
void HouseEvolution_Tick_evolveAndConsumeResources()
{
	resetCityInfoServiceRequiredCounters();
	int hasExpanded = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
		if (BuildingIsInUse(i) && BuildingIsHouse(b->type)) {
			BuildingHouse_checkForCorruption(b);
			(*callbacks[b->type - 10])(i, b, &hasExpanded);
			if (game_time_day() == 0 || game_time_day() == 7) {
				consumeResources(i);
			}
		}
	}
	if (hasExpanded) {
		map_routing_update_land();
	}
}

static int checkEvolveDesirability(building *b)
{
	int level = b->subtype.houseLevel;
    const model_house *model = model_get_house(level);
	int evolveDes = model->evolve_desirability;
	if (level >= HOUSE_LUXURY_PALACE) {
		evolveDes = 1000;
	}
	int currentDes = b->desirability;
	int status;
	if (currentDes <= model->devolve_desirability) {
		status = Devolve;
	} else if (currentDes >= evolveDes) {
		status = Evolve;
	} else {
		status = None;
	}
	b->data.house.evolveTextId = status; // BUG? -1 in an unsigned char?
	return status;
}

static int hasRequiredGoodsAndServices(building *b, int forUpgrade)
{
	int level = b->subtype.houseLevel;
	if (forUpgrade) {
		++level;
	}
	const model_house *model = model_get_house(level);
	// water
	int water = model->water;
	if (!b->hasWaterAccess) {
		if (water >= 2) {
			++Data_CityInfo.housesRequiringFountainToEvolve;
			return 0;
		}
		if (water == 1 && !b->hasWellAccess) {
			++Data_CityInfo.housesRequiringWellToEvolve;
			return 0;
		}
	}
	// entertainment
	int entertainment = model->entertainment;
	if (b->data.house.entertainment < entertainment) {
		if (b->data.house.entertainment) {
			++Data_CityInfo.housesRequiringMoreEntertainmentToEvolve;
		} else {
			++Data_CityInfo.housesRequiringEntertainmentToEvolve;
		}
		return 0;
	}
	// education
	int education = model->education;
	if (b->data.house.education < education) {
		if (b->data.house.education) {
			++Data_CityInfo.housesRequiringMoreEducationToEvolve;
		} else {
			++Data_CityInfo.housesRequiringEducationToEvolve;
		}
		return 0;
	}
	if (education == 2) {
		++Data_CityInfo.housesRequiringSchool;
		++Data_CityInfo.housesRequiringLibrary;
	} else if (education == 1) {
		++Data_CityInfo.housesRequiringSchool;
	}
	// religion
	int religion = model->religion;
	if (b->data.house.numGods < religion) {
		if (religion == 1) {
			++Data_CityInfo.housesRequiringReligionToEvolve;
			return 0;
		} else if (religion == 2) {
			++Data_CityInfo.housesRequiringMoreReligionToEvolve;
			return 0;
		} else if (religion == 3) {
			++Data_CityInfo.housesRequiringEvenMoreReligionToEvolve;
			return 0;
		}
	} else if (religion > 0) {
		++Data_CityInfo.housesRequiringReligion;
	}
	// barber
	int barber = model->barber;
	if (b->data.house.barber < barber) {
		++Data_CityInfo.housesRequiringBarberToEvolve;
		return 0;
	}
	if (barber == 1) {
		++Data_CityInfo.housesRequiringBarber;
	}
	// bathhouse
	int bathhouse = model->bathhouse;
	if (b->data.house.bathhouse < bathhouse) {
		++Data_CityInfo.housesRequiringBathhouseToEvolve;
		return 0;
	}
	if (bathhouse == 1) {
		++Data_CityInfo.housesRequiringBathhouse;
	}
	// health
	int health = model->health;
	if (b->data.house.health < health) {
		if (health < 2) {
			++Data_CityInfo.housesRequiringClinicToEvolve;
		} else {
			++Data_CityInfo.housesRequiringHospitalToEvolve;
		}
		return 0;
	}
	if (health >= 1) {
		++Data_CityInfo.housesRequiringClinic;
	}
	// food types
	int foodtypesRequired = model->food_types;
	int foodtypesAvailable = 0;
	for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
		if (b->data.house.inventory[i]) {
			foodtypesAvailable++;
		}
	}
	if (foodtypesAvailable < foodtypesRequired) {
		++Data_CityInfo.housesRequiringFoodToEvolve;
		return 0;
	}
	// goods
	if (b->data.house.inventory[INVENTORY_POTTERY] < model->pottery) {
		return 0;
	}
	if (b->data.house.inventory[INVENTORY_OIL] < model->oil) {
		return 0;
	}
	if (b->data.house.inventory[INVENTORY_FURNITURE] < model->furniture) {
		return 0;
	}
	int wine = model->wine;
	if (wine && b->data.house.inventory[INVENTORY_WINE] <= 0) {
		return 0;
	}
	if (wine >= 2 && Data_CityInfo.resourceWineTypesAvailable < 2) {
		++Data_CityInfo.housesRequiringSecondWineToEvolve;
		return 0;
	}
	return 1;
}

static void consumeResources(int buildingId)
{
	building *b = building_get(buildingId);
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
		if (!BuildingIsInUse(i) || !b->houseSize) {
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
		if (!BuildingIsInUse(i) || !b->houseSize) {
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

void HouseEvolution_determineEvolveText(int buildingId, int hasBadDesirabilityBuilding)
{
	building *b = building_get(buildingId);
	int level = b->subtype.houseLevel;
	
	// this house will devolve soon because...

    const model_house *model = model_get_house(level);
	// desirability
	if (b->desirability <= model->devolve_desirability) {
		b->data.house.evolveTextId = 0;
		return;
	}
	// water
	int water = model->water;
	if (water == 1 && !b->hasWaterAccess && !b->hasWellAccess) {
		b->data.house.evolveTextId = 1;
		return;
	}
	if (water == 2 && !b->hasWaterAccess) {
		b->data.house.evolveTextId = 2;
		return;
	}
	// entertainment
	int entertainment = model->entertainment;
	if (b->data.house.entertainment < entertainment) {
		if (!b->data.house.entertainment) {
			b->data.house.evolveTextId = 3;
		} else if (entertainment < 10) {
			b->data.house.evolveTextId = 4;
		} else if (entertainment < 25) {
			b->data.house.evolveTextId = 5;
		} else if (entertainment < 50) {
			b->data.house.evolveTextId = 6;
		} else if (entertainment < 80) {
			b->data.house.evolveTextId = 7;
		} else {
			b->data.house.evolveTextId = 8;
		}
		return;
	}
	// food types
	int foodtypesRequired = model->food;
	int foodtypesAvailable = 0;
	for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
		if (b->data.house.inventory[i]) {
			foodtypesAvailable++;
		}
	}
	if (foodtypesAvailable < foodtypesRequired) {
		if (foodtypesRequired == 1) {
			b->data.house.evolveTextId = 9;
			return;
		} else if (foodtypesRequired == 2) {
			b->data.house.evolveTextId = 10;
			return;
		} else if (foodtypesRequired == 3) {
			b->data.house.evolveTextId = 11;
			return;
		}
	}
	// education
	int education = model->education;
	if (b->data.house.education < education) {
		if (education == 1) {
			b->data.house.evolveTextId = 14;
			return;
		} else if (education == 2) {
			if (b->data.house.school) {
				b->data.house.evolveTextId = 15;
				return;
			} else if (b->data.house.library) {
				b->data.house.evolveTextId = 16;
				return;
			}
		} else if (education == 3) {
			b->data.house.evolveTextId = 17;
			return;
		}
	}
	// bathhouse
	if (b->data.house.bathhouse < model->bathhouse) {
		b->data.house.evolveTextId = 18;
		return;
	}
	// pottery
	if (b->data.house.inventory[INVENTORY_POTTERY] < model->pottery) {
		b->data.house.evolveTextId = 19;
		return;
	}
	// religion
	int religion = model->religion;
	if (b->data.house.numGods < religion) {
		if (religion == 1) {
			b->data.house.evolveTextId = 20;
			return;
		} else if (religion == 2) {
			b->data.house.evolveTextId = 21;
			return;
		} else if (religion == 3) {
			b->data.house.evolveTextId = 22;
			return;
		}
	}
	// barber
	if (b->data.house.barber < model->barber) {
		b->data.house.evolveTextId = 23;
		return;
	}
	// health
	int health = model->health;
	if (b->data.house.health < health) {
		if (health == 1) {
			b->data.house.evolveTextId = 24;
		} else if (b->data.house.clinic) {
			b->data.house.evolveTextId = 25;
		} else {
			b->data.house.evolveTextId = 26;
		}
		return;
	}
	// oil
	if (b->data.house.inventory[INVENTORY_OIL] < model->oil) {
		b->data.house.evolveTextId = 27;
		return;
	}
	// furniture
	if (b->data.house.inventory[INVENTORY_FURNITURE] < model->furniture) {
		b->data.house.evolveTextId = 28;
		return;
	}
	// wine
	int wine = model->wine;
	if (b->data.house.inventory[INVENTORY_WINE] < wine) {
		b->data.house.evolveTextId = 29;
		return;
	}
	if (wine > 1 && Data_CityInfo.resourceWineTypesAvailable < 2) {
		b->data.house.evolveTextId = 65;
		return;
	}
	if (level >= 19) { // max level!
		b->data.house.evolveTextId = 60;
		return;
	}

	// this house will evolve if ...

	// desirability
	if (b->desirability < model->evolve_desirability) {
		if (hasBadDesirabilityBuilding) {
			b->data.house.evolveTextId = 62;
		} else {
			b->data.house.evolveTextId = 30;
		}
		return;
	}
	model = model_get_house(++level);
	// water
	water = model->water;
	if (water == 1 && !b->hasWaterAccess && !b->hasWellAccess) {
		b->data.house.evolveTextId = 31;
		return;
	}
	if (water == 2 && !b->hasWaterAccess) {
		b->data.house.evolveTextId = 32;
		return;
	}
	// entertainment
	entertainment = model->entertainment;
	if (b->data.house.entertainment < entertainment) {
		if (!b->data.house.entertainment) {
			b->data.house.evolveTextId = 33;
		} else if (entertainment < 10) {
			b->data.house.evolveTextId = 34;
		} else if (entertainment < 25) {
			b->data.house.evolveTextId = 35;
		} else if (entertainment < 50) {
			b->data.house.evolveTextId = 36;
		} else if (entertainment < 80) {
			b->data.house.evolveTextId = 37;
		} else {
			b->data.house.evolveTextId = 38;
		}
		return;
	}
	// food types
	foodtypesRequired = model->food_types;
	if (foodtypesAvailable < foodtypesRequired) {
		if (foodtypesRequired == 1) {
			b->data.house.evolveTextId = 39;
			return;
		} else if (foodtypesRequired == 2) {
			b->data.house.evolveTextId = 40;
			return;
		} else if (foodtypesRequired == 3) {
			b->data.house.evolveTextId = 41;
			return;
		}
	}
	// education
	education = model->education;
	if (b->data.house.education < education) {
		if (education == 1) {
			b->data.house.evolveTextId = 44;
			return;
		} else if (education == 2) {
			if (b->data.house.school) {
				b->data.house.evolveTextId = 45;
				return;
			} else if (b->data.house.library) {
				b->data.house.evolveTextId = 46;
				return;
			}
		} else if (education == 3) {
			b->data.house.evolveTextId = 47;
			return;
		}
	}
	// bathhouse
	if (b->data.house.bathhouse < model->bathhouse) {
		b->data.house.evolveTextId = 48;
		return;
	}
	// pottery
	if (b->data.house.inventory[INVENTORY_POTTERY] < model->pottery) {
		b->data.house.evolveTextId = 49;
		return;
	}
	// religion
	religion = model->religion;
	if (b->data.house.numGods < religion) {
		if (religion == 1) {
			b->data.house.evolveTextId = 50;
			return;
		} else if (religion == 2) {
			b->data.house.evolveTextId = 51;
			return;
		} else if (religion == 3) {
			b->data.house.evolveTextId = 52;
			return;
		}
	}
	// barber
	if (b->data.house.barber < model->barber) {
		b->data.house.evolveTextId = 53;
		return;
	}
	// health
	health = model->health;
	if (b->data.house.health < health) {
		if (health == 1) {
			b->data.house.evolveTextId = 54;
		} else if (b->data.house.clinic) {
			b->data.house.evolveTextId = 55;
		} else {
			b->data.house.evolveTextId = 56;
		}
		return;
	}
	// oil
	if (b->data.house.inventory[INVENTORY_OIL] < model->oil) {
		b->data.house.evolveTextId = 57;
		return;
	}
	// furniture
	if (b->data.house.inventory[INVENTORY_FURNITURE] < model->furniture) {
		b->data.house.evolveTextId = 58;
		return;
	}
	// wine
	wine = model->wine;
	if (b->data.house.inventory[INVENTORY_WINE] < wine) {
		b->data.house.evolveTextId = 59;
		return;
	}
	if (wine > 1 && Data_CityInfo.resourceWineTypesAvailable < 2) {
		b->data.house.evolveTextId = 66;
		return;
	}
	// house is evolving
	b->data.house.evolveTextId = 61;
	if (b->data.house.noSpaceToExpand == 1) {
		// house would like to evolve but can't
		b->data.house.evolveTextId = 64;
	}
}
