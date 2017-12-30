#include "CityInfo.h"

#include "Building.h"
#include "Formation.h"

#include "Data/CityInfo.h"

#include "building/count.h"
#include "building/granary.h"
#include "building/industry.h"
#include "city/constants.h"
#include "city/culture.h"
#include "city/message.h"
#include "city/sentiment.h"
#include "core/calc.h"
#include "core/random.h"
#include "figuretype/water.h"
#include "game/settings.h"
#include "game/time.h"
#include "scenario/property.h"
#include "scenario/invasion.h"

#define MAX_GODS 5

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define TIE 10

#define UPDATE(x,val,min,max) \
	Data_CityInfo.x += (val);\
	if (Data_CityInfo.x < (min)) Data_CityInfo.x = (min); \
	else if (Data_CityInfo.x > (max)) Data_CityInfo.x = (max);

static void performBlessing(int god)
{
	switch (god) {
		case GOD_CERES:
			city_message_post(1, MESSAGE_BLESSING_FROM_CERES, 0, 0);
			building_bless_farms();
			break;
		case GOD_NEPTUNE:
			city_message_post(1, MESSAGE_BLESSING_FROM_NEPTUNE, 0, 0);
			Data_CityInfo.godBlessingNeptuneDoubleTrade = 1;
			break;
		case GOD_MERCURY:
			city_message_post(1, MESSAGE_BLESSING_FROM_MERCURY, 0, 0);
			building_granary_bless();
			break;
		case GOD_MARS:
			city_message_post(1, MESSAGE_BLESSING_FROM_MARS, 0, 0);
			Data_CityInfo.godBlessingMarsEnemiesToKill = 10;
			break;
		case GOD_VENUS:
			city_message_post(1, MESSAGE_BLESSING_FROM_VENUS, 0, 0);
			city_sentiment_change_happiness(25);
			break;
	}
}

static void performSmallCurse(int god)
{
	switch (god) {
		case GOD_CERES:
			city_message_post(1, MESSAGE_CERES_IS_UPSET, 0, 0);
			building_curse_farms(0);
			break;
		case GOD_NEPTUNE:
			city_message_post(1, MESSAGE_NEPTUNE_IS_UPSET, 0, 0);
			figure_sink_all_ships();
			Data_CityInfo.godCurseNeptuneSankShips = 1;
			break;
		case GOD_MERCURY:
			city_message_post(1, MESSAGE_MERCURY_IS_UPSET, 0, 0);
			building_granary_warehouse_curse(0);
			break;
		case GOD_MARS:
			if (scenario_invasion_start_from_mars()) {
				city_message_post(1, MESSAGE_MARS_IS_UPSET, 0, 0);
			} else {
				city_message_post(1, MESSAGE_WRATH_OF_MARS_NO_MILITARY, 0, 0);
			}
			break;
		case GOD_VENUS:
			city_message_post(1, MESSAGE_VENUS_IS_UPSET, 0, 0);
			city_sentiment_set_max_happiness(50);
			city_sentiment_change_happiness(-5);
			CityInfo_Population_changeHealthRate(-10);
			city_sentiment_update();
			break;
	}
}

static int performLargeCurse(int god)
{
	switch (god) {
		case GOD_CERES:
			city_message_post(1, MESSAGE_WRATH_OF_CERES, 0, 0);
			building_curse_farms(1);
			break;
		case GOD_NEPTUNE:
			if (Data_CityInfo.tradeNumOpenSeaRoutes <= 0) {
				city_message_post(1, MESSAGE_WRATH_OF_NEPTUNE_NO_SEA_TRADE, 0, 0);
				return 0;
			} else {
				city_message_post(1, MESSAGE_WRATH_OF_NEPTUNE, 0, 0);
				figure_sink_all_ships();
				Data_CityInfo.godCurseNeptuneSankShips = 1;
				Data_CityInfo.tradeSeaProblemDuration = 80;
			}
			break;
		case GOD_MERCURY:
			city_message_post(1, MESSAGE_WRATH_OF_MERCURY, 0, 0);
			building_granary_warehouse_curse(1);
			break;
		case GOD_MARS:
			if (Formation_marsCurseFort()) {
				city_message_post(1, MESSAGE_WRATH_OF_MARS, 0, 0);
				scenario_invasion_start_from_mars();
			} else {
				city_message_post(1, MESSAGE_WRATH_OF_MARS_NO_MILITARY, 0, 0);
			}
			break;
		case GOD_VENUS:
			city_message_post(1, MESSAGE_WRATH_OF_VENUS, 0, 0);
			city_sentiment_set_max_happiness(40);
			city_sentiment_change_happiness(-10);
			if (Data_CityInfo.healthRate >= 80) {
				CityInfo_Population_changeHealthRate(-50);
			} else if (Data_CityInfo.healthRate >= 60) {
				CityInfo_Population_changeHealthRate(-40);
			} else {
				CityInfo_Population_changeHealthRate(-20);
			}
			Data_CityInfo.godCurseVenusActive = 1;
			city_sentiment_update();
			break;
	}
	return 1;
}

