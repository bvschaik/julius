#include "Building.h"
#include "Calc.h"
#include "Sound.h"
#include "WalkerAction.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Walker.h"

#include <string.h>

static char walkerSounds[32][20][32] = {
{ // 0
	"vigils_starv1.wav", "vigils_nojob1.wav", "vigils_needjob1.wav", "vigils_nofun1.wav",
	"vigils_relig1.wav", "vigils_great1.wav", "vigils_great2.wav", "vigils_exact1.wav",
	"vigils_exact2.wav", "vigils_exact3.wav", "vigils_exact4.wav", "vigils_exact5.wav",
	"vigils_exact6.wav", "vigils_exact7.wav", "vigils_exact8.wav", "vigils_exact9.wav",
	"vigils_exact0.wav", "vigils_free1.wav", "vigils_free2.wav", "vigils_free3.wav"
},
{ // 1
	"wallguard_starv1.wav", "wallguard_nojob1.wav", "wallguard_needjob1.wav", "wallguard_nofun1.wav",
	"wallguard_relig1.wav", "wallguard_great1.wav", "wallguard_great2.wav", "wallguard_exact1.wav",
	"wallguard_exact2.wav", "wallguard_exact3.wav", "wallguard_exact4.wav", "wallguard_exact5.wav",
	"wallguard_exact6.wav", "wallguard_exact7.wav", "wallguard_exact8.wav", "wallguard_exact9.wav",
	"wallguard_exact0.wav", "wallguard_free1.wav", "wallguard_free2.wav", "wallguard_free3.wav"
},
{ // 2
	"engine_starv1.wav", "engine_nojob1.wav", "engine_needjob1.wav", "engine_nofun1.wav",
	"engine_relig1.wav", "engine_great1.wav", "engine_great2.wav", "engine_exact1.wav",
	"engine_exact2.wav", "engine_exact3.wav", "engine_exact4.wav", "engine_exact5.wav",
	"engine_exact6.wav", "engine_exact7.wav", "engine_exact8.wav", "engine_exact9.wav",
	"engine_exact0.wav", "engine_free1.wav", "engine_free2.wav", "engine_free3.wav"
},
{ // 3
	"taxman_starv1.wav", "taxman_nojob1.wav", "taxman_needjob1.wav", "taxman_nofun1.wav",
	"taxman_relig1.wav", "taxman_great1.wav", "taxman_great2.wav", "taxman_exact1.wav",
	"taxman_exact2.wav", "taxman_exact3.wav", "taxman_exact4.wav", "taxman_exact5.wav",
	"taxman_exact6.wav", "taxman_exact7.wav", "taxman_exact8.wav", "taxman_exact9.wav",
	"taxman_exact0.wav", "taxman_free1.wav", "taxman_free2.wav", "taxman_free3.wav"
},
{ // 4
	"market_starv1.wav", "market_nojob1.wav", "market_needjob1.wav", "market_nofun1.wav",
	"market_relig1.wav", "market_great1.wav", "market_great2.wav", "market_exact2.wav",
	"market_exact1.wav", "market_exact3.wav", "market_exact4.wav", "market_exact5.wav",
	"market_exact6.wav", "market_exact7.wav", "market_exact8.wav", "market_exact9.wav",
	"market_exact0.wav", "market_free1.wav", "market_free2.wav", "market_free3.wav"
},
{ // 5
	"crtpsh_starv1.wav", "crtpsh_nojob1.wav", "crtpsh_needjob1.wav", "crtpsh_nofun1.wav",
	"crtpsh_relig1.wav", "crtpsh_great1.wav", "crtpsh_great2.wav", "crtpsh_exact1.wav",
	"crtpsh_exact2.wav", "crtpsh_exact3.wav", "crtpsh_exact4.wav", "crtpsh_exact5.wav",
	"crtpsh_exact6.wav", "crtpsh_exact7.wav", "crtpsh_exact8.wav", "crtpsh_exact9.wav",
	"crtpsh_exact0.wav", "crtpsh_free1.wav", "crtpsh_free2.wav", "crtpsh_free3.wav"
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
	"priest_starv1.wav", "priest_nojob1.wav", "priest_needjob1.wav", "priest_nofun1.wav",
	"priest_relig1.wav", "priest_great1.wav", "priest_great2.wav", "priest_exact1.wav",
	"priest_exact2.wav", "priest_exact3.wav", "priest_exact4.wav", "priest_exact5.wav",
	"priest_exact6.wav", "priest_exact7.wav", "priest_exact8.wav", "priest_exact9.wav",
	"priest_exact0.wav", "priest_free1.wav", "priest_free2.wav", "priest_free3.wav"
},
{ // 9
	"teach_starv1.wav", "teach_nojob1.wav", "teach_needjob1.wav", "teach_nofun1.wav",
	"teach_relig1.wav", "teach_great1.wav", "teach_great2.wav", "teach_exact1.wav",
	"teach_exact2.wav", "teach_exact3.wav", "teach_exact4.wav", "teach_exact5.wav",
	"teach_exact6.wav", "teach_exact7.wav", "teach_exact8.wav", "teach_exact9.wav",
	"teach_exact0.wav", "teach_free1.wav", "teach_free2.wav", "teach_free3.wav"
},
{ // 10
	"pupils_starv1.wav", "pupils_nojob1.wav", "pupils_needjob1.wav", "pupils_nofun1.wav",
	"pupils_relig1.wav", "pupils_great1.wav", "pupils_great2.wav", "pupils_exact1.wav",
	"pupils_exact2.wav", "pupils_exact3.wav", "pupils_exact4.wav", "pupils_exact5.wav",
	"pupils_exact6.wav", "pupils_exact7.wav", "pupils_exact8.wav", "pupils_exact9.wav",
	"pupils_exact0.wav", "pupils_free1.wav", "pupils_free2.wav", "pupils_free3.wav"
},
{ // 11
	"bather_starv1.wav", "bather_nojob1.wav", "bather_needjob1.wav", "bather_nofun1.wav",
	"bather_relig1.wav", "bather_great1.wav", "bather_great2.wav", "bather_exact1.wav",
	"bather_exact2.wav", "bather_exact3.wav", "bather_exact4.wav", "bather_exact5.wav",
	"bather_exact6.wav", "bather_exact7.wav", "bather_exact8.wav", "bather_exact9.wav",
	"bather_exact0.wav", "bather_free1.wav", "bather_free2.wav", "bather_free3.wav"
},
{ // 12
	"doctor_starv1.wav", "doctor_nojob1.wav", "doctor_needjob1.wav", "doctor_nofun1.wav",
	"doctor_relig1.wav", "doctor_great1.wav", "doctor_great2.wav", "doctor_exact1.wav",
	"doctor_exact2.wav", "doctor_exact3.wav", "doctor_exact4.wav", "doctor_exact5.wav",
	"doctor_exact6.wav", "doctor_exact7.wav", "doctor_exact8.wav", "doctor_exact9.wav",
	"doctor_exact0.wav", "doctor_free1.wav", "doctor_free2.wav", "doctor_free3.wav"
},
{ // 13
	"barber_starv1.wav", "barber_nojob1.wav", "barber_needjob1.wav", "barber_nofun1.wav",
	"barber_relig1.wav", "barber_great1.wav", "barber_great2.wav", "barber_exact1.wav",
	"barber_exact2.wav", "barber_exact3.wav", "barber_exact4.wav", "barber_exact5.wav",
	"barber_exact6.wav", "barber_exact7.wav", "barber_exact8.wav", "barber_exact9.wav",
	"barber_exact0.wav", "barber_free1.wav", "barber_free2.wav", "barber_free3.wav"
},
{ // 14
	"actors_starv1.wav", "actors_nojob1.wav", "actors_needjob1.wav", "actors_nofun1.wav",
	"actors_relig1.wav", "actors_great1.wav", "actors_great2.wav", "actors_exact1.wav",
	"actors_exact2.wav", "actors_exact3.wav", "actors_exact4.wav", "actors_exact5.wav",
	"actors_exact6.wav", "actors_exact7.wav", "actors_exact8.wav", "actors_exact9.wav",
	"actors_exact0.wav", "actors_free1.wav", "actors_free2.wav", "actors_free3.wav"
},
{ // 15
	"gladtr_starv1.wav", "gladtr_nojob1.wav", "gladtr_needjob1.wav", "gladtr_nofun1.wav",
	"gladtr_relig1.wav", "gladtr_great1.wav", "gladtr_great2.wav", "gladtr_exact1.wav",
	"gladtr_exact2.wav", "gladtr_exact3.wav", "gladtr_exact4.wav", "gladtr_exact5.wav",
	"gladtr_exact6.wav", "gladtr_exact7.wav", "gladtr_exact8.wav", "gladtr_exact9.wav",
	"gladtr_exact0.wav", "gladtr_free1.wav", "gladtr_free2.wav", "gladtr_free3.wav"
},
{ // 16
	"liontr_starv1.wav", "liontr_nojob1.wav", "liontr_needjob1.wav", "liontr_nofun1.wav",
	"liontr_relig1.wav", "liontr_great1.wav", "liontr_great2.wav", "liontr_exact1.wav",
	"liontr_exact2.wav", "liontr_exact3.wav", "liontr_exact4.wav", "liontr_exact5.wav",
	"liontr_exact6.wav", "liontr_exact7.wav", "liontr_exact8.wav", "liontr_exact9.wav",
	"liontr_exact0.wav", "liontr_free1.wav", "liontr_free2.wav", "liontr_free3.wav"
},
{ // 17
	"charot_starv1.wav", "charot_nojob1.wav", "charot_needjob1.wav", "charot_nofun1.wav",
	"charot_relig1.wav", "charot_great1.wav", "charot_great2.wav", "charot_exact1.wav",
	"charot_exact2.wav", "charot_exact3.wav", "charot_exact4.wav", "charot_exact5.wav",
	"charot_exact6.wav", "charot_exact7.wav", "charot_exact8.wav", "charot_exact9.wav",
	"charot_exact0.wav", "charot_free1.wav", "charot_free2.wav", "charot_free3.wav"
},
{ // 18
	"patric_starv1.wav", "patric_nojob1.wav", "patric_needjob1.wav", "patric_nofun1.wav",
	"patric_relig1.wav", "patric_great1.wav", "patric_great2.wav", "patric_exact1.wav",
	"patric_exact2.wav", "patric_exact3.wav", "patric_exact4.wav", "patric_exact5.wav",
	"patric_exact6.wav", "patric_exact7.wav", "patric_exact8.wav", "patric_exact9.wav",
	"patric_exact0.wav", "patric_free1.wav", "patric_free2.wav", "patric_free3.wav"
},
{ // 19
	"pleb_starv1.wav", "pleb_nojob1.wav", "pleb_needjob1.wav", "pleb_nofun1.wav",
	"pleb_relig1.wav", "pleb_great1.wav", "pleb_great2.wav", "pleb_exact1.wav",
	"pleb_exact2.wav", "pleb_exact3.wav", "pleb_exact4.wav", "pleb_exact5.wav",
	"pleb_exact6.wav", "pleb_exact7.wav", "pleb_exact8.wav", "pleb_exact9.wav",
	"pleb_exact0.wav", "pleb_free1.wav", "pleb_free2.wav", "pleb_free3.wav"
},
{ // 20
	"rioter_starv1.wav", "rioter_nojob1.wav", "rioter_needjob1.wav", "rioter_nofun1.wav",
	"rioter_relig1.wav", "rioter_great1.wav", "rioter_great2.wav", "rioter_exact1.wav",
	"rioter_exact2.wav", "rioter_exact3.wav", "rioter_exact4.wav", "rioter_exact5.wav",
	"rioter_exact6.wav", "rioter_exact7.wav", "rioter_exact8.wav", "rioter_exact9.wav",
	"rioter_exact0.wav", "rioter_free1.wav", "rioter_free2.wav", "rioter_free3.wav"
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
	"emigrate_starv1.wav", "emigrate_nojob1.wav", "emigrate_needjob1.wav", "emigrate_nofun1.wav",
	"emigrate_relig1.wav", "emigrate_great1.wav", "emigrate_great2.wav", "emigrate_exact1.wav",
	"emigrate_exact2.wav", "emigrate_exact3.wav", "emigrate_exact4.wav", "emigrate_exact5.wav",
	"emigrate_exact6.wav", "emigrate_exact7.wav", "emigrate_exact8.wav", "emigrate_exact9.wav",
	"emigrate_exact0.wav", "emigrate_free1.wav", "emigrate_free2.wav", "emigrate_free3.wav"
},
{ // 24
	"immigrant_starv1.wav", "immigrant_nojob1.wav", "immigrant_needjob1.wav", "immigrant_nofun1.wav",
	"immigrant_relig1.wav", "immigrant_great1.wav", "immigrant_great2.wav", "immigrant_exact1.wav",
	"immigrant_exact2.wav", "immigrant_exact3.wav", "immigrant_exact4.wav", "immigrant_exact5.wav",
	"immigrant_exact6.wav", "immigrant_exact7.wav", "immigrant_exact8.wav", "immigrant_exact9.wav",
	"immigrant_exact0.wav", "immigrant_free1.wav", "immigrant_free2.wav", "immigrant_free3.wav"
},
{ // 25
	"enemy_starv1.wav", "enemy_nojob1.wav", "enemy_needjob1.wav", "enemy_nofun1.wav",
	"enemy_relig1.wav", "enemy_great1.wav", "enemy_great2.wav", "enemy_exact1.wav",
	"enemy_exact2.wav", "enemy_exact3.wav", "enemy_exact4.wav", "enemy_exact5.wav",
	"enemy_exact6.wav", "enemy_exact7.wav", "enemy_exact8.wav", "enemy_exact9.wav",
	"enemy_exact0.wav", "enemy_free1.wav", "enemy_free2.wav", "enemy_free3.wav"
},
{ // 26
	"local_starv1.wav", "local_nojob1.wav", "local_needjob1.wav", "local_nofun1.wav",
	"local_relig1.wav", "local_great1.wav", "local_great2.wav", "local_exact1.wav",
	"local_exact2.wav", "local_exact3.wav", "local_exact4.wav", "local_exact5.wav",
	"local_exact6.wav", "local_exact7.wav", "local_exact8.wav", "local_exact9.wav",
	"local_exact0.wav", "local_free1.wav", "local_free2.wav", "local_free3.wav"
},
{ // 27
	"libary_starv1.wav", "libary_nojob1.wav", "libary_needjob1.wav", "libary_nofun1.wav",
	"libary_relig1.wav", "libary_great1.wav", "libary_great2.wav", "libary_exact1.wav",
	"libary_exact2.wav", "libary_exact3.wav", "libary_exact4.wav", "libary_exact5.wav",
	"libary_exact6.wav", "libary_exact7.wav", "libary_exact8.wav", "libary_exact9.wav",
	"libary_exact0.wav", "libary_free1.wav", "libary_free2.wav", "libary_free3.wav"
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
			if (w->minMaxSeen >= HouseLevel_LargeCasa) {
				phraseId = 7;
			} else if (w->minMaxSeen >= HouseLevel_SmallHovel) {
				phraseId = 8;
			} else if (w->minMaxSeen >= HouseLevel_LargeTent) {
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
			if (w->actionState == WalkerActionState_145_MarketBuyerGoingToStorage) {
				phraseId = 7;
			} else if (w->actionState == WalkerActionState_146_MarketBuyerReturning) {
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
			if (w->actionState == WalkerActionState_135_DockerImportGoingToWarehouse ||
				w->actionState == WalkerActionState_136_DockerExportGoingToWarehouse) {
				if (Calc_distanceMaximum(
					w->destinationX, w->destinationY, w->sourceX, w->sourceY) >= 25) {
					phraseId = 9; // too far
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
				int state = WalkerAction_TradeShip_isBuyingOrSelling(walkerId);
				if (state == 1) {
					phraseId = 8; // buying goods
				} else if (state == 2) {
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
		case Walker_Enemy48_Chariot:
		case Walker_Enemy52_MountedArcher:
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
		case Walker_Enemy43_Spear:
		case Walker_Enemy44_Sword:
		case Walker_Enemy45_Sword:
		case Walker_Enemy49_FastSword:
		case Walker_Enemy50_Sword:
		case Walker_Enemy51_Spear:
		case Walker_Enemy53_Axe:
		case Walker_Enemy54_Gladiator:
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
			Sound_Speech_playFile("wavs/army_war_cry.wav");
		}
	} else if (WalkerIsLegion(walkerType)) {
		if (Data_CityInfo.numSoldiersInCity == 1) {
			Sound_Speech_playFile("wavs/barbarian_war_cry.wav");
		}
	}
}

#define PLAY_HIT_SOUND(t, ch) \
	Data_CityInfo.t--;\
	if (Data_CityInfo.t <= 0) {\
		Data_CityInfo.t = 8;\
		Sound_Effects_playChannel(ch);\
	}

void Walker_playHitSound(int walkerType)
{
	switch (walkerType) {
		case Walker_FortLegionary:
		case Walker_EnemyCaesarLegionary:
			PLAY_HIT_SOUND(soundHitSoldier, SoundChannel_Sword);
			break;
		case Walker_FortMounted:
		case Walker_Enemy45_Sword:
		case Walker_Enemy48_Chariot:
		case Walker_Enemy50_Sword:
		case Walker_Enemy52_MountedArcher:
		case Walker_Enemy54_Gladiator:
			PLAY_HIT_SOUND(soundHitSoldier, SoundChannel_SwordSwing);
			break;
		case Walker_FortJavelin:
			PLAY_HIT_SOUND(soundHitSoldier, SoundChannel_SwordLight);
			break;
		case Walker_Enemy43_Spear:
		case Walker_Enemy51_Spear:
			PLAY_HIT_SOUND(soundHitSpear, SoundChannel_Spear);
			break;
		case Walker_Enemy44_Sword:
		case Walker_Enemy49_FastSword:
			PLAY_HIT_SOUND(soundHitClub, SoundChannel_Club);
			break;
		case Walker_Enemy53_Axe:
			PLAY_HIT_SOUND(soundHitAxe, SoundChannel_Axe);
			break;
		case Walker_Enemy46_Camel:
			Sound_Effects_playChannel(SoundChannel_Camel);
			break;
		case Walker_Enemy47_Elephant:
			if (Data_CityInfo.soundHitElephant == 1) {
				Sound_Effects_playChannel(SoundChannel_ElephantHit);
				Data_CityInfo.soundHitElephant = 0;
			} else {
				Sound_Effects_playChannel(SoundChannel_Elephant);
				Data_CityInfo.soundHitElephant = 1;
			}
			break;
		case Walker_LionTamer:
			Sound_Effects_playChannel(SoundChannel_LionAttack);
			break;
		case Walker_Wolf:
			Data_CityInfo.soundHitWolf--;
			if (Data_CityInfo.soundHitWolf <= 0) {
				Data_CityInfo.soundHitWolf = 4;
				Sound_Effects_playChannel(SoundChannel_WolfAttack);
			}
			break;
	}
}
