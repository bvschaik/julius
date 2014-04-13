#include "BuildingHouse.h"
#include "Routing.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Model.h"

static int checkEvolveDesirability(int buildingId);
static int hasRequiredGoodsAndServices(int buildingId, int forUpgrade);
static void resetCityInfoServiceRequiredCounters();
static void consumeResources(int buildingId);

enum {
	Evolve = 1,
	None = 0,
	Devolve = -1
};

#define CHECK_REQUIREMENTS() \
	int status = checkEvolveDesirability(buildingId);\
	if (!hasRequiredGoodsAndServices(buildingId, 0)) {\
		status = Devolve;\
	} else if (status == Evolve) {\
		status = hasRequiredGoodsAndServices(buildingId, 1);\
	}

#define CHECK_DEVOLVE_DELAY() \
	if (status == Devolve && Data_Buildings[buildingId].data.house.devolveDelay < 2) {\
		++Data_Buildings[buildingId].data.house.devolveDelay;\
		return;\
	}\
	Data_Buildings[buildingId].data.house.devolveDelay = 0;

static void evolveSmallTent(int buildingId, int *hasExpanded)
{
	if (Data_Buildings[buildingId].housePopulation > 0) {
		BuildingHouse_checkMerge(buildingId);
		CHECK_REQUIREMENTS();
		if (status == Evolve) {
			BuildingHouse_changeTo(buildingId, Building_HouseLargeTent);
		}
	}
}

static void evolveLargeTent(int buildingId, int *hasExpanded)
{
	if (Data_Buildings[buildingId].housePopulation > 0) {
		BuildingHouse_checkMerge(buildingId);
		CHECK_REQUIREMENTS();
		CHECK_DEVOLVE_DELAY();
		if (status == Evolve) {
			BuildingHouse_changeTo(buildingId, Building_HouseSmallShack);
		} else if (status == Devolve) {
			BuildingHouse_changeTo(buildingId, Building_HouseSmallTent);
		}
	}
}

static void evolveSmallShack(int buildingId, int *hasExpanded)
{
	BuildingHouse_checkMerge(buildingId);
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseLargeShack);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseLargeTent);
	}
}

static void evolveLargeShack(int buildingId, int *hasExpanded)
{
	BuildingHouse_checkMerge(buildingId);
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseSmallHovel);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseSmallShack);
	}
}

static void evolveSmallHovel(int buildingId, int *hasExpanded)
{
	BuildingHouse_checkMerge(buildingId);
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseLargeHovel);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseLargeShack);
	}
}

static void evolveLargeHovel(int buildingId, int *hasExpanded)
{
	BuildingHouse_checkMerge(buildingId);
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseSmallCasa);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseSmallHovel);
	}
}

static void evolveSmallCasa(int buildingId, int *hasExpanded)
{
	BuildingHouse_checkMerge(buildingId);
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseLargeCasa);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseLargeHovel);
	}
}

static void evolveLargeCasa(int buildingId, int *hasExpanded)
{
	BuildingHouse_checkMerge(buildingId);
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseSmallInsula);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseSmallCasa);
	}
}

static void evolveSmallInsula(int buildingId, int *hasExpanded)
{
	BuildingHouse_checkMerge(buildingId);
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseMediumInsula);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseLargeCasa);
	}
}

static void evolveMediumInsula(int buildingId, int *hasExpanded)
{
	BuildingHouse_checkMerge(buildingId);
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		if (BuildingHouse_canExpand(buildingId, 4)) {
			Data_Buildings[buildingId].houseIsMerged = 0;
			BuildingHouse_expandToLargeInsula(buildingId);
			*hasExpanded = 1;
		}
	} else if (status == Devolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseSmallInsula);
	}
}

static void evolveLargeInsula(int buildingId, int *hasExpanded)
{
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseGrandInsula);
	} else if (status == Devolve) {
		BuildingHouse_devolveFromLargeInsula(buildingId);
	}
}

static void evolveGrandInsula(int buildingId, int *hasExpanded)
{
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseSmallVilla);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseLargeInsula);
	}
}

