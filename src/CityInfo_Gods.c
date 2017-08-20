#include "CityInfo.h"

#include "Building.h"
#include "Event.h"
#include "Figure.h"
#include "Formation.h"
#include "PlayerMessage.h"
#include "Util.h"

#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Settings.h"

#include "core/random.h"

#define MAX_GODS 5

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define TIE 10

#define UPDATE(x,val,min,max) \
	Data_CityInfo.x += val;\
	if (Data_CityInfo.x < min) Data_CityInfo.x = min; \
	else if (Data_CityInfo.x > max) Data_CityInfo.x = max;

static void performBlessing(int god)
{
	switch (god) {
		case God_Ceres:
			PlayerMessage_post(1, Message_96_BlessingFromCeres, 0, 0);
			Building_Industry_blessFarmsFromCeres();
			break;
		case God_Neptune:
			PlayerMessage_post(1, Message_97_BlessingFromNeptune, 0, 0);
			Data_CityInfo.godBlessingNeptuneDoubleTrade = 1;
			break;
		case God_Mercury:
			PlayerMessage_post(1, Message_98_BlessingFromMercury, 0, 0);
			Building_Mercury_fillGranary();
			break;
		case God_Mars:
			PlayerMessage_post(1, Message_99_BlessingFromMars, 0, 0);
			Data_CityInfo.godBlessingMarsEnemiesToKill = 10;
			break;
		case God_Venus:
			PlayerMessage_post(1, Message_100_BlessingFromVenus, 0, 0);
			CityInfo_Population_changeHappiness(25);
			break;
	}
}

static void performSmallCurse(int god)
{
	switch (god) {
		case God_Ceres:
			PlayerMessage_post(1, Message_91_CeresIsUpset, 0, 0);
			Building_Industry_witherFarmCropsFromCeres(0);
			break;
		case God_Neptune:
			PlayerMessage_post(1, Message_92_NeptuneIsUpset, 0, 0);
			Figure_sinkAllShips();
			Data_CityInfo.godCurseNeptuneSankShips = 1;
			break;
		case God_Mercury:
			PlayerMessage_post(1, Message_93_MercuryIsUpset, 0, 0);
			Building_Mercury_removeResources(0);
			break;
		case God_Mars:
			if (Event_startInvasionLocalUprisingFromMars()) {
				PlayerMessage_post(1, Message_94_MarsIsUpset, 0, 0);
			} else {
				PlayerMessage_post(1, Message_44_WrathOfMarsNoMilitary, 0, 0);
			}
			break;
		case God_Venus:
			PlayerMessage_post(1, Message_95_VenusIsUpset, 0, 0);
			CityInfo_Population_setMaxHappiness(50);
			CityInfo_Population_changeHappiness(-5);
			CityInfo_Population_changeHealthRate(-10);
			CityInfo_Population_calculateSentiment();
			break;
	}
}