static void updateGodMoods()
{
	for (int i = 0; i < MAX_GODS; i++) {
		if (Data_CityInfo.godHappiness[i] < Data_CityInfo.godTargetHappiness[i]) {
			Data_CityInfo.godHappiness[i]++;
		} else if (Data_CityInfo.godHappiness[i] > Data_CityInfo.godTargetHappiness[i]) {
			Data_CityInfo.godHappiness[i]--;
		}
		if (scenario_is_tutorial_1()) {
			if (Data_CityInfo.godHappiness[i] < 50) {
				Data_CityInfo.godHappiness[i] = 50;
			}
		}
	}
	for (int i = 0; i < MAX_GODS; i++) {
		if (Data_CityInfo.godHappiness[i] > 50) {
			Data_CityInfo.godSmallCurseDone[i] = 0;
		}
		if (Data_CityInfo.godHappiness[i] < 50) {
			Data_CityInfo.godBlessingDone[i] = 0;
		}
	}

	int god = random_byte() & 7;
	if (god < MAX_GODS) {
		if (Data_CityInfo.godHappiness[god] >= 50) {
			Data_CityInfo.godWrathBolts[god] = 0;
		} else if (Data_CityInfo.godHappiness[god] < 40) {
			if (Data_CityInfo.godHappiness[god] >= 20) {
				Data_CityInfo.godWrathBolts[god] += 1;
			} else if (Data_CityInfo.godHappiness[god] >= 10) {
				Data_CityInfo.godWrathBolts[god] += 2;
			} else {
				Data_CityInfo.godWrathBolts[god] += 5;
			}
		}
		if (Data_CityInfo.godWrathBolts[god] > 50) {
			Data_CityInfo.godWrathBolts[god] = 50;
		}
	}
	if (game_time_day() != 0) {
		return;
	}

	// handle blessings, curses, etc every month
	for (int i = 0; i < MAX_GODS; i++) {
		Data_CityInfo.godMonthsSinceFestival[i]++;
	}
	if (god >= MAX_GODS) {
		if (CityInfo_Gods_calculateLeastHappy()) {
			god = Data_CityInfo.godLeastHappy - 1;
		}
	}
	if (!setting_gods_enabled()) {
		return;
	}
	if (god < MAX_GODS) {
		if (Data_CityInfo.godHappiness[god] >= 100 &&
				!Data_CityInfo.godBlessingDone[god]) {
			Data_CityInfo.godBlessingDone[god] = 1;
			performBlessing(god);
		} else if (Data_CityInfo.godWrathBolts[god] >= 20 &&
				!Data_CityInfo.godSmallCurseDone[god] &&
				Data_CityInfo.godMonthsSinceFestival[god] > 3) {
			Data_CityInfo.godSmallCurseDone[god] = 1;
			Data_CityInfo.godWrathBolts[god] = 0;
			Data_CityInfo.godHappiness[god] += 12;
			performSmallCurse(god);
		} else if (Data_CityInfo.godWrathBolts[god] >= 50 &&
				Data_CityInfo.godMonthsSinceFestival[god] > 3) {
			if (scenario_campaign_rank() < 4 && !scenario_is_custom()) {
				// no large curses in early scenarios
				Data_CityInfo.godSmallCurseDone[god] = 0;
				return;
			}
			Data_CityInfo.godWrathBolts[god] = 0;
			Data_CityInfo.godHappiness[god] += 30;
			if (!performLargeCurse(god)) {
				return;
			}
		}
	}

	int minHappiness = 100;
	for (int i = 0; i < MAX_GODS; i++) {
		if (Data_CityInfo.godHappiness[i] < minHappiness) {
			minHappiness = Data_CityInfo.godHappiness[i];
		}
	}
	if (Data_CityInfo.godAngryMessageDelay) {
		Data_CityInfo.godAngryMessageDelay--;
	} else if (minHappiness < 30) {
		Data_CityInfo.godAngryMessageDelay = 20;
		if (minHappiness < 10) {
			city_message_post(0, MESSAGE_GODS_WRATHFUL, 0, 0);
		} else {
			city_message_post(0, MESSAGE_GODS_UNHAPPY, 0, 0);
		}
	}
}

