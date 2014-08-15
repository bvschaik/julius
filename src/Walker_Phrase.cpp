#include "Building.h"
#include "Calc.h"
#include "Sound.h"
#include "WalkerAction.h"
#include "Data/Walker.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"

#include <string.h>

static char walkerSounds[32][20][32] = {
{ // 0
	"Vigils_starv1.wav", "Vigils_nojob1.wav", "Vigils_needjob1.wav", "Vigils_nofun1.wav",
	"Vigils_relig1.wav", "Vigils_great1.wav", "Vigils_great2.wav", "Vigils_exact1.wav",
	"Vigils_exact2.wav", "Vigils_exact3.wav", "Vigils_exact4.wav", "Vigils_exact5.wav",
	"Vigils_exact6.wav", "Vigils_exact7.wav", "Vigils_exact8.wav", "Vigils_exact9.wav",
	"Vigils_exact0.wav", "Vigils_free1.wav", "Vigils_free2.wav", "Vigils_free3.wav"
},
{ // 1
	"Wallguard_starv1.wav", "Wallguard_nojob1.wav", "Wallguard_needjob1.wav", "Wallguard_nofun1.wav",
	"Wallguard_relig1.wav", "Wallguard_great1.wav", "Wallguard_great2.wav", "Wallguard_exact1.wav",
	"Wallguard_exact2.wav", "Wallguard_exact3.wav", "Wallguard_exact4.wav", "Wallguard_exact5.wav",
	"Wallguard_exact6.wav", "Wallguard_exact7.wav", "Wallguard_exact8.wav", "Wallguard_exact9.wav",
	"Wallguard_exact0.wav", "Wallguard_free1.wav", "Wallguard_free2.wav", "Wallguard_free3.wav"
},
{ // 2
	"Engine_starv1.wav", "Engine_nojob1.wav", "Engine_needjob1.wav", "Engine_nofun1.wav",
	"Engine_relig1.wav", "Engine_great1.wav", "Engine_great2.wav", "Engine_exact1.wav",
	"Engine_exact2.wav", "Engine_exact3.wav", "Engine_exact4.wav", "Engine_exact5.wav",
	"Engine_exact6.wav", "Engine_exact7.wav", "Engine_exact8.wav", "Engine_exact9.wav",
	"Engine_exact0.wav", "Engine_free1.wav", "Engine_free2.wav", "Engine_free3.wav"
},
{ // 3
	"Taxman_starv1.wav", "Taxman_nojob1.wav", "Taxman_needjob1.wav", "Taxman_nofun1.wav",
	"Taxman_relig1.wav", "Taxman_great1.wav", "Taxman_great2.wav", "Taxman_exact1.wav",
	"Taxman_exact2.wav", "Taxman_exact3.wav", "Taxman_exact4.wav", "Taxman_exact5.wav",
	"Taxman_exact6.wav", "Taxman_exact7.wav", "Taxman_exact8.wav", "Taxman_exact9.wav",
	"Taxman_exact0.wav", "Taxman_free1.wav", "Taxman_free2.wav", "Taxman_free3.wav"
},
{ // 4
	"Market_starv1.wav", "Market_nojob1.wav", "Market_needjob1.wav", "Market_nofun1.wav",
	"Market_relig1.wav", "Market_great1.wav", "Market_great2.wav", "Market_exact2.wav",
	"Market_exact1.wav", "Market_exact3.wav", "Market_exact4.wav", "Market_exact5.wav",
	"Market_exact6.wav", "Market_exact7.wav", "Market_exact8.wav", "Market_exact9.wav",
	"Market_exact0.wav", "Market_free1.wav", "Market_free2.wav", "Market_free3.wav"
},
{ // 5
	"CrtPsh_starv1.wav", "CrtPsh_nojob1.wav", "CrtPsh_needjob1.wav", "CrtPsh_nofun1.wav",
	"CrtPsh_relig1.wav", "CrtPsh_great1.wav", "CrtPsh_great2.wav", "CrtPsh_exact1.wav",
	"CrtPsh_exact2.wav", "CrtPsh_exact3.wav", "CrtPsh_exact4.wav", "CrtPsh_exact5.wav",
	"CrtPsh_exact6.wav", "CrtPsh_exact7.wav", "CrtPsh_exact8.wav", "CrtPsh_exact9.wav",
	"CrtPsh_exact0.wav", "CrtPsh_free1.wav", "CrtPsh_free2.wav", "CrtPsh_free3.wav"
},
{ // 6
	"donkey_starv1.wav", "donkey_nojob1.wav", "donkey_needjob1.wav", "donkey_nofun1.wav",
	"donkey_relig1.wav", "donkey_great1.wav", "donkey_great2.wav", "donkey_exact1.wav",
	"donkey_exact2.wav", "donkey_exact3.wav", "donkey_exact4.wav", "donkey_exact5.wav",
	"donkey_exact6.wav", "donkey_exact7.wav", "donkey_exact8.wav", "donkey_exact9.wav",
	"donkey_exact0.wav", "donkey_free1.wav", "donkey_free2.wav", "donkey_free3.wav"
},
{ // 7
	"boats_starv1.wav", "boats_nojob1.wav", "boats_needjob1.wav", "boats_nofun1.wav",
	"boats_relig1.wav", "boats_great1.wav", "boats_great2.wav", "boats_exact2.wav",
	"boats_exact1.wav", "boats_exact3.wav", "boats_exact4.wav", "boats_exact5.wav",
	"boats_exact6.wav", "boats_exact7.wav", "boats_exact8.wav", "boats_exact9.wav",
	"boats_exact0.wav", "boats_free1.wav", "boats_free2.wav", "boats_free3.wav"
},
{ // 8
	"Priest_starv1.wav", "Priest_nojob1.wav", "Priest_needjob1.wav", "Priest_nofun1.wav",
	"Priest_relig1.wav", "Priest_great1.wav", "Priest_great2.wav", "Priest_exact1.wav",
	"Priest_exact2.wav", "Priest_exact3.wav", "Priest_exact4.wav", "Priest_exact5.wav",
	"Priest_exact6.wav", "Priest_exact7.wav", "Priest_exact8.wav", "Priest_exact9.wav",
	"Priest_exact0.wav", "Priest_free1.wav", "Priest_free2.wav", "Priest_free3.wav"
},
{ // 9
	"Teach_starv1.wav", "Teach_nojob1.wav", "Teach_needjob1.wav", "Teach_nofun1.wav",
	"Teach_relig1.wav", "Teach_great1.wav", "Teach_great2.wav", "Teach_exact1.wav",
	"Teach_exact2.wav", "Teach_exact3.wav", "Teach_exact4.wav", "Teach_exact5.wav",
	"Teach_exact6.wav", "Teach_exact7.wav", "Teach_exact8.wav", "Teach_exact9.wav",
	"Teach_exact0.wav", "Teach_free1.wav", "Teach_free2.wav", "Teach_free3.wav"
},
{ // 10
	"Pupils_starv1.wav", "Pupils_nojob1.wav", "Pupils_needjob1.wav", "Pupils_nofun1.wav",
	"Pupils_relig1.wav", "Pupils_great1.wav", "Pupils_great2.wav", "Pupils_exact1.wav",
	"Pupils_exact2.wav", "Pupils_exact3.wav", "Pupils_exact4.wav", "Pupils_exact5.wav",
	"Pupils_exact6.wav", "Pupils_exact7.wav", "Pupils_exact8.wav", "Pupils_exact9.wav",
	"Pupils_exact0.wav", "Pupils_free1.wav", "Pupils_free2.wav", "Pupils_free3.wav"
},
{ // 11
	"Bather_starv1.wav", "Bather_nojob1.wav", "Bather_needjob1.wav", "Bather_nofun1.wav",
	"Bather_relig1.wav", "Bather_great1.wav", "Bather_great2.wav", "Bather_exact1.wav",
	"Bather_exact2.wav", "Bather_exact3.wav", "Bather_exact4.wav", "Bather_exact5.wav",
	"Bather_exact6.wav", "Bather_exact7.wav", "Bather_exact8.wav", "Bather_exact9.wav",
	"Bather_exact0.wav", "Bather_free1.wav", "Bather_free2.wav", "Bather_free3.wav"
},
{ // 12
	"Doctor_starv1.wav", "Doctor_nojob1.wav", "Doctor_needjob1.wav", "Doctor_nofun1.wav",
	"Doctor_relig1.wav", "Doctor_great1.wav", "Doctor_great2.wav", "Doctor_exact1.wav",
	"Doctor_exact2.wav", "Doctor_exact3.wav", "Doctor_exact4.wav", "Doctor_exact5.wav",
	"Doctor_exact6.wav", "Doctor_exact7.wav", "Doctor_exact8.wav", "Doctor_exact9.wav",
	"Doctor_exact0.wav", "Doctor_free1.wav", "Doctor_free2.wav", "Doctor_free3.wav"
},
{ // 13
	"barber_starv1.wav", "barber_nojob1.wav", "barber_needjob1.wav", "barber_nofun1.wav",
	"barber_relig1.wav", "barber_great1.wav", "barber_great2.wav", "barber_exact1.wav",
	"barber_exact2.wav", "barber_exact3.wav", "barber_exact4.wav", "barber_exact5.wav",
	"barber_exact6.wav", "barber_exact7.wav", "barber_exact8.wav", "barber_exact9.wav",
	"barber_exact0.wav", "barber_free1.wav", "barber_free2.wav", "barber_free3.wav"
},
{ // 14
	"Actors_starv1.wav", "Actors_nojob1.wav", "Actors_needjob1.wav", "Actors_nofun1.wav",
	"Actors_relig1.wav", "Actors_great1.wav", "Actors_great2.wav", "Actors_exact1.wav",
	"Actors_exact2.wav", "Actors_exact3.wav", "Actors_exact4.wav", "Actors_exact5.wav",
	"Actors_exact6.wav", "Actors_exact7.wav", "Actors_exact8.wav", "Actors_exact9.wav",
	"Actors_exact0.wav", "Actors_free1.wav", "Actors_free2.wav", "Actors_free3.wav"
},
{ // 15
	"Gladtr_starv1.wav", "Gladtr_nojob1.wav", "Gladtr_needjob1.wav", "Gladtr_nofun1.wav",
	"Gladtr_relig1.wav", "Gladtr_great1.wav", "Gladtr_great2.wav", "Gladtr_exact1.wav",
	"Gladtr_exact2.wav", "Gladtr_exact3.wav", "Gladtr_exact4.wav", "Gladtr_exact5.wav",
	"Gladtr_exact6.wav", "Gladtr_exact7.wav", "Gladtr_exact8.wav", "Gladtr_exact9.wav",
	"Gladtr_exact0.wav", "Gladtr_free1.wav", "Gladtr_free2.wav", "Gladtr_free3.wav"
},
{ // 16
	"LionTr_starv1.wav", "LionTr_nojob1.wav", "LionTr_needjob1.wav", "LionTr_nofun1.wav",
	"LionTr_relig1.wav", "LionTr_great1.wav", "LionTr_great2.wav", "LionTr_exact1.wav",
	"LionTr_exact2.wav", "LionTr_exact3.wav", "LionTr_exact4.wav", "LionTr_exact5.wav",
	"LionTr_exact6.wav", "LionTr_exact7.wav", "LionTr_exact8.wav", "LionTr_exact9.wav",
	"LionTr_exact0.wav", "LionTr_free1.wav", "LionTr_free2.wav", "LionTr_free3.wav"
},
{ // 17
	"Charot_starv1.wav", "Charot_nojob1.wav", "Charot_needjob1.wav", "Charot_nofun1.wav",
	"Charot_relig1.wav", "Charot_great1.wav", "Charot_great2.wav", "Charot_exact1.wav",
	"Charot_exact2.wav", "Charot_exact3.wav", "Charot_exact4.wav", "Charot_exact5.wav",
	"Charot_exact6.wav", "Charot_exact7.wav", "Charot_exact8.wav", "Charot_exact9.wav",
	"Charot_exact0.wav", "Charot_free1.wav", "Charot_free2.wav", "Charot_free3.wav"
},
{ // 18
	"Patric_starv1.wav", "Patric_nojob1.wav", "Patric_needjob1.wav", "Patric_nofun1.wav",
	"Patric_relig1.wav", "Patric_great1.wav", "Patric_great2.wav", "Patric_exact1.wav",
	"Patric_exact2.wav", "Patric_exact3.wav", "Patric_exact4.wav", "Patric_exact5.wav",
	"Patric_exact6.wav", "Patric_exact7.wav", "Patric_exact8.wav", "Patric_exact9.wav",
	"Patric_exact0.wav", "Patric_free1.wav", "Patric_free2.wav", "Patric_free3.wav"
},
{ // 19
	"Pleb_starv1.wav", "Pleb_nojob1.wav", "Pleb_needjob1.wav", "Pleb_nofun1.wav",
	"Pleb_relig1.wav", "Pleb_great1.wav", "Pleb_great2.wav", "Pleb_exact1.wav",
	"Pleb_exact2.wav", "Pleb_exact3.wav", "Pleb_exact4.wav", "Pleb_exact5.wav",
	"Pleb_exact6.wav", "Pleb_exact7.wav", "Pleb_exact8.wav", "Pleb_exact9.wav",
	"Pleb_exact0.wav", "Pleb_free1.wav", "Pleb_free2.wav", "Pleb_free3.wav"
},
{ // 20
	"Rioter_starv1.wav", "Rioter_nojob1.wav", "Rioter_needjob1.wav", "Rioter_nofun1.wav",
	"Rioter_relig1.wav", "Rioter_great1.wav", "Rioter_great2.wav", "Rioter_exact1.wav",
	"Rioter_exact2.wav", "Rioter_exact3.wav", "Rioter_exact4.wav", "Rioter_exact5.wav",
	"Rioter_exact6.wav", "Rioter_exact7.wav", "Rioter_exact8.wav", "Rioter_exact9.wav",
	"Rioter_exact0.wav", "Rioter_free1.wav", "Rioter_free2.wav", "Rioter_free3.wav"
},
{ // 21
	"homeless_starv1.wav", "homeless_nojob1.wav", "homeless_needjob1.wav", "homeless_nofun1.wav",
	"homeless_relig1.wav", "homeless_great1.wav", "homeless_great2.wav", "homeless_exact1.wav",
	"homeless_exact2.wav", "homeless_exact3.wav", "homeless_exact4.wav", "homeless_exact5.wav",
	"homeless_exact6.wav", "homeless_exact7.wav", "homeless_exact8.wav", "homeless_exact9.wav",
	"homeless_exact0.wav", "homeless_free1.wav", "homeless_free2.wav", "homeless_free3.wav"
},
{ // 22
	"unemploy_starv1.wav", "unemploy_nojob1.wav", "unemploy_needjob1.wav", "unemploy_nofun1.wav",
	"unemploy_relig1.wav", "unemploy_great1.wav", "unemploy_great2.wav", "unemploy_exact1.wav",
	"unemploy_exact2.wav", "unemploy_exact3.wav", "unemploy_exact4.wav", "unemploy_exact5.wav",
	"unemploy_exact6.wav", "unemploy_exact7.wav", "unemploy_exact8.wav", "unemploy_exact9.wav",
	"unemploy_exact0.wav", "unemploy_free1.wav", "unemploy_free2.wav", "unemploy_free3.wav"
},
{ // 23
	"Emigrate_starv1.wav", "Emigrate_nojob1.wav", "Emigrate_needjob1.wav", "Emigrate_nofun1.wav",
	"Emigrate_relig1.wav", "Emigrate_great1.wav", "Emigrate_great2.wav", "Emigrate_exact1.wav",
	"Emigrate_exact2.wav", "Emigrate_exact3.wav", "Emigrate_exact4.wav", "Emigrate_exact5.wav",
	"Emigrate_exact6.wav", "Emigrate_exact7.wav", "Emigrate_exact8.wav", "Emigrate_exact9.wav",
	"Emigrate_exact0.wav", "Emigrate_free1.wav", "Emigrate_free2.wav", "Emigrate_free3.wav"
},
{ // 24
	"Immigrant_starv1.wav", "Immigrant_nojob1.wav", "Immigrant_needjob1.wav", "Immigrant_nofun1.wav",
	"Immigrant_relig1.wav", "Immigrant_great1.wav", "Immigrant_great2.wav", "Immigrant_exact1.wav",
	"Immigrant_exact2.wav", "Immigrant_exact3.wav", "Immigrant_exact4.wav", "Immigrant_exact5.wav",
	"Immigrant_exact6.wav", "Immigrant_exact7.wav", "Immigrant_exact8.wav", "Immigrant_exact9.wav",
	"Immigrant_exact0.wav", "Immigrant_free1.wav", "Immigrant_free2.wav", "Immigrant_free3.wav"
},
{ // 25
	"Enemy_starv1.wav", "Enemy_nojob1.wav", "Enemy_needjob1.wav", "Enemy_nofun1.wav",
	"Enemy_relig1.wav", "Enemy_great1.wav", "Enemy_great2.wav", "Enemy_exact1.wav",
	"Enemy_exact2.wav", "Enemy_exact3.wav", "Enemy_exact4.wav", "Enemy_exact5.wav",
	"Enemy_exact6.wav", "Enemy_exact7.wav", "Enemy_exact8.wav", "Enemy_exact9.wav",
	"Enemy_exact0.wav", "Enemy_free1.wav", "Enemy_free2.wav", "Enemy_free3.wav"
},
{ // 26
	"Local_starv1.wav", "Local_nojob1.wav", "Local_needjob1.wav", "Local_nofun1.wav",
	"Local_relig1.wav", "Local_great1.wav", "Local_great2.wav", "Local_exact1.wav",
	"Local_exact2.wav", "Local_exact3.wav", "Local_exact4.wav", "Local_exact5.wav",
	"Local_exact6.wav", "Local_exact7.wav", "Local_exact8.wav", "Local_exact9.wav",
	"Local_exact0.wav", "Local_free1.wav", "Local_free2.wav", "Local_free3.wav"
},
{ // 27
	"Libary_starv1.wav", "Libary_nojob1.wav", "Libary_needjob1.wav", "Libary_nofun1.wav",
	"Libary_relig1.wav", "Libary_great1.wav", "Libary_great2.wav", "Libary_exact1.wav",
	"Libary_exact2.wav", "Libary_exact3.wav", "Libary_exact4.wav", "Libary_exact5.wav",
	"Libary_exact6.wav", "Libary_exact7.wav", "Libary_exact8.wav", "Libary_exact9.wav",
	"Libary_exact0.wav", "Libary_free1.wav", "Libary_free2.wav", "Libary_free3.wav"
},
{ // 28
	"srgeon_starv1.wav", "srgeon_nojob1.wav", "srgeon_needjob1.wav", "srgeon_nofun1.wav",
	"srgeon_relig1.wav", "srgeon_great1.wav", "srgeon_great2.wav", "srgeon_exact1.wav",
	"srgeon_exact2.wav", "srgeon_exact3.wav", "srgeon_exact4.wav", "srgeon_exact5.wav",
	"srgeon_exact6.wav", "srgeon_exact7.wav", "srgeon_exact8.wav", "srgeon_exact9.wav",
	"srgeon_exact0.wav", "srgeon_free1.wav", "srgeon_free2.wav", "srgeon_free3.wav"
},
{ // 29
	"docker_starv1.wav", "docker_nojob1.wav", "docker_needjob1.wav", "docker_nofun1.wav",
	"docker_relig1.wav", "docker_great1.wav", "docker_great2.wav", "docker_exact1.wav",
	"docker_exact2.wav", "docker_exact3.wav", "docker_exact4.wav", "docker_exact5.wav",
	"docker_exact6.wav", "docker_exact7.wav", "docker_exact8.wav", "docker_exact9.wav",
	"docker_exact0.wav", "docker_free1.wav", "docker_free2.wav", "docker_free3.wav"
},
{ // 30
	"missionary_starv1.wav", "missionary_nojob1.wav", "missionary_needjob1.wav", "missionary_nofun1.wav",
	"missionary_relig1.wav", "missionary_great1.wav", "missionary_great2.wav", "missionary_exact1.wav",
	"missionary_exact2.wav", "missionary_exact3.wav", "missionary_exact4.wav", "missionary_exact5.wav",
	"missionary_exact6.wav", "missionary_exact7.wav", "missionary_exact8.wav", "missionary_exact9.wav",
	"missionary_exact0.wav", "missionary_free1.wav", "missionary_free2.wav", "missionary_free3.wav"
},
{ // 31
	"granboy_starv1.wav", "granboy_nojob1.wav", "granboy_needjob1.wav", "granboy_nofun1.wav",
	"granboy_relig1.wav", "granboy_great1.wav", "granboy_great2.wav", "granboy_exact1.wav",
	"granboy_exact2.wav", "granboy_exact3.wav", "granboy_exact4.wav", "granboy_exact5.wav",
	"granboy_exact6.wav", "granboy_exact7.wav", "granboy_exact8.wav", "granboy_exact9.wav",
	"granboy_exact0.wav", "granboy_free1.wav", "granboy_free2.wav", "granboy_free3.wav"
}
};

