
#include "Building.h"
#include "WalkerAction.h"

#include "Data/Building.h"
#include "Data/Settings.h"
#include "Data/Grid.h"
#include "Data/Model.h"
#include "Data/Walker.h"

#define FOR_XY_RADIUS(block) \
	int xMin = x - 2;\
	int yMin = y - 2;\
	int xMax = x + 2;\
	int yMax = y + 2;\
	if (xMin < 0) xMin = 0;\
	if (yMin < 0) yMin = 0;\
	if (xMax >= Data_Settings_Map.width) xMax = Data_Settings_Map.width - 1;\
	if (yMax >= Data_Settings_Map.height) yMax = Data_Settings_Map.height - 1;\
	int gridOffset = GridOffset(xMin, yMin);\
	for (int yy = yMin; yy <= yMax; yy++) {\
		for (int xx = xMin; xx <= xMax; xx++) {\
			int buildingId = Data_Grid_buildingIds[gridOffset];\
			if (buildingId) {\
				block;\
			}\
			++gridOffset;\
		}\
		gridOffset += 162 - (xMax - xMin + 1);\
	}

static int provideEngineerCoverage(int x, int y, int *maxDamageRiskSeen)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].type == Building_Hippodrome) {
			buildingId = Building_getMainBuildingId(buildingId);
		}
		if (Data_Buildings[buildingId].damageRisk > *maxDamageRiskSeen) {
			*maxDamageRiskSeen = Data_Buildings[buildingId].damageRisk;
		}
		Data_Buildings[buildingId].damageRisk = 0;
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			serviced++;
		}
	);
	return serviced;
}

static int providePrefectFireCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].type == Building_Hippodrome) {
			buildingId = Building_getMainBuildingId(buildingId);
		}
		Data_Buildings[buildingId].fireRisk = 0;
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			serviced++;
		}
	);
	return serviced;
}

static int getPrefectCrimeCoverage(int x, int y)
{
	int minHappinessSeen = 100;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].sentiment.houseHappiness < minHappinessSeen) {
			minHappinessSeen = Data_Buildings[buildingId].sentiment.houseHappiness;
		}
	);
	return minHappinessSeen;
}

static int provideTheaterCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.theater = 96;
			serviced++;
		}
	);
	return serviced;
}

static int provideAmphitheaterCoverage(int x, int y, int numShows)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.amphitheaterActor = 96;
			if (numShows == 2) {
				Data_Buildings[buildingId].data.house.amphitheaterGladiator = 96;
			}
			serviced++;
		}
	);
	return serviced;
}

static int provideColosseumCoverage(int x, int y, int numShows)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.colosseumGladiator = 96;
			if (numShows == 2) {
				Data_Buildings[buildingId].data.house.colosseumLion = 96;
			}
			serviced++;
		}
	);
	return serviced;
}

static int provideHippodromeCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.hippodrome = 96;
			serviced++;
		}
	);
	return serviced;
}

static int provideMarketGoods(int marketBuildingId, int x, int y)
{
	int serviced = 0;
	Data_Building *market = &Data_Buildings[marketBuildingId];
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			serviced++;
			Data_Building *house = &Data_Buildings[buildingId];
			int level = house->subtype.houseLevel;
			if (level < 19) {
				level++;
			}
			int maxFoodStocks = 4 * house->houseMaxPopulationSeen;
			int foodTypesStoredMax = 0;
			for (int i = 0; i < 4; i++) {
				if (house->data.house.inventory.all[i] >= maxFoodStocks) {
					foodTypesStoredMax++;
				}
			}
			if (Data_Model_Houses[level].food > foodTypesStoredMax) {
				for (int i = 0; i < 4; i++) {
					if (house->data.house.inventory.all[i] >= maxFoodStocks) {
						continue;
					}
					if (market->data.market.food[i] >= maxFoodStocks) {
						house->data.house.inventory.all[i] += maxFoodStocks;
						market->data.market.food[i] -= maxFoodStocks;
					} else if (market->data.market.food[i]) {
						house->data.house.inventory.all[i] += market->data.market.food[i];
						market->data.market.food[i] = 0;
					}
				}
			}
			if (Data_Model_Houses[level].pottery) {
				market->data.market.potteryDemand = 10;
				int potteryWanted = 8 * Data_Model_Houses[level].pottery - house->data.house.inventory.one.pottery;
				if (market->data.market.pottery && potteryWanted > 0) {
					if (potteryWanted <= market->data.market.pottery) {
						house->data.house.inventory.one.pottery += potteryWanted;
						market->data.market.pottery -= potteryWanted;
					} else {
						house->data.house.inventory.one.pottery += market->data.market.pottery;
						market->data.market.pottery = 0;
					}
				}
			}
			if (Data_Model_Houses[level].furniture) {
				market->data.market.furnitureDemand = 10;
				int furnitureWanted = 4 * Data_Model_Houses[level].furniture - house->data.house.inventory.one.furniture;
				if (market->data.market.furniture && furnitureWanted > 0) {
					if (furnitureWanted <= market->data.market.furniture) {
						house->data.house.inventory.one.furniture += furnitureWanted;
						market->data.market.furniture -= furnitureWanted;
					} else {
						house->data.house.inventory.one.furniture += market->data.market.furniture;
						market->data.market.furniture = 0;
					}
				}
			}
			if (Data_Model_Houses[level].oil) {
				market->data.market.oilDemand = 10;
				int oilWanted = 4 * Data_Model_Houses[level].oil - house->data.house.inventory.one.oil;
				if (market->data.market.oil && oilWanted > 0) {
					if (oilWanted <= market->data.market.oil) {
						house->data.house.inventory.one.oil += oilWanted;
						market->data.market.oil -= oilWanted;
					} else {
						house->data.house.inventory.one.oil += market->data.market.oil;
						market->data.market.oil = 0;
					}
				}
			}
			if (Data_Model_Houses[level].wine) {
				market->data.market.wineDemand = 10;
				int wineWanted = 4 * Data_Model_Houses[level].wine - house->data.house.inventory.one.wine;
				if (market->data.market.wine && wineWanted > 0) {
					if (wineWanted <= market->data.market.wine) {
						house->data.house.inventory.one.wine += wineWanted;
						market->data.market.wine -= wineWanted;
					} else {
						house->data.house.inventory.one.wine += market->data.market.wine;
						market->data.market.wine = 0;
					}
				}
			}
		}
	);
	return serviced;
}