static void evolveSmallVilla(int buildingId, int *hasExpanded)
{
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseMediumVilla);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseGrandInsula);
	}
}

static void evolveMediumVilla(int buildingId, int *hasExpanded)
{
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		if (BuildingHouse_canExpand(buildingId, 9)) {
			BuildingHouse_expandToLargeVilla(buildingId);
			*hasExpanded = 1;
		}
	} else if (status == Devolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseSmallVilla);
	}
}

static void evolveLargeVilla(int buildingId, int *hasExpanded)
{
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseGrandVilla);
	} else if (status == Devolve) {
		BuildingHouse_devolveFromLargeVilla(buildingId);
	}
}

static void evolveGrandVilla(int buildingId, int *hasExpanded)
{
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseSmallPalace);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseLargeVilla);
	}
}

static void evolveSmallPalace(int buildingId, int *hasExpanded)
{
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseMediumPalace);
	} else if (status == Devolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseGrandVilla);
	}
}

static void evolveMediumPalace(int buildingId, int *hasExpanded)
{
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		if (BuildingHouse_canExpand(buildingId, 16)) {
			BuildingHouse_expandToLargePalace(buildingId);
			*hasExpanded = 1;
		}
	} else if (status == Devolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseSmallPalace);
	}
}

static void evolveLargePalace(int buildingId, int *hasExpanded)
{
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseLuxuryPalace);
	} else if (status == Devolve) {
		BuildingHouse_devolveFromLargePalace(buildingId);
	}
}

static void evolveLuxuryPalace(int buildingId, int *hasExpanded)
{
	int status = checkEvolveDesirability(buildingId);
	if (!hasRequiredGoodsAndServices(buildingId, 0)) {
		status = Devolve;
	}
	CHECK_DEVOLVE_DELAY();
	if (status == Devolve) {
		BuildingHouse_changeTo(buildingId, Building_HouseLargePalace);
	}
}

void HouseEvolution_Tick_evolveAndConsumeResources()
{
	static void (*callbacks[])(int, int*) = {
		evolveSmallTent, evolveLargeTent, evolveSmallShack, evolveLargeShack,
		evolveSmallHovel, evolveLargeHovel, evolveSmallCasa, evolveLargeCasa,
		evolveSmallInsula, evolveMediumInsula, evolveLargeInsula, evolveGrandInsula,
		evolveSmallVilla, evolveMediumVilla, evolveLargeVilla, evolveGrandVilla,
		evolveSmallPalace, evolveMediumPalace, evolveLargePalace, evolveLuxuryPalace
	};
	resetCityInfoServiceRequiredCounters();
	int hasExpanded = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse == 1 &&
			Data_Buildings[i].type >= Building_HouseVacantLot &&
			Data_Buildings[i].type <= Building_HouseLuxuryPalace) {
			BuildingHouse_checkForCorruption(i);
			(*callbacks[Data_Buildings[i].type - 10])(i, &hasExpanded);
			if (Data_CityInfo_Extra.gameTimeDay == 0 || Data_CityInfo_Extra.gameTimeDay == 7) {
				consumeResources(i);
			}
		}
	}
	if (hasExpanded) {
		Routing_determineLandCitizen();
		Routing_determineLandNonCitizen();
	}
}

static int checkEvolveDesirability(int buildingId)
{
	int level = Data_Buildings[buildingId].subtype.houseLevel;
	int evolveDes = Data_Model_Houses[level].evolveDesirability;
	if (level >= 19) {
		evolveDes = 1000;
	}
	int currentDes = Data_Buildings[buildingId].desirability;
	int status;
	if (currentDes <= Data_Model_Houses[level].devolveDesirability) {
		status = Devolve;
	} else if (currentDes >= evolveDes) {
		status = Evolve;
	} else {
		status = None;
	}
	Data_Buildings[buildingId].data.house.evolveTextId = status;
	return status;
}

