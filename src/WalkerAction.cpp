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
	WalkerAction_militaryStandard,
	WalkerAction_entertainer,
	WalkerAction_entertainer,
	WalkerAction_entertainer,
	WalkerAction_entertainer,
	WalkerAction_tradeCaravan,
	WalkerAction_tradeShip, //20
	WalkerAction_tradeCaravanDonkey,
	WalkerAction_protestor,
	WalkerAction_criminal,
	WalkerAction_rioter,
	WalkerAction_fishingBoat,
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
	WalkerAction_flotsam,
	WalkerAction_docker,
	WalkerAction_marketBuyer,
	WalkerAction_patrician, //40
	WalkerAction_indigenousNative,
	WalkerAction_towerSentry,
	WalkerAction_enemy43_Spear,
	WalkerAction_enemy44_Sword,
	WalkerAction_enemy45_Sword,
	WalkerAction_enemy46_Camel,
	WalkerAction_enemy47_Elephant,
	WalkerAction_enemy48_Chariot,
	WalkerAction_enemy49_FastSword,
	WalkerAction_enemy50_Sword, //50
	WalkerAction_enemy51_Spear,
	WalkerAction_enemy52_MountedArcher,
	WalkerAction_enemy53_Axe,
	WalkerAction_enemy54_Gladiator,
	WalkerAction_nobody,
	WalkerAction_nobody,
	WalkerAction_enemyCaesarLegionary,
	WalkerAction_nativeTrader,
	WalkerAction_arrow,
	WalkerAction_javelin, //60
	WalkerAction_bolt,
	WalkerAction_ballista,
	WalkerAction_nobody,
	WalkerAction_missionary,
	WalkerAction_seagulls,
	WalkerAction_deliveryBoy,
	WalkerAction_shipwreck,
	WalkerAction_sheep,
	WalkerAction_wolf,
	WalkerAction_zebra, //70
	WalkerAction_spear,
	WalkerAction_hippodromeHorse,
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
	for (int i = 1; i < MAX_FIGURES; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state) {
			if (w->targetedByWalkerId) {
				if (Data_Walkers[w->targetedByWalkerId].state != FigureState_Alive) {
					w->targetedByWalkerId = 0;
				}
				if (Data_Walkers[w->targetedByWalkerId].targetWalkerId != i) {
					w->targetedByWalkerId = 0;
				}
			}
			walkerActionCallbacks[w->type](i);
			if (w->state == FigureState_Dead) {
				Figure_delete(i);
			}
		}
	}
}

void WalkerAction_nobody(int walkerId)
{
}