static int provideBathhouseCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.bathhouse = 96;
			serviced++;
		}
	);
	return serviced;
}

static int provideReligionCoverage(int x, int y, int god)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			switch (god) {
				case 0:
					Data_Buildings[buildingId].data.house.templeCeres = 96;
					break;
				case 1:
					Data_Buildings[buildingId].data.house.templeNeptune = 96;
					break;
				case 2:
					Data_Buildings[buildingId].data.house.templeMercury = 96;
					break;
				case 3:
					Data_Buildings[buildingId].data.house.templeMars = 96;
					break;
				case 4:
					Data_Buildings[buildingId].data.house.templeVenus = 96;
					break;
			}
			serviced++;
		}
	);
	return serviced;
}

static int provideSchoolCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.school = 96;
			serviced++;
		}
	);
	return serviced;
}

static int provideAcademyCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.academy = 96;
			serviced++;
		}
	);
	return serviced;
}

static int provideLibraryCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.library = 96;
			serviced++;
		}
	);
	return serviced;
}

static int provideBarberCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.barber = 96;
			serviced++;
		}
	);
	return serviced;
}

static int provideClinicCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.clinic = 96;
			serviced++;
		}
	);
	return serviced;
}

static int provideHospitalCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.hospital = 96;
			serviced++;
		}
	);
	return serviced;
}

static int provideMissionaryCoverage(int x, int y)
{
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].type == Building_NativeHut ||
			Data_Buildings[buildingId].type == Building_NativeMeeting) {
			Data_Buildings[buildingId].sentiment.nativeAnger = 0;
		}
	);
	return 1;
}

static int provideLaborSeekerCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			serviced++;
		}
	);
	return serviced;
}

static int provideTaxCollectorCoverage(int x, int y, char *maxTaxMultiplier)
{
	int serviced = 0;
	FOR_XY_RADIUS(
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			int taxMultiplier = Data_Model_Houses[Data_Buildings[buildingId].subtype.houseLevel].taxMultiplier;
			if (taxMultiplier > *maxTaxMultiplier) {
				*maxTaxMultiplier = taxMultiplier;
			}
			Data_Buildings[buildingId].houseTaxCoverage = 50;
			serviced++;
		}
	);
	return serviced;
}