static int hasRequiredGoodsAndServices(int buildingId, int forUpgrade)
{
	Data_Building *b = &Data_Buildings[buildingId];
	int level = b->subtype.houseLevel;
	if (forUpgrade) {
		++level;
	}
	// water
	int water = Data_Model_Houses[level].water;
	if (!b->hasWaterAccess) {
		if (water >= 2) {
			++Data_CityInfo.housesRequiringFountainToEvolve;
			return 0;
		}
		if (water == 1 && !b->houseHasWellAccess) {
			++Data_CityInfo.housesRequiringWaterToEvolve;
			return 0;
		}
	}
	// entertainment
	int entertainment = Data_Model_Houses[level].entertainment;
	if (b->data.house.entertainment < entertainment) {
		if (b->data.house.entertainment) {
			++Data_CityInfo.housesRequiringMoreEntertainmentToEvolve;
		} else {
			++Data_CityInfo.housesRequiringEntertainmentToEvolve;
		}
		return 0;
	}
	//education
	int education = Data_Model_Houses[level].education;
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
	int religion = Data_Model_Houses[level].religion;
	if (b->data.house.numGods < religion) {
		if (religion == 1) {
			++Data_CityInfo.housesRequiringReligionToEvolve;
		} else if (religion == 2) {
			++Data_CityInfo.housesRequiringMoreReligionToEvolve;
		} else if (religion == 3) {
			++Data_CityInfo.housesRequiringEvenMoreReligionToEvolve;
		}
		return 0;
	}
	if (religion > 0) {
		++Data_CityInfo.housesRequiringReligion;
	}
	// barber
	int barber = Data_Model_Houses[level].barber;
	if (b->data.house.barber < barber) {
		++Data_CityInfo.housesRequiringBarberToEvolve;
		return 0;
	}
	if (barber) {
		++Data_CityInfo.housesRequiringBarber;
	}
	// bathhouse
	int bathhouse = Data_Model_Houses[level].bathhouse;
	if (b->data.house.bathhouse < bathhouse) {
		++Data_CityInfo.housesRequiringBathhouseToEvolve;
		return 0;
	}
	if (bathhouse) {
		++Data_CityInfo.housesRequiringBathhouse;
	}
	// health
	int health = Data_Model_Houses[level].health;
	if (b->data.house.health < health) {
		if (health < 2) {
			++Data_CityInfo.housesRequiringClinicToEvolve;
		} else {
			++Data_CityInfo.housesRequiringHospitalToEvolve;
		}
		return 0;
	}
	if (health) {
		++Data_CityInfo.housesRequiringClinic;
	}
	// food types
	int foodtypesRequired = Data_Model_Houses[level].food;
	int foodtypesAvailable = 0;
	for (int i = 0; i < 4; i++) {
		if (b->data.house.inventory.all[i]) {
			foodtypesAvailable++;
		}
	}
	if (foodtypesAvailable < foodtypesRequired) {
		++Data_CityInfo.housesRequiringFoodToEvolve;
		return 0;
	}
	// goods
	if (b->data.house.inventory.one.pottery < Data_Model_Houses[level].pottery) {
		return 0;
	}
	if (b->data.house.inventory.one.furniture < Data_Model_Houses[level].furniture) {
		return 0;
	}
	if (b->data.house.inventory.one.oil < Data_Model_Houses[level].oil) {
		return 0;
	}
	int wine = Data_Model_Houses[level].wine;
	if (wine && !b->data.house.inventory.one.wine) {
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
	Data_Building *b = &Data_Buildings[buildingId];
	int level = b->subtype.houseLevel;
	int pottery = Data_Model_Houses[level].pottery;
	int furniture = Data_Model_Houses[level].furniture;
	int oil = Data_Model_Houses[level].oil;
	int wine = Data_Model_Houses[level].wine;

	if (pottery) {
		if (pottery > b->data.house.inventory.one.pottery) {
			b->data.house.inventory.one.pottery = 0;
		} else {
			b->data.house.inventory.one.pottery -= pottery;
		}
	}
	if (furniture) {
		if (furniture > b->data.house.inventory.one.furniture) {
			b->data.house.inventory.one.furniture = 0;
		} else {
			b->data.house.inventory.one.furniture -= furniture;
		}
	}
	if (oil) {
		if (oil > b->data.house.inventory.one.oil) {
			b->data.house.inventory.one.oil = 0;
		} else {
			b->data.house.inventory.one.oil -= oil;
		}
	}
	if (wine) {
		if (wine > b->data.house.inventory.one.wine) {
			b->data.house.inventory.one.wine = 0;
		} else {
			b->data.house.inventory.one.wine -= wine;
		}
	}
}

static void resetCityInfoServiceRequiredCounters()
{
	Data_CityInfo.housesRequiringFountainToEvolve = 0;
	Data_CityInfo.housesRequiringWaterToEvolve = 0;
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

#define DECAY(svc) if (Data_Buildings[i].data.house.svc > 0) --Data_Buildings[i].data.house.svc
void HouseEvolution_Tick_decayCultureService()
{
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse != 1 || Data_Buildings[i].houseSize) {
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
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse != 1 || Data_Buildings[i].houseSize) {
			continue;
		}
		Data_Building *b = &Data_Buildings[i];

		b->data.house.entertainment = 0;
		b->data.house.education = 0;
		b->data.house.health = 0;
		b->data.house.numGods = 0;

		// entertainment
		b->data.house.entertainment =
			((Data_CityInfo_CultureCoverage.hippodrome +
			Data_CityInfo_CultureCoverage.colosseum +
			Data_CityInfo_CultureCoverage.amphitheater +
			Data_CityInfo_CultureCoverage.theater) / 4) / 5;
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
	Data_Building *b = &Data_Buildings[buildingId];
	int level = b->subtype.houseLevel;
	
	// this house will devolve soon because...

	// desirability
	if (b->desirability <= Data_Model_Houses[level].devolveDesirability) {
		b->data.house.evolveTextId = 0;
		return;
	}
	// water
	int water = Data_Model_Houses[level].water;
	if (water == 1 && !b->hasWaterAccess && !b->houseHasWellAccess) {
		b->data.house.evolveTextId = 1;
		return;
	}
	if (water == 2 && !b->hasWaterAccess) {
		b->data.house.evolveTextId = 2;
		return;
	}
	// entertainment
	int entertainment = Data_Model_Houses[level].entertainment;
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
	int foodtypesRequired = Data_Model_Houses[level].food;
	int foodtypesAvailable = 0;
	for (int i = 0; i < 4; i++) {
		if (b->data.house.inventory.all[i]) {
			foodtypesAvailable++;
		}
	}
	if (foodtypesAvailable < foodtypesRequired) {
		if (foodtypesRequired == 1) {
			b->data.house.evolveTextId = 9;
		} else if (foodtypesRequired == 2) {
			b->data.house.evolveTextId = 10;
		} else if (foodtypesRequired == 3) {
			b->data.house.evolveTextId = 11;
		}
		return;
	}
	// education
	int education = Data_Model_Houses[level].education;
	if (b->data.house.education < education) {
		if (education == 1) {
			b->data.house.evolveTextId = 14;
		} else if (education == 2) {
			if (b->data.house.school) {
				b->data.house.evolveTextId = 15;
			} else {
				b->data.house.evolveTextId = 16;
			}
		} else if (education == 3) {
			b->data.house.evolveTextId = 17;
		}
		return;
	}
	// bathhouse
	if (b->data.house.bathhouse < Data_Model_Houses[level].bathhouse) {
		b->data.house.evolveTextId = 18;
		return;
	}
	// pottery
	if (b->data.house.inventory.one.pottery < Data_Model_Houses[level].pottery) {
		b->data.house.evolveTextId = 19;
		return;
	}
	// religion
	int religion = Data_Model_Houses[level].religion;
	if (b->data.house.numGods < religion) {
		if (religion == 1) {
			b->data.house.evolveTextId = 20;
		} else if (religion == 2) {
			b->data.house.evolveTextId = 21;
		} else if (religion == 3) {
			b->data.house.evolveTextId = 22;
		}
		return;
	}
	// barber
	if (b->data.house.barber < Data_Model_Houses[level].barber) {
		b->data.house.evolveTextId = 23;
		return;
	}
	// health
	int health = Data_Model_Houses[level].health;
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
	if (b->data.house.inventory.one.oil < Data_Model_Houses[level].oil) {
		b->data.house.evolveTextId = 27;
		return;
	}
	// furniture
	if (b->data.house.inventory.one.furniture < Data_Model_Houses[level].furniture) {
		b->data.house.evolveTextId = 28;
		return;
	}
	// wine
	int wine = Data_Model_Houses[level].wine;
	if (b->data.house.inventory.one.wine < wine) {
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

	++level;
	// desirability
	if (b->desirability < Data_Model_Houses[level-1].evolveDesirability) {
		if (hasBadDesirabilityBuilding) {
			b->data.house.evolveTextId = 62;
		} else {
			b->data.house.evolveTextId = 30;
		}
		return;
	}
	// water
	water = Data_Model_Houses[level].water;
	if (water == 1 && !b->hasWaterAccess && !b->houseHasWellAccess) {
		b->data.house.evolveTextId = 31;
		return;
	}
	if (water == 2 && !b->hasWaterAccess) {
		b->data.house.evolveTextId = 32;
		return;
	}
	// entertainment
	entertainment = Data_Model_Houses[level].entertainment;
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
	foodtypesRequired = Data_Model_Houses[level].food;
	if (foodtypesAvailable < foodtypesRequired) {
		if (foodtypesRequired == 1) {
			b->data.house.evolveTextId = 39;
		} else if (foodtypesRequired == 2) {
			b->data.house.evolveTextId = 40;
		} else if (foodtypesRequired == 3) {
			b->data.house.evolveTextId = 41;
		}
		return;
	}
	// education
	education = Data_Model_Houses[level].education;
	if (b->data.house.education < education) {
		if (education == 1) {
			b->data.house.evolveTextId = 44;
		} else if (education == 2) {
			if (b->data.house.school) {
				b->data.house.evolveTextId = 45;
			} else {
				b->data.house.evolveTextId = 46;
			}
		} else if (education == 3) {
			b->data.house.evolveTextId = 47;
		}
		return;
	}
	// bathhouse
	if (b->data.house.bathhouse < Data_Model_Houses[level].bathhouse) {
		b->data.house.evolveTextId = 48;
		return;
	}
	// pottery
	if (b->data.house.inventory.one.pottery < Data_Model_Houses[level].pottery) {
		b->data.house.evolveTextId = 49;
		return;
	}
	// religion
	religion = Data_Model_Houses[level].religion;
	if (b->data.house.numGods < religion) {
		if (religion == 1) {
			b->data.house.evolveTextId = 50;
		} else if (religion == 2) {
			b->data.house.evolveTextId = 51;
		} else if (religion == 3) {
			b->data.house.evolveTextId = 52;
		}
		return;
	}
	// barber
	if (b->data.house.barber < Data_Model_Houses[level].barber) {
		b->data.house.evolveTextId = 53;
		return;
	}
	// health
	health = Data_Model_Houses[level].health;
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
	if (b->data.house.inventory.one.oil < Data_Model_Houses[level].oil) {
		b->data.house.evolveTextId = 57;
		return;
	}
	// furniture
	if (b->data.house.inventory.one.furniture < Data_Model_Houses[level].furniture) {
		b->data.house.evolveTextId = 58;
		return;
	}
	// wine
	wine = Data_Model_Houses[level].wine;
	if (b->data.house.inventory.one.wine < wine) {
		b->data.house.evolveTextId = 59;
		return;
	}
	if (wine > 1 && Data_CityInfo.resourceWineTypesAvailable < 2) {
		b->data.house.evolveTextId = 66;
		return;
	}
	// house is evolving
	b->data.house.evolveTextId = 61;
	if (b->data.house.noSpaceToExpand) {
		// house would like to evolve but can't
		b->data.house.evolveTextId = 64;
	}
}
