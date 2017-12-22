#include "Figure.h"

#include "Building.h"
#include "FigureAction.h"

#include "building/building.h"
#include "building/model.h"
#include "city/culture.h"
#include "figure/type.h"
#include "game/resource.h"
#include "map/building.h"
#include "map/grid.h"

#define FOR_XY_RADIUS \
	int x_min, y_min, x_max, y_max;\
	map_grid_get_area(x, y, 1, 2, &x_min, &y_min, &x_max, &y_max);\
	int gridOffset = map_grid_offset(x_min, y_min);\
	for (int yy = y_min; yy <= y_max; yy++) {\
		for (int xx = x_min; xx <= x_max; xx++) {\
			int building_id = map_building_at(gridOffset);\
			if (building_id) {\
                building *b = building_get(building_id);

#define END_FOR_XY_RADIUS \
			}\
			++gridOffset;\
		}\
		gridOffset += 162 - (x_max - x_min + 1);\
	}

static int provideEngineerCoverage(int x, int y, int *maxDamageRiskSeen)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (b->type == BUILDING_HIPPODROME) {
			b = building_main(b);
		}
		if (b->damageRisk > *maxDamageRiskSeen) {
			*maxDamageRiskSeen = b->damageRisk;
		}
		b->damageRisk = 0;
		if (b->houseSize && b->housePopulation > 0) {
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int providePrefectFireCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (b->type == BUILDING_HIPPODROME) {
			b = building_main(b);
		}
		b->fireRisk = 0;
		if (b->houseSize && b->housePopulation > 0) {
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int getPrefectCrimeCoverage(int x, int y)
{
	int minHappinessSeen = 100;
	FOR_XY_RADIUS {
		if (b->sentiment.houseHappiness < minHappinessSeen) {
			minHappinessSeen = b->sentiment.houseHappiness;
		}
	} END_FOR_XY_RADIUS;
	return minHappinessSeen;
}

static int provideTheaterCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (b->houseSize && b->housePopulation > 0) {
			b->data.house.theater = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideAmphitheaterCoverage(int x, int y, int numShows)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (b->houseSize && b->housePopulation > 0) {
			b->data.house.amphitheaterActor = 96;
			if (numShows == 2) {
				b->data.house.amphitheaterGladiator = 96;
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
		if (b->houseSize && b->housePopulation > 0) {
			b->data.house.colosseumGladiator = 96;
			if (numShows == 2) {
				b->data.house.colosseumLion = 96;
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
		if (b->houseSize && b->housePopulation > 0) {
			b->data.house.hippodrome = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideMarketGoods(int marketBuildingId, int x, int y)
{
	int serviced = 0;
	building *market = building_get(marketBuildingId);
	FOR_XY_RADIUS {
		if (b->houseSize && b->housePopulation > 0) {
			serviced++;
			int level = b->subtype.houseLevel;
			if (level < HOUSE_LUXURY_PALACE) {
				level++;
			}
			int maxFoodStocks = 4 * b->houseMaxPopulationSeen;
			int foodTypesStoredMax = 0;
			for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
				if (b->data.house.inventory[i] >= maxFoodStocks) {
					foodTypesStoredMax++;
				}
			}
			const model_house *model = model_get_house(level);
			if (model->food_types > foodTypesStoredMax) {
				for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
					if (b->data.house.inventory[i] >= maxFoodStocks) {
						continue;
					}
					if (market->data.market.inventory[i] >= maxFoodStocks) {
						b->data.house.inventory[i] += maxFoodStocks;
						market->data.market.inventory[i] -= maxFoodStocks;
						break;
					} else if (market->data.market.inventory[i]) {
						b->data.house.inventory[i] += market->data.market.inventory[i];
						market->data.market.inventory[i] = 0;
						break;
					}
				}
			}
			if (model->pottery) {
				market->data.market.potteryDemand = 10;
				int potteryWanted = 8 * model->pottery - b->data.house.inventory[INVENTORY_POTTERY];
				if (market->data.market.inventory[INVENTORY_POTTERY] > 0 && potteryWanted > 0) {
					if (potteryWanted <= market->data.market.inventory[INVENTORY_POTTERY]) {
						b->data.house.inventory[INVENTORY_POTTERY] += potteryWanted;
						market->data.market.inventory[INVENTORY_POTTERY] -= potteryWanted;
					} else {
						b->data.house.inventory[INVENTORY_POTTERY] += market->data.market.inventory[INVENTORY_POTTERY];
						market->data.market.inventory[INVENTORY_POTTERY] = 0;
					}
				}
			}
			if (model->furniture) {
				market->data.market.furnitureDemand = 10;
				int furnitureWanted = 4 * model->furniture - b->data.house.inventory[INVENTORY_FURNITURE];
				if (market->data.market.inventory[INVENTORY_FURNITURE] > 0 && furnitureWanted > 0) {
					if (furnitureWanted <= market->data.market.inventory[INVENTORY_FURNITURE]) {
						b->data.house.inventory[INVENTORY_FURNITURE] += furnitureWanted;
						market->data.market.inventory[INVENTORY_FURNITURE] -= furnitureWanted;
					} else {
						b->data.house.inventory[INVENTORY_FURNITURE] += market->data.market.inventory[INVENTORY_FURNITURE];
						market->data.market.inventory[INVENTORY_FURNITURE] = 0;
					}
				}
			}
			if (model->oil) {
				market->data.market.oilDemand = 10;
				int oilWanted = 4 * model->oil - b->data.house.inventory[INVENTORY_OIL];
				if (market->data.market.inventory[INVENTORY_OIL] > 0 && oilWanted > 0) {
					if (oilWanted <= market->data.market.inventory[INVENTORY_OIL]) {
						b->data.house.inventory[INVENTORY_OIL] += oilWanted;
						market->data.market.inventory[INVENTORY_OIL] -= oilWanted;
					} else {
						b->data.house.inventory[INVENTORY_OIL] += market->data.market.inventory[INVENTORY_OIL];
						market->data.market.inventory[INVENTORY_OIL] = 0;
					}
				}
			}
			if (model->wine) {
				market->data.market.wineDemand = 10;
				int wineWanted = 4 * model->wine - b->data.house.inventory[INVENTORY_WINE];
				if (market->data.market.inventory[INVENTORY_WINE] > 0 && wineWanted > 0) {
					if (wineWanted <= market->data.market.inventory[INVENTORY_WINE]) {
						b->data.house.inventory[INVENTORY_WINE] += wineWanted;
						market->data.market.inventory[INVENTORY_WINE] -= wineWanted;
					} else {
						b->data.house.inventory[INVENTORY_WINE] += market->data.market.inventory[INVENTORY_WINE];
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
		if (b->houseSize && b->housePopulation > 0) {
			b->data.house.bathhouse = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideReligionCoverage(int x, int y, int god)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (b->houseSize && b->housePopulation > 0) {
			switch (god) {
				case GOD_CERES:
					b->data.house.templeCeres = 96;
					break;
				case GOD_NEPTUNE:
					b->data.house.templeNeptune = 96;
					break;
				case GOD_MERCURY:
					b->data.house.templeMercury = 96;
					break;
				case GOD_MARS:
					b->data.house.templeMars = 96;
					break;
				case GOD_VENUS:
					b->data.house.templeVenus = 96;
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
		if (b->houseSize && b->housePopulation > 0) {
			b->data.house.school = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideAcademyCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (b->houseSize && b->housePopulation > 0) {
			b->data.house.academy = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideLibraryCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (b->houseSize && b->housePopulation > 0) {
			b->data.house.library = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideBarberCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (b->houseSize && b->housePopulation > 0) {
			b->data.house.barber = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideClinicCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (b->houseSize && b->housePopulation > 0) {
			b->data.house.clinic = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideHospitalCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (b->houseSize && b->housePopulation > 0) {
			b->data.house.hospital = 96;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static int provideMissionaryCoverage(int x, int y)
{
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, 1, 4, &x_min, &y_min, &x_max, &y_max);
	int gridOffset = map_grid_offset(x_min, y_min);
	for (int yy = y_min; yy <= y_max; yy++) {
		for (int xx = x_min; xx <= x_max; xx++) {
			int buildingId = map_building_at(gridOffset);
			if (buildingId) {
                building *b = building_get(buildingId);
				if (b->type == BUILDING_NATIVE_HUT || b->type == BUILDING_NATIVE_MEETING) {
					b->sentiment.nativeAnger = 0;
				}
			}
			++gridOffset;
		}
		gridOffset += 162 - (x_max - x_min + 1);
	}
	return 1;
}

static int provideLaborSeekerCoverage(int x, int y)
{
	int serviced = 0;
	FOR_XY_RADIUS {
		if (b->houseSize && b->housePopulation > 0) {
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
		if (b->houseSize && b->housePopulation > 0) {
			int taxMultiplier = model_get_house(b->subtype.houseLevel)->tax_multiplier;
			if (taxMultiplier > *maxTaxMultiplier) {
				*maxTaxMultiplier = taxMultiplier;
			}
			b->houseTaxCoverage = 50;
			serviced++;
		}
	} END_FOR_XY_RADIUS;
	return serviced;
}

static building *get_entertainment_building(const figure *f)
{
    if (f->actionState == FigureActionState_94_EntertainerRoaming ||
        f->actionState == FigureActionState_95_EntertainerReturning) {
        return building_get(f->buildingId);
    } else { // going to venue
        return building_get(f->destinationBuildingId);
    }
}

int Figure_provideServiceCoverage(figure *f)
{
	int numHousesServiced = 0;
	int x = f->x;
	int y = f->y;
	building *b;
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
			switch (building_get(f->buildingId)->type) {
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
			b = get_entertainment_building(f);
			if (b->type == BUILDING_THEATER) {
				numHousesServiced = provideTheaterCoverage(x, y);
			} else if (b->type == BUILDING_AMPHITHEATER) {
				numHousesServiced = provideAmphitheaterCoverage(x, y,
					b->data.entertainment.days1 ? 2 : 1);
			}
			break;
		case FIGURE_GLADIATOR:
			b = get_entertainment_building(f);
			if (b->type == BUILDING_AMPHITHEATER) {
				numHousesServiced = provideAmphitheaterCoverage(x, y,
					b->data.entertainment.days2 ? 2 : 1);
			} else if (b->type == BUILDING_COLOSSEUM) {
				numHousesServiced = provideColosseumCoverage(x, y,
					b->data.entertainment.days1 ? 2 : 1);
			}
			break;
		case FIGURE_LION_TAMER:
			b = get_entertainment_building(f);
			numHousesServiced = provideColosseumCoverage(x, y,
				b->data.entertainment.days2 ? 2 : 1);
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
		b = building_get(f->buildingId);
		b->housesCovered += numHousesServiced;
		if (b->housesCovered > 300) {
			b->housesCovered = 300;
		}
	}
	return 0;
}
