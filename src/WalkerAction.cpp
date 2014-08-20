#include "WalkerAction_private.h"

#include "Walker.h"
#include "Data/CityInfo.h"

static void (*walkerActionCallbacks[])(int walkerId) = {
	WalkerAction_nobody, //0
	WalkerAction_immigrant,
	WalkerAction_emigrant,
	WalkerAction_homeless,
	WalkerAction_cartpusher,
	WalkerAction_laborSeeker,
	WalkerAction_explosionCloud,
	WalkerAction_taxCollector,
	WalkerAction_engineer,
	WalkerAction_warehouseman,
	WalkerAction_prefect, //10
	WalkerAction_soldier,
	WalkerAction_soldier,
	WalkerAction_soldier,
	WalkerAction_nobody, //TODO WalkerAction_militaryStandard,
	WalkerAction_entertainer,
	WalkerAction_entertainer,
	WalkerAction_entertainer,
	WalkerAction_entertainer,
	WalkerAction_tradeCaravan,
	WalkerAction_tradeShip, //20
	WalkerAction_tradeCaravanDonkey,
	WalkerAction_protestor,
	WalkerAction_criminal,
	WalkerAction_nobody, //TODO WalkerAction_rioter,
	WalkerAction_nobody, //TODO WalkerAction_fishingBoat,
	WalkerAction_marketTrader,
	WalkerAction_priest,
	WalkerAction_schoolChild,
	WalkerAction_teacher,
	WalkerAction_librarian, //30
	WalkerAction_barber,
	WalkerAction_bathhouseWorker,
	WalkerAction_doctor,
	WalkerAction_surgeon,
	WalkerAction_worker,
	WalkerAction_nobody, // mapFlag: editor only
	WalkerAction_nobody, //TODO WalkerAction_flotsam,
	WalkerAction_nobody, //TODO WalkerAction_docker,
	WalkerAction_marketBuyer,
	WalkerAction_patrician, //40
	WalkerAction_nobody, //TODO WalkerAction_indigenousNative,
	WalkerAction_nobody, //TODO WalkerAction_towerSentry,
	WalkerAction_nobody, //TODO WalkerAction_enemy43,
	WalkerAction_nobody, //TODO WalkerAction_enemy44,
	WalkerAction_nobody, //TODO WalkerAction_enemy45,
	WalkerAction_nobody, //TODO WalkerAction_enemy46,
	WalkerAction_nobody, //TODO WalkerAction_enemy47,
	WalkerAction_nobody, //TODO WalkerAction_enemy48,
	WalkerAction_nobody, //TODO WalkerAction_enemy49,
	WalkerAction_nobody, //TODO WalkerAction_enemy50, //50
	WalkerAction_nobody, //TODO WalkerAction_enemy51,
	WalkerAction_nobody, //TODO WalkerAction_enemy52,
	WalkerAction_nobody, //TODO WalkerAction_enemy53,
	WalkerAction_nobody, //TODO WalkerAction_enemy54,
	WalkerAction_nobody,
	WalkerAction_nobody,
	WalkerAction_nobody, //TODO WalkerAction_legionaryCaesar,
	WalkerAction_nobody, //TODO WalkerAction_nativeTrader,
	WalkerAction_arrow,
	WalkerAction_javelin, //60
	WalkerAction_bolt,
	WalkerAction_nobody, //TODO WalkerAction_ballista,
	WalkerAction_nobody,
	WalkerAction_missionary,
	WalkerAction_seagulls,
	WalkerAction_deliveryBoy,
	WalkerAction_shipwreck,
	WalkerAction_sheep,
	WalkerAction_wolf,
	WalkerAction_zebra, //70
	WalkerAction_spear,
	WalkerAction_nobody, //TODO WalkerAction_hippodromeHorse,
	WalkerAction_nobody,
	WalkerAction_nobody,
	WalkerAction_nobody,
	WalkerAction_nobody,
	WalkerAction_nobody,
	WalkerAction_nobody,
	WalkerAction_nobody
}; //80

void WalkerAction_handle()
{
	Data_CityInfo.numEnemiesInCity = 0;
	Data_CityInfo.numRiotersInCity = 0;
	Data_CityInfo.numAttackingNativesInCity = 0;
	Data_CityInfo.numAnimalsInCity = 0;
	Data_CityInfo.numImperialSoldiersInCity = 0;
	Data_CityInfo.numSoldiersInCity = 0;
	Data_CityInfo.entertainmentHippodromeHasShow = 0;
	if (Data_CityInfo.riotersOrAttackingNativesInCity > 0) {
		Data_CityInfo.riotersOrAttackingNativesInCity--;
	}
	for (int i = 1; i < MAX_WALKERS; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state) {
			if (w->targetedByWalkerId) {
				if (Data_Walkers[w->targetedByWalkerId].state != WalkerState_Alive) {
					w->targetedByWalkerId = 0;
				}
				if (Data_Walkers[w->targetedByWalkerId].targetWalkerId != i) {
					w->targetedByWalkerId = 0;
				}
			}
			walkerActionCallbacks[w->type](i);
			if (w->state == WalkerState_Dead) {
				Walker_delete(i);
			}
		}
	}
}

void WalkerAction_nobody(int walkerId)
{
}
