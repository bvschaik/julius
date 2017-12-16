#include "CityInfo.h"
#include "Data/CityInfo.h"
#include "Data/Building.h"

#include "building/count.h"
#include "city/finance.h"
#include "figure/figure.h"

void CityInfo_Tick_countBuildingTypes()
{
    building_count_clear();
	Data_CityInfo.numWorkingWharfs = 0;
	Data_CityInfo.shipyardBoatsRequested = 0;
	for (int i = 0; i < 8; i++) {
		Data_CityInfo.workingDockBuildingIds[i] = 0;
	}
	Data_CityInfo.numWorkingDocks = 0;
	Data_CityInfo.numHospitalWorkers = 0;

	for (int i = 1; i < MAX_BUILDINGS; i++) {
        struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i) || b->houseSize) {
			continue;
		}
		int isEntertainmentVenue = 0;
        int type = b->type;
		switch (type) {
            // SPECIAL TREATMENT
            // entertainment venues
            case BUILDING_THEATER:
            case BUILDING_AMPHITHEATER:
            case BUILDING_COLOSSEUM:
            case BUILDING_HIPPODROME:
                isEntertainmentVenue = 1;
                building_count_increase(type, b->numWorkers > 0);
                break;

            case BUILDING_BARRACKS:
                Data_CityInfo.buildingBarracksBuildingId = i;
                building_count_increase(type, b->numWorkers > 0);
                break;

            case BUILDING_HOSPITAL:
                building_count_increase(type, b->numWorkers > 0);
                Data_CityInfo.numHospitalWorkers += b->numWorkers;
                break;
            
            // water
            case BUILDING_RESERVOIR:
            case BUILDING_FOUNTAIN:
                building_count_increase(type, b->hasWaterAccess);
                break;

            // DEFAULT TREATMENT
			// education
			case BUILDING_SCHOOL:
			case BUILDING_LIBRARY:
			case BUILDING_ACADEMY:
			// health
			case BUILDING_BARBER:
			case BUILDING_BATHHOUSE:
			case BUILDING_DOCTOR:
			// government
			case BUILDING_FORUM:
			case BUILDING_FORUM_UPGRADED:
			case BUILDING_SENATE:
			case BUILDING_SENATE_UPGRADED:
			// entertainment schools
			case BUILDING_ACTOR_COLONY:
			case BUILDING_GLADIATOR_SCHOOL:
			case BUILDING_LION_HOUSE:
			case BUILDING_CHARIOT_MAKER:
			// distribution
			case BUILDING_MARKET:
			// military
			case BUILDING_MILITARY_ACADEMY:
			// religion
			case BUILDING_SMALL_TEMPLE_CERES:
			case BUILDING_SMALL_TEMPLE_NEPTUNE:
			case BUILDING_SMALL_TEMPLE_MERCURY:
			case BUILDING_SMALL_TEMPLE_MARS:
			case BUILDING_SMALL_TEMPLE_VENUS:
			case BUILDING_LARGE_TEMPLE_CERES:
			case BUILDING_LARGE_TEMPLE_NEPTUNE:
			case BUILDING_LARGE_TEMPLE_MERCURY:
			case BUILDING_LARGE_TEMPLE_MARS:
			case BUILDING_LARGE_TEMPLE_VENUS:
			case BUILDING_ORACLE:
				building_count_increase(type, b->numWorkers > 0);
				break;

			// industry
			case BUILDING_WHEAT_FARM:
				building_count_industry_increase(RESOURCE_WHEAT, b->numWorkers > 0);
				break;
			case BUILDING_VEGETABLE_FARM:
				building_count_industry_increase(RESOURCE_VEGETABLES, b->numWorkers > 0);
				break;
			case BUILDING_FRUIT_FARM:
				building_count_industry_increase(RESOURCE_FRUIT, b->numWorkers > 0);
				break;
			case BUILDING_OLIVE_FARM:
				building_count_industry_increase(RESOURCE_OLIVES, b->numWorkers > 0);
				break;
			case BUILDING_VINES_FARM:
				building_count_industry_increase(RESOURCE_VINES, b->numWorkers > 0);
				break;
			case BUILDING_PIG_FARM:
				building_count_industry_increase(RESOURCE_MEAT, b->numWorkers > 0);
				break;
			case BUILDING_MARBLE_QUARRY:
				building_count_industry_increase(RESOURCE_MARBLE, b->numWorkers > 0);
				break;
			case BUILDING_IRON_MINE:
				building_count_industry_increase(RESOURCE_IRON, b->numWorkers > 0);
				break;
			case BUILDING_TIMBER_YARD:
				building_count_industry_increase(RESOURCE_TIMBER, b->numWorkers > 0);
				break;
			case BUILDING_CLAY_PIT:
				building_count_industry_increase(RESOURCE_CLAY, b->numWorkers > 0);
				break;
			case BUILDING_WINE_WORKSHOP:
				building_count_industry_increase(RESOURCE_WINE, b->numWorkers > 0);
				break;
			case BUILDING_OIL_WORKSHOP:
				building_count_industry_increase(RESOURCE_OIL, b->numWorkers > 0);
				break;
			case BUILDING_WEAPONS_WORKSHOP:
				building_count_industry_increase(RESOURCE_WEAPONS, b->numWorkers > 0);
				break;
			case BUILDING_FURNITURE_WORKSHOP:
				building_count_industry_increase(RESOURCE_FURNITURE, b->numWorkers > 0);
				break;
			case BUILDING_POTTERY_WORKSHOP:
				building_count_industry_increase(RESOURCE_POTTERY, b->numWorkers > 0);
				break;

			// water-side
			case BUILDING_WHARF:
				if (b->numWorkers > 0) {
					++Data_CityInfo.numWorkingWharfs;
					if (!b->data.other.boatFigureId) {
						++Data_CityInfo.shipyardBoatsRequested;
					}
				}
				break;
			case BUILDING_DOCK:
				if (b->numWorkers > 0 && b->hasWaterAccess) {
					if (Data_CityInfo.numWorkingDocks < 10) {
						Data_CityInfo.workingDockBuildingIds[Data_CityInfo.numWorkingDocks] = i;
					}
					++Data_CityInfo.numWorkingDocks;
				}
				break;
		}
		if (b->immigrantFigureId) {
			figure *f = figure_get(b->immigrantFigureId);
			if (f->state != FigureState_Alive || f->destinationBuildingId != i) {
				b->immigrantFigureId = 0;
			}
		}
		if (isEntertainmentVenue) {
			// update number of shows
			int shows = 0;
			if (b->data.entertainment.days1 > 0) {
				--b->data.entertainment.days1;
				++shows;
			}
			if (b->data.entertainment.days2 > 0) {
				--b->data.entertainment.days2;
				++shows;
			}
			b->data.entertainment.numShows = shows;
		}
	}
	building_count_limit_hippodrome();
}