int Walker_provideServiceCoverage(int walkerId)
{
	int numHousesServiced = 0;
	int x = Data_Walkers[walkerId].x;
	int y = Data_Walkers[walkerId].y;
	int buildingId;
	switch (Data_Walkers[walkerId].type) {
		case Walker_Patrician:
			return 0;
		case Walker_LaborSeeker:
			numHousesServiced = provideLaborSeekerCoverage(x, y);
			break;
		case Walker_TaxCollector:
			numHousesServiced = provideTaxCollectorCoverage(x, y, &Data_Walkers[walkerId].minMaxSeen);
			break;
		case Walker_MarketTrader:
		case Walker_MarketBuyer:
			numHousesServiced = provideMarketGoods(Data_Walkers[walkerId].buildingId, x, y);
			break;
		case Walker_BathhouseWorker:
			numHousesServiced = provideBathhouseCoverage(x, y);
			break;
		case Walker_SchoolChild:
			numHousesServiced = provideSchoolCoverage(x, y);
			break;
		case Walker_Teacher:
			numHousesServiced = provideAcademyCoverage(x, y);
			break;
		case Walker_Librarian:
			numHousesServiced = provideLibraryCoverage(x, y);
			break;
		case Walker_Barber:
			numHousesServiced = provideBarberCoverage(x, y);
			break;
		case Walker_Doctor:
			numHousesServiced = provideClinicCoverage(x, y);
			break;
		case Walker_Surgeon:
			numHousesServiced = provideHospitalCoverage(x, y);
			break;
		case Walker_Missionary:
			numHousesServiced = provideMissionaryCoverage(x, y);
			break;
		case Walker_Priest:
			switch (Data_Buildings[Data_Walkers[walkerId].buildingId].type) {
				case Building_SmallTempleCeres:
				case Building_LargeTempleCeres:
					numHousesServiced = provideReligionCoverage(x, y, 0);
					break;
				case Building_SmallTempleNeptune:
				case Building_LargeTempleNeptune:
					numHousesServiced = provideReligionCoverage(x, y, 1);
					break;
				case Building_SmallTempleMercury:
				case Building_LargeTempleMercury:
					numHousesServiced = provideReligionCoverage(x, y, 2);
					break;
				case Building_SmallTempleMars:
				case Building_LargeTempleMars:
					numHousesServiced = provideReligionCoverage(x, y, 3);
					break;
				case Building_SmallTempleVenus:
				case Building_LargeTempleVenus:
					numHousesServiced = provideReligionCoverage(x, y, 4);
					break;
				default:
					break;
			}
			break;
		case Walker_Actor:
			if (Data_Walkers[walkerId].actionState == WalkerActionState_94_EntertainerAtVenueCreated||
				Data_Walkers[walkerId].actionState == WalkerActionState_95_Entertainer) {
				buildingId = Data_Walkers[walkerId].buildingId;
			} else { // going to venue
				buildingId = Data_Walkers[walkerId].destinationBuildingId;
			}
			if (Data_Buildings[buildingId].type == Building_Theater) {
				numHousesServiced = provideTheaterCoverage(x, y);
			} else if (Data_Buildings[buildingId].type == Building_Amphitheater) {
				numHousesServiced = provideAmphitheaterCoverage(x, y,
					Data_Buildings[buildingId].data.entertainment.days1 ? 2 : 1);
			}
			break;
		case Walker_Gladiator:
			if (Data_Walkers[walkerId].actionState == WalkerActionState_94_EntertainerAtVenueCreated ||
				Data_Walkers[walkerId].actionState == WalkerActionState_95_Entertainer) {
				buildingId = Data_Walkers[walkerId].buildingId;
			} else { // going to venue
				buildingId = Data_Walkers[walkerId].destinationBuildingId;
			}
			if (Data_Buildings[buildingId].type == Building_Amphitheater) {
				numHousesServiced = provideAmphitheaterCoverage(x, y,
					Data_Buildings[buildingId].data.entertainment.days2 ? 2 : 1);
			} else if (Data_Buildings[buildingId].type == Building_Colosseum) {
				numHousesServiced = provideColosseumCoverage(x, y,
					Data_Buildings[buildingId].data.entertainment.days1 ? 2 : 1);
			}
			break;
		case Walker_LionTamer:
			if (Data_Walkers[walkerId].actionState == WalkerActionState_94_EntertainerAtVenueCreated ||
				Data_Walkers[walkerId].actionState == WalkerActionState_95_Entertainer) {
				buildingId = Data_Walkers[walkerId].buildingId;
			} else { // going to venue
				buildingId = Data_Walkers[walkerId].destinationBuildingId;
			}
			numHousesServiced = provideColosseumCoverage(x, y,
				Data_Buildings[buildingId].data.entertainment.days2 ? 2 : 1);
			break;
		case Walker_Charioteer:
			numHousesServiced = provideHippodromeCoverage(x, y);
			break;
		case Walker_Engineer:
			int maxDamage;
			numHousesServiced = provideEngineerCoverage(x, y, &maxDamage);
			if (maxDamage > Data_Walkers[walkerId].minMaxSeen) {
				Data_Walkers[walkerId].minMaxSeen = maxDamage;
			} else if (Data_Walkers[walkerId].minMaxSeen <= 10) {
				Data_Walkers[walkerId].minMaxSeen = 0;
			} else {
				Data_Walkers[walkerId].minMaxSeen -= 10;
			}
			break;
		case Walker_Prefect:
			numHousesServiced = providePrefectFireCoverage(x, y);
			Data_Walkers[walkerId].minMaxSeen = getPrefectCrimeCoverage(x, y);
			break;
		case Walker_Rioter:
			if (WalkerAction_rioterCollapseBuilding(walkerId) == 1) {
				return 1;
			}
			break;
	}
	if (Data_Walkers[walkerId].buildingId) {
		buildingId = Data_Walkers[walkerId].buildingId;
		Data_Buildings[buildingId].housesCovered += numHousesServiced;
		if (Data_Buildings[buildingId].housesCovered > 300) {
			Data_Buildings[buildingId].housesCovered = 300;
		}
	}
	return 0;
}
