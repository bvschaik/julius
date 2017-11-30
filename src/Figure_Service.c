#include "Figure.h"

#include "Building.h"
#include "FigureAction.h"

#include "Data/Building.h"
#include "Data/Grid.h"
#include "Data/Figure.h"
#include "Data/State.h"

#include "building/model.h"
#include "city/culture.h"
#include "figure/type.h"
#include "game/resource.h"

#define FOR_XY_RADIUS \
	int xMin = x - 2;\
	int yMin = y - 2;\
	int xMax = x + 2;\
	int yMax = y + 2;\
	Bound2ToMap(xMin, yMin, xMax, yMax);\
	int gridOffset = GridOffset(xMin, yMin);\
	for (int yy = yMin; yy <= yMax; yy++) {\
		for (int xx = xMin; xx <= xMax; xx++) {\
			int buildingId = Data_Grid_buildingIds[gridOffset];\
			if (buildingId) {

#define END_FOR_XY_RADIUS \
			}\
			++gridOffset;\
		}\
		gridOffset += 162 - (xMax - xMin + 1);\
	}

static int provideEngineerCoverage(int x, int y, int *maxDamageRiskSeen)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].type == BUILDING_HIPPODROME) {
			buildingId = Building_getMainBuildingId(buildingId);
		}
		if (Data_Buildings[buildingId].damageRisk > *maxDamageRiskSeen) {
			*maxDamageRiskSeen = Data_Buildings[buildingId].damageRisk;
		}
		Data_Buildings[buildingId].damageRisk = 0;
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int providePrefectFireCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].type == BUILDING_HIPPODROME) {
			buildingId = Building_getMainBuildingId(buildingId);
		}
		Data_Buildings[buildingId].fireRisk = 0;
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int getPrefectCrimeCoverage(int x, int y)
{
	int minHappinessSeen = 100;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].sentiment.houseHappiness < minHappinessSeen) {
			minHappinessSeen = Data_Buildings[buildingId].sentiment.houseHappiness;
		}
	} END_FOR_XY_RADIUS;
	return minHappinessSeen;
}