static int walkerTypeToSoundType[] = {
	-1, 24, 23, 21, 5, 19, -1, 3, 2, 5, // 0-9
	0, 1, 1, 1, -1, 14, 15, 16, 17, 6, // 10-19
	7, -1, 20, 20, 20, -1, 4, 8, 10, 9, // 20-29
	9, 13, 11, 12, 12, 19, -1, -1, 5, 4, // 30-39
	18, -1, 1, 25, 25, 25, 25, 25, 25, 25, // 40-49
	25, 25, 25, 25, 25, 25, 25, 25, -1, -1, // 50-59
	-1, -1, -1, -1, 30, -1, 31, -1, -1, -1, // 60-69
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1 // 70-79
};

int Walker_determinePhrase(int walkerId)
{
	if (walkerId <= 0) {
		return 0;
	}

	Data_Walker *w = &Data_Walkers[walkerId];
	int phraseId = w->phraseId = 0;

	if (WalkerIsEnemyOrNative(w->type)) {
		return w->phraseId = -1;
	}

	// phrase id based on walker state
	switch (w->type) {
		case Walker_LaborSeeker:
		case Walker_35:
		case Walker_Actor:
		case Walker_Charioteer:
		case Walker_SchoolChild:
		case Walker_BathhouseWorker:
		case Walker_Barber:
		case Walker_Doctor:
		case Walker_Surgeon:
		case Walker_Priest:
		case Walker_Patrician:
		case Walker_Teacher:
		case Walker_Librarian:
			break;
		case Walker_LionTamer:
			if (w->actionState == WalkerActionState_150_Attack) {
				if (++w->phraseSequenceExact >= 3) {
					w->phraseSequenceExact = 0;
				}
				phraseId = 7 + w->phraseSequenceExact;
			}
			break;
		case Walker_Gladiator:
			if (w->actionState == WalkerActionState_150_Attack) {
				phraseId = 7;
			}
			break;
		case Walker_TaxCollector:
			if (w->minMaxSeen >= 7) {
				phraseId = 7;
			} else if (w->minMaxSeen >= 4) {
				phraseId = 8;
			} else if (w->minMaxSeen >= 1) {
				phraseId = 9;
			}
			break;
		case Walker_MarketTrader:
			if (w->actionState == WalkerActionState_126_RoamerReturning) {
				if (Building_Market_getMaxFoodStock(w->buildingId) <= 0) {
					phraseId = 9; // run out of goods
				}
			}
			break;
		case Walker_MarketBuyer:
			if (w->actionState == WalkerActionState_145_MarketBuyerCreated) {
				phraseId = 7;
			} else if (w->actionState == WalkerActionState_146_MarketBuyer) {
				phraseId = 8;
			}
			break;
		case Walker_CartPusher:
			if (w->actionState == WalkerActionState_20_CartpusherInitial) {
				if (w->minMaxSeen == 2) {
					phraseId = 7;
				} else if (w->minMaxSeen == 1) {
					phraseId = 8;
				}
			} else if (w->actionState == WalkerActionState_21_CartpusherDeliveringToWarehouse ||
					w->actionState == WalkerActionState_22_CartpusherDeliveringToGranary ||
					w->actionState == WalkerActionState_23_CartpusherDeliveringToWorkshop) {
				if (Calc_distanceMaximum(
					w->destinationX, w->destinationY, w->sourceX, w->sourceY) >= 25) {
					phraseId = 9; // too far
				}
			}
			break;
		case Walker_Warehouseman:
			if (w->actionState == WalkerActionState_51_WarehousemanDeliveringResource) {
				if (Calc_distanceMaximum(
					w->destinationX, w->destinationY, w->sourceX, w->sourceY) >= 25) {
					phraseId = 9; // too far
				}
			}
			break;
		case Walker_Prefect:
			if (++w->phraseSequenceExact >= 4) {
				w->phraseSequenceExact = 0;
			}
			if (w->actionState == WalkerActionState_74_PrefectGoingToFire) {
				phraseId = 10;
			} else if (w->actionState == WalkerActionState_75_PrefectAtFire) {
				phraseId = 11 + (w->phraseSequenceExact % 2);
			} else if (w->actionState == WalkerActionState_150_Attack) {
				phraseId = 13 + w->phraseSequenceExact;
			} else if (w->minMaxSeen >= 50) {
				phraseId = 7;
			} else if (w->minMaxSeen >= 10) {
				phraseId = 8;
			} else {
				phraseId = 9;
			}
			break;
		case Walker_Engineer:
			if (w->minMaxSeen >= 60) {
				phraseId = 7;
			} else if (w->minMaxSeen >= 10) {
				phraseId = 8;
			}
			break;
		case Walker_Protester:
		case Walker_Criminal:
		case Walker_Rioter:
		case Walker_DeliveryBoy:
		case Walker_Missionary:
			if (++w->phraseSequenceExact >= 3) {
				w->phraseSequenceExact = 0;
			}
			phraseId = 7 + w->phraseSequenceExact;
			break;
		case Walker_Homeless:
		case Walker_Immigrant:
			if (++w->phraseSequenceExact >= 2) {
				w->phraseSequenceExact = 0;
			}
			phraseId = 7 + w->phraseSequenceExact;
			break;
		case Walker_Emigrant:
			switch (Data_CityInfo.populationEmigrationCause) {
				case EmigrationCause_NoFood: phraseId = 8; break;
				case EmigrationCause_NoJobs: phraseId = 7; break;
				case EmigrationCause_HighTaxes: phraseId = 9; break;
				case EmigrationCause_LowWages: phraseId = 10; break;
				default: phraseId = 11; break;
			}
			break;
		case Walker_TowerSentry:
			if (++w->phraseSequenceExact >= 2) {
				w->phraseSequenceExact = 0;
			}
			if (!Data_CityInfo.numEnemiesInCity) {
				phraseId = 7 + w->phraseSequenceExact;
			} else if (Data_CityInfo.numEnemiesInCity <= 10) {
				phraseId = 9;
			} else if (Data_CityInfo.numEnemiesInCity <= 30) {
				phraseId = 10;
			} else {
				phraseId = 11;
			}
			break;
		case Walker_FortJavelin:
		case Walker_FortMounted:
		case Walker_FortLegionary:
			if (Data_CityInfo.numEnemiesInCity >= 40) {
				phraseId = 11;
			} else if (Data_CityInfo.numEnemiesInCity > 20) {
				phraseId = 10;
			} else if (Data_CityInfo.numEnemiesInCity) {
				phraseId = 9;
			}
			break;
		case Walker_Dockman:
			if (w->actionState == WalkerActionState_135_Dockman ||
				w->actionState == WalkerActionState_136_Dockman) {
				if (Calc_distanceMaximum(
					w->destinationX, w->destinationY, w->sourceX, w->sourceY) >= 25) {
					phraseId = 9; // too far?
				}
			}
			break;
		case Walker_TradeCaravan:
			if (++w->phraseSequenceExact >= 2) {
				w->phraseSequenceExact = 0;
			}
			phraseId = 8 + w->phraseSequenceExact;
			if (w->actionState == WalkerActionState_103_TradeCaravanLeaving) {
				if (!Data_Walker_Traders[w->traderId].totalSold &&
					!Data_Walker_Traders[w->traderId].totalBought) {
					phraseId = 7; // no trade
				}
			} else if (w->actionState == WalkerActionState_102_TradeCaravanTrading) {
				if (WalkerAction_TradeCaravan_canBuy(walkerId, w->destinationBuildingId, w->empireCityId)) {
					phraseId = 11; // buying goods
				} else if (WalkerAction_TradeCaravan_canSell(walkerId, w->destinationBuildingId, w->empireCityId)) {
					phraseId = 10; // selling goods
				}
			}
			break;
		case Walker_TradeShip:
			if (w->actionState == WalkerActionState_115_TradeShipLeaving) {
				if (!Data_Walker_Traders[w->traderId].totalSold &&
					!Data_Walker_Traders[w->traderId].totalBought) {
					phraseId = 9; // no trade
				} else {
					phraseId = 11; // good trade
				}
			} else if (w->actionState == WalkerActionState_112_TradeShipMoored) {
				int ship = WalkerAction_TradeShip_canBuyOrSell(walkerId);
				if (ship == 1) {
					phraseId = 8; // buying goods
				} else if (ship == 2) {
					phraseId = 7; // selling goods
				} else {
					phraseId = 9; // no trade
				}
			} else {
				phraseId = 10; // can't wait to trade
			}
			break;
	}

	if (phraseId) {
		w->phraseId = phraseId;
		return phraseId;
	}

	// phrase id based on city state
	if (++w->phraseSequenceCity >= 1) {
		w->phraseSequenceCity = 0;
	}

	int leastGodHappiness = 100;
	for (int i = 0; i < 5; i++) {
		if (Data_CityInfo.godHappiness[i] < leastGodHappiness) {
			leastGodHappiness = Data_CityInfo.godHappiness[i];
		}
	}
	int godPhraseId;
	if (leastGodHappiness < 20) {
		godPhraseId = 1;
	} else if (leastGodHappiness < 40) {
		godPhraseId = 2;
	} else {
		godPhraseId = 0;
	}

	phraseId = -1;
	if (Data_CityInfo.foodInfoFoodSupplyMonths <= 0) {
		phraseId = w->phraseSequenceCity;
	} else if (Data_CityInfo.unemploymentPercentage >= 17) {
		phraseId = 1 + w->phraseSequenceCity;
	} else if (Data_CityInfo.workersNeeded >= 10) {
		phraseId = 2 + w->phraseSequenceCity;
	} else if (Data_CityInfo.citywideAverageEntertainment == 0) {
		phraseId = 3 + w->phraseSequenceCity;
	} else if (godPhraseId == 1) {
		phraseId = 4 + w->phraseSequenceCity;
	} else if (Data_CityInfo.citywideAverageEntertainment <= 10) {
		phraseId = 3 + w->phraseSequenceCity;
	} else if (godPhraseId == 2) {
		phraseId = 4 + w->phraseSequenceCity;
	} else if (Data_CityInfo.citywideAverageEntertainment <= 20) {
		phraseId = 3 + w->phraseSequenceCity;
	} else if (Data_CityInfo.foodInfoFoodSupplyMonths >= 4 &&
			Data_CityInfo.unemploymentPercentage <= 5 &&
			Data_CityInfo.citywideAverageHealth > 0 &&
			Data_CityInfo.citywideAverageEducation > 0) {
		if (Data_CityInfo.population < 500) {
			phraseId = 5 + w->phraseSequenceCity;
		} else {
			phraseId = 6 + w->phraseSequenceCity;
		}
	} else if (Data_CityInfo.unemploymentPercentage >= 10) {
		phraseId = 1 + w->phraseSequenceCity;
	} else {
		phraseId = 5 + w->phraseSequenceCity;
	}
	return w->phraseId = phraseId;
}

