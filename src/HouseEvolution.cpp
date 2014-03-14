#include "HouseOperation.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Model.h"

static int checkEvolveDesirability(int buildingId);
static int hasRequiredGoodsAndServices(int buildingId, int forUpgrade);
static void resetCityInfoServiceRequiredCounters();


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
		HouseOperation_checkMerge(buildingId);
		CHECK_REQUIREMENTS();
		if (status == Evolve) {
			HouseOperation_changeTo(buildingId, Building_HouseLargeTent);
		}
	}
}

static void evolveLargeTent(int buildingId, int *hasExpanded)
{
	if (Data_Buildings[buildingId].housePopulation > 0) {
		HouseOperation_checkMerge(buildingId);
		CHECK_REQUIREMENTS();
		CHECK_DEVOLVE_DELAY();
		if (status == Evolve) {
			HouseOperation_changeTo(buildingId, Building_HouseSmallShack);
		} else if (status == Devolve) {
			HouseOperation_changeTo(buildingId, Building_HouseSmallTent);
		}
	}
}

static void evolveSmallShack(int buildingId, int *hasExpanded)
{
	HouseOperation_checkMerge(buildingId);
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		HouseOperation_changeTo(buildingId, Building_HouseLargeShack);
	} else if (status == Devolve) {
		HouseOperation_changeTo(buildingId, Building_HouseLargeTent);
	}
}

static void evolveLargeShack(int buildingId, int *hasExpanded)
{
	HouseOperation_checkMerge(buildingId);
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		HouseOperation_changeTo(buildingId, Building_HouseSmallHovel);
	} else if (status == Devolve) {
		HouseOperation_changeTo(buildingId, Building_HouseSmallShack);
	}
}

static void evolveSmallHovel(int buildingId, int *hasExpanded)
{
	HouseOperation_checkMerge(buildingId);
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		HouseOperation_changeTo(buildingId, Building_HouseLargeHovel);
	} else if (status == Devolve) {
		HouseOperation_changeTo(buildingId, Building_HouseLargeShack);
	}
}

static void evolveLargeHovel(int buildingId, int *hasExpanded)
{
	HouseOperation_checkMerge(buildingId);
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		HouseOperation_changeTo(buildingId, Building_HouseSmallCasa);
	} else if (status == Devolve) {
		HouseOperation_changeTo(buildingId, Building_HouseSmallHovel);
	}
}

static void evolveSmallCasa(int buildingId, int *hasExpanded)
{
	HouseOperation_checkMerge(buildingId);
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		HouseOperation_changeTo(buildingId, Building_HouseLargeCasa);
	} else if (status == Devolve) {
		HouseOperation_changeTo(buildingId, Building_HouseLargeHovel);
	}
}

static void evolveLargeCasa(int buildingId, int *hasExpanded)
{
	HouseOperation_checkMerge(buildingId);
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		HouseOperation_changeTo(buildingId, Building_HouseSmallInsula);
	} else if (status == Devolve) {
		HouseOperation_changeTo(buildingId, Building_HouseSmallCasa);
	}
}

static void evolveSmallInsula(int buildingId, int *hasExpanded)
{
	HouseOperation_checkMerge(buildingId);
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		HouseOperation_changeTo(buildingId, Building_HouseMediumInsula);
	} else if (status == Devolve) {
		HouseOperation_changeTo(buildingId, Building_HouseLargeCasa);
	}
}

static void evolveMediumInsula(int buildingId, int *hasExpanded)
{
	HouseOperation_checkMerge(buildingId);
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		if (HouseOperation_canExpand(buildingId, 4)) {
			Data_Buildings[buildingId].houseIsMerged = 0;
			HouseOperation_expandToLargeInsula(buildingId);
			*hasExpanded = 1;
			// TODO add recalculating garden graphics to evolveDevolve func
		}
	} else if (status == Devolve) {
		HouseOperation_changeTo(buildingId, Building_HouseSmallInsula);
	}
}

static void evolveLargeInsula(int buildingId, int *hasExpanded)
{
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		HouseOperation_changeTo(buildingId, Building_HouseGrandInsula);
	} else if (status == Devolve) {
		HouseOperation_splitFromLargeInsula(buildingId);
	}
}

static void evolveGrandInsula(int buildingId, int *hasExpanded)
{
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		HouseOperation_changeTo(buildingId, Building_HouseSmallVilla);
	} else if (status == Devolve) {
		HouseOperation_changeTo(buildingId, Building_HouseLargeInsula);
	}
}

static void evolveSmallVilla(int buildingId, int *hasExpanded)
{
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		HouseOperation_changeTo(buildingId, Building_HouseMediumVilla);
	} else if (status == Devolve) {
		HouseOperation_changeTo(buildingId, Building_HouseGrandInsula);
	}
}

static void evolveMediumVilla(int buildingId, int *hasExpanded)
{
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		if (HouseOperation_canExpand(buildingId, 9)) {
			HouseOperation_expandToLargeVilla(buildingId);
			*hasExpanded = 1;
		}
	} else if (status == Devolve) {
		HouseOperation_changeTo(buildingId, Building_HouseSmallVilla);
	}
}

static void evolveLargeVilla(int buildingId, int *hasExpanded)
{
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		HouseOperation_changeTo(buildingId, Building_HouseGrandVilla);
	} else if (status == Devolve) {
		HouseOperation_splitFromLargeVilla(buildingId);
	}
}

static void evolveGrandVilla(int buildingId, int *hasExpanded)
{
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		HouseOperation_changeTo(buildingId, Building_HouseSmallPalace);
	} else if (status == Devolve) {
		HouseOperation_changeTo(buildingId, Building_HouseLargeVilla);
	}
}

static void evolveSmallPalace(int buildingId, int *hasExpanded)
{
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		HouseOperation_changeTo(buildingId, Building_HouseMediumPalace);
	} else if (status == Devolve) {
		HouseOperation_changeTo(buildingId, Building_HouseGrandVilla);
	}
}

static void evolveMediumPalace(int buildingId, int *hasExpanded)
{
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		if (HouseOperation_canExpand(buildingId, 16)) {
			HouseOperation_expandToLargePalace(buildingId);
			*hasExpanded = 1;
		}
	} else if (status == Devolve) {
		HouseOperation_changeTo(buildingId, Building_HouseSmallPalace);
	}
}

static void evolveLargePalace(int buildingId, int *hasExpanded)
{
	CHECK_REQUIREMENTS();
	CHECK_DEVOLVE_DELAY();
	if (status == Evolve) {
		HouseOperation_changeTo(buildingId, Building_HouseLuxuryPalace);
	} else if (status == Devolve) {
		HouseOperation_splitFromLargePalace(buildingId);
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
		HouseOperation_changeTo(buildingId, Building_HouseLargePalace);
	}
}

void HouseEvolution_Tick_evolveAndConsumeResources()
{
	// TODO
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
		if (b->data.house.entertainment) {
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
