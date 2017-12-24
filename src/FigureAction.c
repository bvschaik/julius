#include "FigureAction_private.h"

#include "Figure.h"
#include "Data/CityInfo.h"
#include "figure/figure.h"
#include "figuretype/animal.h"
#include "figuretype/entertainer.h"
#include "figuretype/migrant.h"
#include "figuretype/wall.h"
#include "figuretype/water.h"

static void FigureAction_nobody(figure *f)
{
}

static void (*figureActionCallbacks[])(figure *f) = {
	FigureAction_nobody, //0
	figure_immigrant_action,
	figure_emigrant_action,
	figure_homeless_action,
	FigureAction_cartpusher,
	FigureAction_laborSeeker,
	FigureAction_explosionCloud,
	FigureAction_taxCollector,
	FigureAction_engineer,
	FigureAction_warehouseman,
	FigureAction_prefect, //10
	FigureAction_soldier,
	FigureAction_soldier,
	FigureAction_soldier,
	FigureAction_militaryStandard,
	figure_entertainer_action,
	figure_entertainer_action,
	figure_entertainer_action,
	figure_entertainer_action,
	FigureAction_tradeCaravan,
	FigureAction_tradeShip, //20
	FigureAction_tradeCaravanDonkey,
	FigureAction_protestor,
	FigureAction_criminal,
	FigureAction_rioter,
	figure_fishing_boat_action,
	FigureAction_marketTrader,
	FigureAction_priest,
	FigureAction_schoolChild,
	FigureAction_teacher,
	FigureAction_librarian, //30
	FigureAction_barber,
	FigureAction_bathhouseWorker,
	FigureAction_doctor,
	FigureAction_surgeon,
	FigureAction_worker,
	FigureAction_nobody, // mapFlag: editor only
	figure_flotsam_action,
	FigureAction_docker,
	FigureAction_marketBuyer,
	FigureAction_patrician, //40
	FigureAction_indigenousNative,
	figure_tower_sentry_action,
	FigureAction_enemy43_Spear,
	FigureAction_enemy44_Sword,
	FigureAction_enemy45_Sword,
	FigureAction_enemy46_Camel,
	FigureAction_enemy47_Elephant,
	FigureAction_enemy48_Chariot,
	FigureAction_enemy49_FastSword,
	FigureAction_enemy50_Sword, //50
	FigureAction_enemy51_Spear,
	FigureAction_enemy52_MountedArcher,
	FigureAction_enemy53_Axe,
	FigureAction_enemy54_Gladiator,
	FigureAction_nobody,
	FigureAction_nobody,
	FigureAction_enemyCaesarLegionary,
	FigureAction_nativeTrader,
	FigureAction_arrow,
	FigureAction_javelin, //60
	FigureAction_bolt,
	figure_ballista_action,
	FigureAction_nobody,
	FigureAction_missionary,
	figure_seagulls_action,
	FigureAction_deliveryBoy,
	figure_shipwreck_action,
	figure_sheep_action,
	figure_wolf_action,
	figure_zebra_action, //70
	FigureAction_spear,
	figure_hippodrome_horse_action,
	FigureAction_nobody,
	FigureAction_nobody,
	FigureAction_nobody,
	FigureAction_nobody,
	FigureAction_nobody,
	FigureAction_nobody,
	FigureAction_nobody
}; //80

void FigureAction_handle()
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
		figure *f = figure_get(i);
		if (f->state) {
			if (f->targetedByFigureId) {
                figure *attacker = figure_get(f->targetedByFigureId);
				if (attacker->state != FigureState_Alive) {
					f->targetedByFigureId = 0;
				}
				if (attacker->targetFigureId != i) {
					f->targetedByFigureId = 0;
				}
			}
			figureActionCallbacks[f->type](f);
			if (f->state == FigureState_Dead) {
				figure_delete(f);
			}
		}
	}
}
