#include "CityInfo.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Building.h"
#include "Data/Figure.h"

#include "building/count.h"

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
		if (!BuildingIsInUse(i) || Data_Buildings[i].houseSize) {
			continue;
		}
		int isEntertainmentVenue = 0;
        int type = Data_Buildings[i].type;
		switch (type) {
            // SPECIAL TREATMENT
            // entertainment venues
            case BUILDING_THEATER:
            case BUILDING_AMPHITHEATER:
            case BUILDING_COLOSSEUM:
            case BUILDING_HIPPODROME:
                isEntertainmentVenue = 1;
                building_count_increase(type, Data_Buildings[i].numWorkers > 0);
                break;

            case BUILDING_BARRACKS:
                Data_CityInfo.buildingBarracksBuildingId = i;
                building_count_increase(type, Data_Buildings[i].numWorkers > 0);
                break;

            case BUILDING_HOSPITAL:
                building_count_increase(type, Data_Buildings[i].numWorkers > 0);
                Data_CityInfo.numHospitalWorkers += Data_Buildings[i].numWorkers;
                break;
            
            // water
            case BUILDING_RESERVOIR:
            case BUILDING_FOUNTAIN:
                building_count_increase(type, Data_Buildings[i].hasWaterAccess);
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
				building_count_increase(type, Data_Buildings[i].numWorkers > 0);
				break;

			// industry
			case BUILDING_WHEAT_FARM:
				building_count_industry_increase(RESOURCE_WHEAT, Data_Buildings[i].numWorkers > 0);
				break;
			case BUILDING_VEGETABLE_FARM:
				building_count_industry_increase(RESOURCE_VEGETABLES, Data_Buildings[i].numWorkers > 0);
				break;
			case BUILDING_FRUIT_FARM:
				building_count_industry_increase(RESOURCE_FRUIT, Data_Buildings[i].numWorkers > 0);
				break;
			case BUILDING_OLIVE_FARM:
				building_count_industry_increase(RESOURCE_OLIVES, Data_Buildings[i].numWorkers > 0);
				break;
			case BUILDING_VINES_FARM:
				building_count_industry_increase(RESOURCE_VINES, Data_Buildings[i].numWorkers > 0);
				break;
			case BUILDING_PIG_FARM:
				building_count_industry_increase(RESOURCE_MEAT, Data_Buildings[i].numWorkers > 0);
				break;
			case BUILDING_MARBLE_QUARRY:
				building_count_industry_increase(RESOURCE_MARBLE, Data_Buildings[i].numWorkers > 0);
				break;
			case BUILDING_IRON_MINE:
				building_count_industry_increase(RESOURCE_IRON, Data_Buildings[i].numWorkers > 0);
				break;
			case BUILDING_TIMBER_YARD:
				building_count_industry_increase(RESOURCE_TIMBER, Data_Buildings[i].numWorkers > 0);
				break;
			case BUILDING_CLAY_PIT:
				building_count_industry_increase(RESOURCE_CLAY, Data_Buildings[i].numWorkers > 0);
				break;
			case BUILDING_WINE_WORKSHOP:
				building_count_industry_increase(RESOURCE_WINE, Data_Buildings[i].numWorkers > 0);
				break;
			case BUILDING_OIL_WORKSHOP:
				building_count_industry_increase(RESOURCE_OIL, Data_Buildings[i].numWorkers > 0);
				break;
			case BUILDING_WEAPONS_WORKSHOP:
				building_count_industry_increase(RESOURCE_WEAPONS, Data_Buildings[i].numWorkers > 0);
				break;
			case BUILDING_FURNITURE_WORKSHOP:
				building_count_industry_increase(RESOURCE_FURNITURE, Data_Buildings[i].numWorkers > 0);
				break;
			case BUILDING_POTTERY_WORKSHOP:
				building_count_industry_increase(RESOURCE_POTTERY, Data_Buildings[i].numWorkers > 0);
				break;

			// water-side
			case BUILDING_WHARF:
				if (Data_Buildings[i].numWorkers > 0) {
					++Data_CityInfo.numWorkingWharfs;
					if (!Data_Buildings[i].data.other.boatFigureId) {
						++Data_CityInfo.shipyardBoatsRequested;
					}
				}
				break;
			case BUILDING_DOCK:
				if (Data_Buildings[i].numWorkers > 0 && Data_Buildings[i].hasWaterAccess) {
					if (Data_CityInfo.numWorkingDocks < 10) {
						Data_CityInfo.workingDockBuildingIds[Data_CityInfo.numWorkingDocks] = i;
					}
					++Data_CityInfo.numWorkingDocks;
				}
				break;
		}
		if (Data_Buildings[i].immigrantFigureId) {
			int immigrantId = Data_Buildings[i].immigrantFigureId;
			if (Data_Figures[immigrantId].state != FigureState_Alive || Data_Figures[immigrantId].destinationBuildingId != i) {
				Data_Buildings[i].immigrantFigureId = 0;
			}
		}
		if (isEntertainmentVenue) {
			// update number of shows
			int shows = 0;
			if (Data_Buildings[i].data.entertainment.days1 > 0) {
				--Data_Buildings[i].data.entertainment.days1;
				++shows;
			}
			if (Data_Buildings[i].data.entertainment.days2 > 0) {
				--Data_Buildings[i].data.entertainment.days2;
				++shows;
			}
			Data_Buildings[i].data.entertainment.numShows = shows;
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
	if (Data_CityInfo.treasury > 0 && units > 0) {
		amountPerUnit = Data_CityInfo.treasury / units;
		remainder = Data_CityInfo.treasury - units * amountPerUnit;
	} else {
		amountPerUnit = 0;
		remainder = 0;
	}

	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!BuildingIsInUse(i) || Data_Buildings[i].houseSize) {
			continue;
		}
		Data_Buildings[i].taxIncomeOrStorage = 0;
		if (Data_Buildings[i].numWorkers <= 0) {
			continue;
		}
		switch (Data_Buildings[i].type) {
			// ordered based on importance: most important gets the remainder
			case BUILDING_SENATE_UPGRADED:
				Data_Buildings[i].taxIncomeOrStorage = 8 * amountPerUnit + remainder;
				remainder = 0;
				break;
			case BUILDING_SENATE:
				if (remainder && !building_count_active(BUILDING_SENATE_UPGRADED)) {
					Data_Buildings[i].taxIncomeOrStorage = 5 * amountPerUnit + remainder;
					remainder = 0;
				} else {
					Data_Buildings[i].taxIncomeOrStorage = 5 * amountPerUnit;
				}
				break;
			case BUILDING_FORUM_UPGRADED:
				if (remainder && !(
					building_count_active(BUILDING_SENATE_UPGRADED) ||
					building_count_active(BUILDING_SENATE))) {
					Data_Buildings[i].taxIncomeOrStorage = 2 * amountPerUnit + remainder;
					remainder = 0;
				} else {
					Data_Buildings[i].taxIncomeOrStorage = 2 * amountPerUnit;
				}
				break;
			case BUILDING_FORUM:
				if (remainder && !(
					building_count_active(BUILDING_SENATE_UPGRADED) ||
					building_count_active(BUILDING_SENATE) ||
					building_count_active(BUILDING_FORUM_UPGRADED))) {
					Data_Buildings[i].taxIncomeOrStorage = amountPerUnit + remainder;
					remainder = 0;
				} else {
					Data_Buildings[i].taxIncomeOrStorage = amountPerUnit;
				}
				break;
		}
	}
}
