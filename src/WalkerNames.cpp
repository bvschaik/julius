//TODO check whether shipwreck has a name!

#include "Random.h"
#include "Data/Walker.h"
#include "Data/Formation.h"
#include "Data/Random.h"

void WalkerName_init()
{
	Random_generateNext();
	Data_Walker_NameSequence.citizenMale = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.patrician = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.citizenFemale = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.taxCollector = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.engineer = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.prefect = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.javelinThrower = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.cavalry = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.legionary = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.actor = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.gladiator = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.lionTamer = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.charioteer = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.barbarian = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.enemyGreek = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.enemyEgyptian = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.enemyArabian = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.trader = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.tradeShip = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.warShip = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Walker_NameSequence.enemyShip = Data_Random.random1_7bit & 0xf;
}

#define SET(var, off, max) \
	Data_Walkers[walkerId].name = Data_Walker_NameSequence.var; \
	++Data_Walker_NameSequence.var; \
	if (Data_Walker_NameSequence.var >= max) Data_Walker_NameSequence.var = 0;

void WalkerName_set(int walkerId)
{
	int type = Data_Walkers[walkerId].type;
	if (type == Walker_Explosion || type == Walker_FortStandard || type == Walker_FishGulls ||
		type == Walker_Creature || type == Walker_HippodromeMiniHorses) {
		return;
	}

	switch (type) {
		case Walker_TaxCollector:
			SET(taxCollector, 132, 32);
			break;
		case Walker_Engineer:
			SET(engineer, 165, 32);
			break;
		case Walker_Prefect:
		case Walker_TowerSentry:
			SET(prefect, 198, 32);
			break;
		case Walker_Actor:
			SET(actor, 330, 32);
			break;
		case Walker_Gladiator:
			SET(gladiator, 363, 32);
			break;
		case Walker_LionTamer:
			SET(lionTamer, 396, 16);
			break;
		case Walker_Charioteer:
			SET(charioteer, 413, 16);
			break;
		case Walker_TradeCaravan:
		case Walker_TradeCaravanDonkey:
			SET(trader, 562, 16);
			break;
		case Walker_TradeShip:
		case Walker_FishingBoat:
			SET(tradeShip, 579, 16);
			break;
		case Walker_MarketTrader:
		case Walker_MarketBuyer:
		case Walker_BathhouseWorker:
			SET(citizenFemale, 99, 32);
			break;
		case Walker_SchoolChild:
		case Walker_DeliveryBoy:
		case Walker_Barber:
		case Walker_35:
		default:
			SET(citizenMale, 1, 64);
			break;
		case Walker_Priest:
		case Walker_Teacher:
		case Walker_Missionary:
		case Walker_Librarian:
		case Walker_Doctor:
		case Walker_Surgeon:
		case Walker_Patrician:
			SET(patrician, 66, 32);
			break;
		case Walker_FortJavelin:
		case Walker_Enemy55_Javelin:
			SET(javelinThrower, 231, 32);
			break;
		case Walker_FortMounted:
		case Walker_Enemy56_Mounted:
			SET(cavalry, 264, 32);
			break;
		case Walker_FortLegionary:
		case Walker_EnemyCaesarLegionary:
			SET(legionary, 297, 32);
			break;
		case Walker_IndigenousNative:
		case Walker_NativeTrader:
			SET(barbarian, 430, 32);
			break;
		case Walker_Enemy43:
		case Walker_Enemy44:
		case Walker_Enemy45:
		case Walker_Enemy46_Camel:
		case Walker_Enemy47_Elephant:
		case Walker_Enemy48_Horse:
		case Walker_Enemy49:
		case Walker_Enemy50:
		case Walker_Enemy51:
		case Walker_Enemy52_Horse:
		case Walker_Enemy53:
			switch (Data_Formations[(int)Data_Walkers[walkerId].formationId].enemyType) {
				case 8:
					SET(enemyGreek, 463, 32);
					break;
				case 9: // 9
					SET(enemyEgyptian, 496, 32);
					break;
				case 1:
				case 5:
				case 10:
					SET(enemyArabian, 529, 32);
					break;
				case 7:
					SET(prefect, 198, 32);
					break;
				default:
					SET(barbarian, 430, 32);
					break;
			}
			break;
	}
}