void CityInfo_Tick_distributeTreasuryOverForumsAndSenates()
{
	int units =
		5 * building_count_active(BUILDING_SENATE) +
		1 * building_count_active(BUILDING_FORUM) +
		8 * building_count_active(BUILDING_SENATE_UPGRADED) +
		2 * building_count_active(BUILDING_FORUM_UPGRADED);
	int amountPerUnit;
	int remainder;
    int treasury = city_finance_treasury();
	if (treasury > 0 && units > 0) {
		amountPerUnit = treasury / units;
		remainder = treasury - units * amountPerUnit;
	} else {
		amountPerUnit = 0;
		remainder = 0;
	}

	for (int i = 1; i < MAX_BUILDINGS; i++) {
        struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i) || b->houseSize) {
			continue;
		}
		b->taxIncomeOrStorage = 0;
		if (b->numWorkers <= 0) {
			continue;
		}
		switch (b->type) {
			// ordered based on importance: most important gets the remainder
			case BUILDING_SENATE_UPGRADED:
				b->taxIncomeOrStorage = 8 * amountPerUnit + remainder;
				remainder = 0;
				break;
			case BUILDING_SENATE:
				if (remainder && !building_count_active(BUILDING_SENATE_UPGRADED)) {
					b->taxIncomeOrStorage = 5 * amountPerUnit + remainder;
					remainder = 0;
				} else {
					b->taxIncomeOrStorage = 5 * amountPerUnit;
				}
				break;
			case BUILDING_FORUM_UPGRADED:
				if (remainder && !(
					building_count_active(BUILDING_SENATE_UPGRADED) ||
					building_count_active(BUILDING_SENATE))) {
					b->taxIncomeOrStorage = 2 * amountPerUnit + remainder;
					remainder = 0;
				} else {
					b->taxIncomeOrStorage = 2 * amountPerUnit;
				}
				break;
			case BUILDING_FORUM:
				if (remainder && !(
					building_count_active(BUILDING_SENATE_UPGRADED) ||
					building_count_active(BUILDING_SENATE) ||
					building_count_active(BUILDING_FORUM_UPGRADED))) {
					b->taxIncomeOrStorage = amountPerUnit + remainder;
					remainder = 0;
				} else {
					b->taxIncomeOrStorage = amountPerUnit;
				}
				break;
		}
	}
}