static int performLargeCurse(int god)
{
	switch (god) {
		case God_Ceres:
			PlayerMessage_post(1, Message_41_WrathOfCeres, 0, 0);
			Building_Industry_witherFarmCropsFromCeres(1);
			break;
		case God_Neptune:
			if (Data_CityInfo.tradeNumOpenSeaRoutes <= 0) {
				PlayerMessage_post(1, Message_42_WrathOfNeptuneNoSeaTrade, 0, 0);
				return 0;
			} else {
				PlayerMessage_post(1, Message_81_WrathOfNeptune, 0, 0);
				Figure_sinkAllShips();
				Data_CityInfo.godCurseNeptuneSankShips = 1;
				Data_CityInfo.tradeSeaProblemDuration = 80;
			}
			break;
		case God_Mercury:
			PlayerMessage_post(1, Message_43_WrathOfMercury, 0, 0);
			Building_Mercury_removeResources(1);
			break;
		case God_Mars:
			if (Formation_marsCurseFort()) {
				PlayerMessage_post(1, Message_82_WrathOfMars, 0, 0);
				Event_startInvasionLocalUprisingFromMars();
			} else {
				PlayerMessage_post(1, Message_44_WrathOfMarsNoMilitary, 0, 0);
			}
			break;
		case God_Venus:
			PlayerMessage_post(1, Message_45_WrathOfVenus, 0, 0);
			CityInfo_Population_setMaxHappiness(40);
			CityInfo_Population_changeHappiness(-10);
			if (Data_CityInfo.healthRate >= 80) {
				CityInfo_Population_changeHealthRate(-50);
			} else if (Data_CityInfo.healthRate >= 60) {
				CityInfo_Population_changeHealthRate(-40);
			} else {
				CityInfo_Population_changeHealthRate(-20);
			}
			Data_CityInfo.godCurseVenusActive = 1;
			CityInfo_Population_calculateSentiment();
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
		if (IsTutorial1()) {
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
	if (Data_CityInfo_Extra.gameTimeDay != 0) {
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
	if (!Data_Settings.godsEnabled) {
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
			if (Data_Settings.currentMissionId < 4 && !Data_Settings.isCustomScenario) {
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
			PlayerMessage_post(0, Message_101_GodsWrathful, 0, 0);
		} else {
			PlayerMessage_post(0, Message_55_GodsUnhappy, 0, 0);
		}
	}
}

void CityInfo_Gods_calculateMoods(int updateMoods)
{
	// base happiness: percentage of houses covered
	Data_CityInfo.godTargetHappiness[God_Ceres] = Data_CityInfo_CultureCoverage.religionCeres;
	Data_CityInfo.godTargetHappiness[God_Neptune] = Data_CityInfo_CultureCoverage.religionNeptune;
	Data_CityInfo.godTargetHappiness[God_Mercury] = Data_CityInfo_CultureCoverage.religionMercury;
	Data_CityInfo.godTargetHappiness[God_Mars] = Data_CityInfo_CultureCoverage.religionMars;
	Data_CityInfo.godTargetHappiness[God_Venus] = Data_CityInfo_CultureCoverage.religionVenus;

	int maxTemples = 0;
	int maxGod = TIE;
	int minTemples = 100000;
	int minGod = TIE;
	for (int i = 0; i < MAX_GODS; i++) {
		int numTemples = 0;
		switch (i) {
			case God_Ceres:
				numTemples = Data_CityInfo_Buildings.smallTempleCeres.total + Data_CityInfo_Buildings.largeTempleCeres.total;
				break;
			case God_Neptune:
				numTemples = Data_CityInfo_Buildings.smallTempleNeptune.total + Data_CityInfo_Buildings.largeTempleNeptune.total;
				break;
			case God_Mercury:
				numTemples = Data_CityInfo_Buildings.smallTempleMercury.total + Data_CityInfo_Buildings.largeTempleMercury.total;
				break;
			case God_Mars:
				numTemples = Data_CityInfo_Buildings.smallTempleMars.total + Data_CityInfo_Buildings.largeTempleMars.total;
				break;
			case God_Venus:
				numTemples = Data_CityInfo_Buildings.smallTempleVenus.total + Data_CityInfo_Buildings.largeTempleVenus.total;
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
		BOUND(Data_CityInfo.godTargetHappiness[i], minHappiness, 100);
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
	if (Data_CityInfo.plannedFestivalSize <= Festival_None) {
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
			case Festival_Small: CityInfo_Population_changeHappiness(7); break;
			case Festival_Large: CityInfo_Population_changeHappiness(9); break;
			case Festival_Grand: CityInfo_Population_changeHappiness(12); break;
		}
	} else if (Data_CityInfo.festivalEffectMonthsDelaySecond <= 0) {
		Data_CityInfo.festivalEffectMonthsDelaySecond = 12;
		switch (Data_CityInfo.plannedFestivalSize) {
			case Festival_Small: CityInfo_Population_changeHappiness(2); break;
			case Festival_Large: CityInfo_Population_changeHappiness(3); break;
			case Festival_Grand: CityInfo_Population_changeHappiness(5); break;
		}
	}
	Data_CityInfo.monthsSinceFestival = 1;
	Data_CityInfo.godMonthsSinceFestival[Data_CityInfo.plannedFestivalGod] = 0;
	switch (Data_CityInfo.plannedFestivalSize) {
		case Festival_Small: PlayerMessage_post(1, Message_38_SmallFestival, 0, 0); break;
		case Festival_Large: PlayerMessage_post(1, Message_39_LargeFestival, 0, 0); break;
		case Festival_Grand: PlayerMessage_post(1, Message_40_GrandFestival, 0, 0); break;
	}
	Data_CityInfo.plannedFestivalSize = Festival_None;
	Data_CityInfo.plannedFestivalMonthsToGo = 0;
}