static int provideTheaterCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.theater = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideAmphitheaterCoverage(int x, int y, int numShows)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.amphitheaterActor = 96;
			if (numShows == 2) {
				Data_Buildings[buildingId].data.house.amphitheaterGladiator = 96;
			}
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideColosseumCoverage(int x, int y, int numShows)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.colosseumGladiator = 96;
			if (numShows == 2) {
				Data_Buildings[buildingId].data.house.colosseumLion = 96;
			}
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideHippodromeCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.hippodrome = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideMarketGoods(int marketBuildingId, int x, int y)
{
	int serviced = 0;
	struct Data_Building *market = &Data_Buildings[marketBuildingId];
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			serviced++;
			struct Data_Building *house = &Data_Buildings[buildingId];
			int level = house->subtype.houseLevel;
			if (level < HOUSE_LUXURY_PALACE) {
				level++;
			}
			int maxFoodStocks = 4 * house->houseMaxPopulationSeen;
			int foodTypesStoredMax = 0;
			for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
				if (house->data.house.inventory[i] >= maxFoodStocks) {
					foodTypesStoredMax++;
				}
			}
			const model_house *model = model_get_house(level);
			if (model->food_types > foodTypesStoredMax) {
				for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
					if (house->data.house.inventory[i] >= maxFoodStocks) {
						continue;
					}
					if (market->data.market.inventory[i] >= maxFoodStocks) {
						house->data.house.inventory[i] += maxFoodStocks;
						market->data.market.inventory[i] -= maxFoodStocks;
						break;
					} else if (market->data.market.inventory[i]) {
						house->data.house.inventory[i] += market->data.market.inventory[i];
						market->data.market.inventory[i] = 0;
						break;
					}
				}
			}
			if (model->pottery) {
				market->data.market.potteryDemand = 10;
				int potteryWanted = 8 * model->pottery - house->data.house.inventory[INVENTORY_POTTERY];
				if (market->data.market.inventory[INVENTORY_POTTERY] > 0 && potteryWanted > 0) {
					if (potteryWanted <= market->data.market.inventory[INVENTORY_POTTERY]) {
						house->data.house.inventory[INVENTORY_POTTERY] += potteryWanted;
						market->data.market.inventory[INVENTORY_POTTERY] -= potteryWanted;
					} else {
						house->data.house.inventory[INVENTORY_POTTERY] += market->data.market.inventory[INVENTORY_POTTERY];
						market->data.market.inventory[INVENTORY_POTTERY] = 0;
					}
				}
			}
			if (model->furniture) {
				market->data.market.furnitureDemand = 10;
				int furnitureWanted = 4 * model->furniture - house->data.house.inventory[INVENTORY_FURNITURE];
				if (market->data.market.inventory[INVENTORY_FURNITURE] > 0 && furnitureWanted > 0) {
					if (furnitureWanted <= market->data.market.inventory[INVENTORY_FURNITURE]) {
						house->data.house.inventory[INVENTORY_FURNITURE] += furnitureWanted;
						market->data.market.inventory[INVENTORY_FURNITURE] -= furnitureWanted;
					} else {
						house->data.house.inventory[INVENTORY_FURNITURE] += market->data.market.inventory[INVENTORY_FURNITURE];
						market->data.market.inventory[INVENTORY_FURNITURE] = 0;
					}
				}
			}
			if (model->oil) {
				market->data.market.oilDemand = 10;
				int oilWanted = 4 * model->oil - house->data.house.inventory[INVENTORY_OIL];
				if (market->data.market.inventory[INVENTORY_OIL] > 0 && oilWanted > 0) {
					if (oilWanted <= market->data.market.inventory[INVENTORY_OIL]) {
						house->data.house.inventory[INVENTORY_OIL] += oilWanted;
						market->data.market.inventory[INVENTORY_OIL] -= oilWanted;
					} else {
						house->data.house.inventory[INVENTORY_OIL] += market->data.market.inventory[INVENTORY_OIL];
						market->data.market.inventory[INVENTORY_OIL] = 0;
					}
				}
			}
			if (model->wine) {
				market->data.market.wineDemand = 10;
				int wineWanted = 4 * model->wine - house->data.house.inventory[INVENTORY_WINE];
				if (market->data.market.inventory[INVENTORY_WINE] > 0 && wineWanted > 0) {
					if (wineWanted <= market->data.market.inventory[INVENTORY_WINE]) {
						house->data.house.inventory[INVENTORY_WINE] += wineWanted;
						market->data.market.inventory[INVENTORY_WINE] -= wineWanted;
					} else {
						house->data.house.inventory[INVENTORY_WINE] += market->data.market.inventory[INVENTORY_WINE];
						market->data.market.inventory[INVENTORY_WINE] = 0;
					}
				}
			}
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideBathhouseCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.bathhouse = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideReligionCoverage(int x, int y, int god)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			switch (god) {
				case GOD_CERES:
					Data_Buildings[buildingId].data.house.templeCeres = 96;
					break;
				case GOD_NEPTUNE:
					Data_Buildings[buildingId].data.house.templeNeptune = 96;
					break;
				case GOD_MERCURY:
					Data_Buildings[buildingId].data.house.templeMercury = 96;
					break;
				case GOD_MARS:
					Data_Buildings[buildingId].data.house.templeMars = 96;
					break;
				case GOD_VENUS:
					Data_Buildings[buildingId].data.house.templeVenus = 96;
					break;
			}
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideSchoolCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.school = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideAcademyCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.academy = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideLibraryCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.library = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideBarberCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.barber = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideClinicCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.clinic = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideHospitalCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			Data_Buildings[buildingId].data.house.hospital = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideMissionaryCoverage(int x, int y)
{
	int xMin = x - 4;
	int yMin = y - 4;
	int xMax = x + 4;
	int yMax = y + 4;
	Bound2ToMap(xMin, yMin, xMax, yMax);
	int gridOffset = GridOffset(xMin, yMin);
	for (int yy = yMin; yy <= yMax; yy++) {
		for (int xx = xMin; xx <= xMax; xx++) {
			int buildingId = Data_Grid_buildingIds[gridOffset];
			if (buildingId) {
				if (Data_Buildings[buildingId].type == BUILDING_NATIVE_HUT ||
					Data_Buildings[buildingId].type == BUILDING_NATIVE_MEETING) {
					Data_Buildings[buildingId].sentiment.nativeAnger = 0;
				}
			}
			++gridOffset;
		}
		gridOffset += 162 - (xMax - xMin + 1);
	}
	return 1;
}