static void playWalkerSoundFile(int walkerSoundId, int phraseId)
{
	char path[32];
	if (walkerSoundId >= 0 && phraseId >= 0) {
		strcpy(path, "wavs/");
		strcat(path, walkerSounds[walkerSoundId][phraseId]);
		Sound_Speech_playFile(path);
	}
}

int Walker_playPhrase(int walkerId)
{
	if (walkerId > 0) {
		int walkerSoundId = walkerTypeToSoundType[Data_Walkers[walkerId].type];
		playWalkerSoundFile(walkerSoundId, Data_Walkers[walkerId].phraseId);
		return walkerSoundId;
	} else {
		return 0;
	}
}

void Walker_playDieSound(int walkerType)
{
	int isSoldier = 0;
	int isCitizen = 0;
	switch (walkerType) {
		case Walker_Wolf:
			Sound_Effects_playChannel(SoundChannel_WolfDie);
			break;
		case Walker_Sheep:
			Sound_Effects_playChannel(SoundChannel_SheepDie);
			break;
		case Walker_Zebra:
			Sound_Effects_playChannel(SoundChannel_ZebraDie);
			break;
		case Walker_LionTamer:
			Sound_Effects_playChannel(SoundChannel_LionDie);
			break;
		case Walker_Enemy48_Horse:
		case Walker_Enemy52_Horse:
			Sound_Effects_playChannel(SoundChannel_Horse2);
			break;
		case Walker_Enemy46_Camel:
			Sound_Effects_playChannel(SoundChannel_Camel);
			break;
		case Walker_Enemy47_Elephant:
			Sound_Effects_playChannel(SoundChannel_ElephantDie);
			break;
		case Walker_NativeTrader:
		case Walker_TradeCaravan:
		case Walker_TradeCaravanDonkey:
			break;
		case Walker_Prefect:
		case Walker_FortJavelin:
		case Walker_FortMounted:
		case Walker_FortLegionary:
		case Walker_Gladiator:
		case Walker_IndigenousNative:
		case Walker_TowerSentry:
		case Walker_Enemy43:
		case Walker_Enemy44:
		case Walker_Enemy45:
		case Walker_Enemy49:
		case Walker_Enemy50:
		case Walker_Enemy51:
		case Walker_Enemy53:
		case Walker_Enemy54:
		case Walker_Enemy55_Javelin:
		case Walker_Enemy56_Mounted:
		case Walker_EnemyCaesarLegionary:
			isSoldier = 1;
			break;
		default:
			isCitizen = 1;
			break;
	}
	if (isSoldier) {
		Data_CityInfo.dieSoundSoldier++;
		if (Data_CityInfo.dieSoundSoldier >= 4) {
			Data_CityInfo.dieSoundSoldier = 0;
		}
		Sound_Effects_playChannel(SoundChannel_SoldierDie + Data_CityInfo.dieSoundSoldier);
	} else if (isCitizen) {
		Data_CityInfo.dieSoundCitizen++;
		if (Data_CityInfo.dieSoundCitizen >= 4) {
			Data_CityInfo.dieSoundCitizen = 0;
		}
		Sound_Effects_playChannel(SoundChannel_CitizenDie + Data_CityInfo.dieSoundCitizen);
	}
	if (WalkerIsEnemy(walkerType)) {
		if (Data_CityInfo.numEnemiesInCity == 1) {
			Sound_Speech_playFile("wavs/Army_war_cry.wav");
		}
	} else if (WalkerIsLegion(walkerType)) {
		if (Data_CityInfo.numSoldiersInCity == 1) {
			Sound_Speech_playFile("wavs/barbarian_war_cry.wav");
		}
	}
}