void CityInfo_Gods_calculateMoods(int updateMoods)
{
	// base happiness: percentage of houses covered
	Data_CityInfo.godTargetHappiness[GOD_CERES] = city_culture_coverage_religion(GOD_CERES);
	Data_CityInfo.godTargetHappiness[GOD_NEPTUNE] = city_culture_coverage_religion(GOD_NEPTUNE);
	Data_CityInfo.godTargetHappiness[GOD_MERCURY] = city_culture_coverage_religion(GOD_MERCURY);
	Data_CityInfo.godTargetHappiness[GOD_MARS] = city_culture_coverage_religion(GOD_MARS);
	Data_CityInfo.godTargetHappiness[GOD_VENUS] = city_culture_coverage_religion(GOD_VENUS);

	int maxTemples = 0;
	int maxGod = TIE;
	int minTemples = 100000;
	int minGod = TIE;
	for (int i = 0; i < MAX_GODS; i++) {
		int numTemples = 0;
		switch (i) {
			case GOD_CERES:
				numTemples = building_count_total(BUILDING_SMALL_TEMPLE_CERES) + building_count_total(BUILDING_LARGE_TEMPLE_CERES);
				break;
			case GOD_NEPTUNE:
				numTemples = building_count_total(BUILDING_SMALL_TEMPLE_NEPTUNE) + building_count_total(BUILDING_LARGE_TEMPLE_NEPTUNE);
				break;
			case GOD_MERCURY:
				numTemples = building_count_total(BUILDING_SMALL_TEMPLE_MERCURY) + building_count_total(BUILDING_LARGE_TEMPLE_MERCURY);
				break;
			case GOD_MARS:
				numTemples = building_count_total(BUILDING_SMALL_TEMPLE_MARS) + building_count_total(BUILDING_LARGE_TEMPLE_MARS);
				break;
			case GOD_VENUS:
				numTemples = building_count_total(BUILDING_SMALL_TEMPLE_VENUS) + building_count_total(BUILDING_LARGE_TEMPLE_VENUS);
				break;
		}
		if (numTemples == maxTemples) {
			maxGod = TIE;
		} else if (numTemples > maxTemples) {
			maxTemples = numTemples;
			maxGod = i;
		}
		if (numTemples == minTemples) {
			minGod = TIE;
		} else if (numTemples < minTemples) {
			minTemples = numTemples;
			minGod = i;
		}
	}
	// happiness factor based on months since festival (max 40)
	for (int i = 0; i < MAX_GODS; i++) {
		int festivalPenalty = Data_CityInfo.godMonthsSinceFestival[i];
		if (festivalPenalty > 40) {
			festivalPenalty = 40;
		}
		Data_CityInfo.godTargetHappiness[i] += 12 - festivalPenalty;
	}

	// BUG poor Venus never gets points here!
	if (maxGod < 4) {
		if (Data_CityInfo.godTargetHappiness[maxGod] >= 50) {
			Data_CityInfo.godTargetHappiness[maxGod] = 100;
		} else {
			Data_CityInfo.godTargetHappiness[maxGod] += 50;
		}
	}
	if (minGod < 4) {
		Data_CityInfo.godTargetHappiness[minGod] -= 25;
	}
	int minHappiness;
	if (Data_CityInfo.population < 100) {
		minHappiness = 50;
	} else if (Data_CityInfo.population < 200) {
		minHappiness = 40;
	} else if (Data_CityInfo.population < 300) {
		minHappiness = 30;
	} else if (Data_CityInfo.population < 400) {
		minHappiness = 20;
	} else if (Data_CityInfo.population < 500) {
		minHappiness = 10;
	} else {
		minHappiness = 0;
	}
	for (int i = 0; i < MAX_GODS; i++) {
		Data_CityInfo.godTargetHappiness[i] = calc_bound(Data_CityInfo.godTargetHappiness[i], minHappiness, 100);
	}
	if (updateMoods) {
		updateGodMoods();
	}
}