static int provideLaborSeekerCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideTaxCollectorCoverage(int x, int y, unsigned char *maxTaxMultiplier)
{
	int serviced = 0;
	*maxTaxMultiplier = 0;
	FOR_XY_RADIUS {
		if (Data_Buildings[buildingId].houseSize && Data_Buildings[buildingId].housePopulation > 0) {
			int taxMultiplier = model_get_house(Data_Buildings[buildingId].subtype.houseLevel)->tax_multiplier;
			if (taxMultiplier > *maxTaxMultiplier) {
				*maxTaxMultiplier = taxMultiplier;
			}
			Data_Buildings[buildingId].houseTaxCoverage = 50;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

int Figure_provideServiceCoverage(figure *f)
{
	int numHousesServiced = 0;
	int x = f->x;
	int y = f->y;
	int buildingId;
	switch (f->type) {
		case FIGURE_PATRICIAN:
			return 0;
		case FIGURE_LABOR_SEEKER:
			numHousesServiced = provideLaborSeekerCoverage(x, y);
			break;
		case FIGURE_TAX_COLLECTOR:
			numHousesServiced = provideTaxCollectorCoverage(x, y, &f->minMaxSeen);
			break;
		case FIGURE_MARKET_TRADER:
		case FIGURE_MARKET_BUYER:
			numHousesServiced = provideMarketGoods(f->buildingId, x, y);
			break;
		case FIGURE_BATHHOUSE_WORKER:
			numHousesServiced = provideBathhouseCoverage(x, y);
			break;
		case FIGURE_SCHOOL_CHILD:
			numHousesServiced = provideSchoolCoverage(x, y);
			break;
		case FIGURE_TEACHER:
			numHousesServiced = provideAcademyCoverage(x, y);
			break;
		case FIGURE_LIBRARIAN:
			numHousesServiced = provideLibraryCoverage(x, y);
			break;
		case FIGURE_BARBER:
			numHousesServiced = provideBarberCoverage(x, y);
			break;
		case FIGURE_DOCTOR:
			numHousesServiced = provideClinicCoverage(x, y);
			break;
		case FIGURE_SURGEON:
			numHousesServiced = provideHospitalCoverage(x, y);
			break;
		case FIGURE_MISSIONARY:
			numHousesServiced = provideMissionaryCoverage(x, y);
			break;
		case FIGURE_PRIEST:
			switch (Data_Buildings[f->buildingId].type) {
				case BUILDING_SMALL_TEMPLE_CERES:
				case BUILDING_LARGE_TEMPLE_CERES:
					numHousesServiced = provideReligionCoverage(x, y, GOD_CERES);
					break;
				case BUILDING_SMALL_TEMPLE_NEPTUNE:
				case BUILDING_LARGE_TEMPLE_NEPTUNE:
					numHousesServiced = provideReligionCoverage(x, y, GOD_NEPTUNE);
					break;
				case BUILDING_SMALL_TEMPLE_MERCURY:
				case BUILDING_LARGE_TEMPLE_MERCURY:
					numHousesServiced = provideReligionCoverage(x, y, GOD_MERCURY);
					break;
				case BUILDING_SMALL_TEMPLE_MARS:
				case BUILDING_LARGE_TEMPLE_MARS:
					numHousesServiced = provideReligionCoverage(x, y, GOD_MARS);
					break;
				case BUILDING_SMALL_TEMPLE_VENUS:
				case BUILDING_LARGE_TEMPLE_VENUS:
					numHousesServiced = provideReligionCoverage(x, y, GOD_VENUS);
					break;
				default:
					break;
			}
			break;
		case FIGURE_ACTOR:
			if (f->actionState == FigureActionState_94_EntertainerRoaming ||
				f->actionState == FigureActionState_95_EntertainerReturning) {
				buildingId = f->buildingId;
			} else { // going to venue
				buildingId = f->destinationBuildingId;
			}
			if (Data_Buildings[buildingId].type == BUILDING_THEATER) {
				numHousesServiced = provideTheaterCoverage(x, y);
			} else if (Data_Buildings[buildingId].type == BUILDING_AMPHITHEATER) {
				numHousesServiced = provideAmphitheaterCoverage(x, y,
					Data_Buildings[buildingId].data.entertainment.days1 ? 2 : 1);
			}
			break;
		case FIGURE_GLADIATOR:
			if (f->actionState == FigureActionState_94_EntertainerRoaming ||
				f->actionState == FigureActionState_95_EntertainerReturning) {
				buildingId = f->buildingId;
			} else { // going to venue
				buildingId = f->destinationBuildingId;
			}
			if (Data_Buildings[buildingId].type == BUILDING_AMPHITHEATER) {
				numHousesServiced = provideAmphitheaterCoverage(x, y,
					Data_Buildings[buildingId].data.entertainment.days2 ? 2 : 1);
			} else if (Data_Buildings[buildingId].type == BUILDING_COLOSSEUM) {
				numHousesServiced = provideColosseumCoverage(x, y,
					Data_Buildings[buildingId].data.entertainment.days1 ? 2 : 1);
			}
			break;
		case FIGURE_LION_TAMER:
			if (f->actionState == FigureActionState_94_EntertainerRoaming ||
				f->actionState == FigureActionState_95_EntertainerReturning) {
				buildingId = f->buildingId;
			} else { // going to venue
				buildingId = f->destinationBuildingId;
			}
			numHousesServiced = provideColosseumCoverage(x, y,
				Data_Buildings[buildingId].data.entertainment.days2 ? 2 : 1);
			break;
		case FIGURE_CHARIOTEER:
			numHousesServiced = provideHippodromeCoverage(x, y);
			break;
		case FIGURE_ENGINEER:
			{int maxDamage = 0;
			numHousesServiced = provideEngineerCoverage(x, y, &maxDamage);
			if (maxDamage > f->minMaxSeen) {
				f->minMaxSeen = maxDamage;
			} else if (f->minMaxSeen <= 10) {
				f->minMaxSeen = 0;
			} else {
				f->minMaxSeen -= 10;
			}}
			break;
		case FIGURE_PREFECT:
			numHousesServiced = providePrefectFireCoverage(x, y);
			f->minMaxSeen = getPrefectCrimeCoverage(x, y);
			break;
		case FIGURE_RIOTER:
			if (FigureAction_Rioter_collapseBuilding(f) == 1) {
				return 1;
			}
			break;
	}
	if (f->buildingId) {
		buildingId = f->buildingId;
		Data_Buildings[buildingId].housesCovered += numHousesServiced;
		if (Data_Buildings[buildingId].housesCovered > 300) {
			Data_Buildings[buildingId].housesCovered = 300;
		}
	}
	return 0;
}
