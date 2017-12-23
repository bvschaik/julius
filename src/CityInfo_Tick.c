#include "CityInfo.h"
#include "Data/CityInfo.h"

#include "building/building.h"
#include "building/count.h"
#include "city/finance.h"
#include "figure/figure.h"

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
        building *b = building_get(i);
		if (!BuildingIsInUse(b) || b->houseSize) {
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