int CityInfo_Gods_calculateLeastHappy()
{
	int maxGod = 0;
	int maxWrath = 0;
	for (int i = 0; i < MAX_GODS; i++) {
		if (Data_CityInfo.godWrathBolts[i] > maxWrath) {
			maxGod = i + 1;
			maxWrath = Data_CityInfo.godWrathBolts[i];
		}
	}
	if (maxGod > 0) {
		Data_CityInfo.godLeastHappy = maxGod;
		return 1;
	}
	int minHappiness = 40;
	for (int i = 0; i < MAX_GODS; i++) {
		if (Data_CityInfo.godHappiness[i] < minHappiness) {
			maxGod = i + 1;
			minHappiness = Data_CityInfo.godHappiness[i];
		}
	}
	Data_CityInfo.godLeastHappy = maxGod;
	return maxGod > 0;
}

void CityInfo_Gods_reset()
{
	for (int i = 0; i < MAX_GODS; i++) {
		Data_CityInfo.godTargetHappiness[i] = 50;
		Data_CityInfo.godHappiness[i] = 50;
		Data_CityInfo.godWrathBolts[i] = 0;
		Data_CityInfo.godBlessingDone[i] = 0;
		Data_CityInfo.godSmallCurseDone[i] = 0;
		Data_CityInfo.godUnused1[i] = 0;
		Data_CityInfo.godUnused2[i] = 0;
		Data_CityInfo.godUnused3[i] = 0;
		Data_CityInfo.godMonthsSinceFestival[i] = 0;
	}
	Data_CityInfo.godAngryMessageDelay = 0;
}

void CityInfo_Gods_checkFestival()
{
	if (Data_CityInfo.festivalEffectMonthsDelayFirst) {
		--Data_CityInfo.festivalEffectMonthsDelayFirst;
	}
	if (Data_CityInfo.festivalEffectMonthsDelaySecond) {
		--Data_CityInfo.festivalEffectMonthsDelaySecond;
	}
	if (Data_CityInfo.plannedFestivalSize <= FESTIVAL_NONE) {
		return;
	}
	Data_CityInfo.plannedFestivalMonthsToGo--;
	if (Data_CityInfo.plannedFestivalMonthsToGo > 0) {
		return;
	}
	// throw a party!
	if (Data_CityInfo.festivalEffectMonthsDelayFirst <= 0) {
		Data_CityInfo.festivalEffectMonthsDelayFirst = 12;
		switch (Data_CityInfo.plannedFestivalSize) {
			case FESTIVAL_SMALL: city_sentiment_change_happiness(7); break;
			case FESTIVAL_LARGE: city_sentiment_change_happiness(9); break;
			case FESTIVAL_GRAND: city_sentiment_change_happiness(12); break;
		}
	} else if (Data_CityInfo.festivalEffectMonthsDelaySecond <= 0) {
		Data_CityInfo.festivalEffectMonthsDelaySecond = 12;
		switch (Data_CityInfo.plannedFestivalSize) {
			case FESTIVAL_SMALL: city_sentiment_change_happiness(2); break;
			case FESTIVAL_LARGE: city_sentiment_change_happiness(3); break;
			case FESTIVAL_GRAND: city_sentiment_change_happiness(5); break;
		}
	}
	Data_CityInfo.monthsSinceFestival = 1;
	Data_CityInfo.godMonthsSinceFestival[Data_CityInfo.plannedFestivalGod] = 0;
	switch (Data_CityInfo.plannedFestivalSize) {
		case FESTIVAL_SMALL: city_message_post(1, MESSAGE_SMALL_FESTIVAL, 0, 0); break;
		case FESTIVAL_LARGE: city_message_post(1, MESSAGE_LARGE_FESTIVAL, 0, 0); break;
		case FESTIVAL_GRAND: city_message_post(1, MESSAGE_GRAND_FESTIVAL, 0, 0); break;
	}
	Data_CityInfo.plannedFestivalSize = FESTIVAL_NONE;
	Data_CityInfo.plannedFestivalMonthsToGo = 0;
}

