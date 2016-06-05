#include "Walker.h"

#include "Random.h"
#include "Data/Constants.h"
#include "Data/Formation.h"
#include "Data/Random.h"
#include "Data/Figure.h"

void FigureName_init()
{
	Random_generateNext();
	Data_Figure_NameSequence.citizenMale = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.patrician = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.citizenFemale = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.taxCollector = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.engineer = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.prefect = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.javelinThrower = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.cavalry = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.legionary = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.actor = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.gladiator = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.lionTamer = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.charioteer = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.barbarian = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.enemyGreek = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.enemyEgyptian = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.enemyArabian = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.trader = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.tradeShip = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.warShip = Data_Random.random1_7bit & 0xf;
	Random_generateNext();
	Data_Figure_NameSequence.enemyShip = Data_Random.random1_7bit & 0xf;
}

#define SET(var, off, max) \
	Data_Walkers[walkerId].name = off + Data_Figure_NameSequence.var; \
	++Data_Figure_NameSequence.var; \
	if (Data_Figure_NameSequence.var >= max) Data_Figure_NameSequence.var = 0;

void FigureName_set(int walkerId)
{
	int type = Data_Walkers[walkerId].type;
	if (type == Figure_Explosion || type == Figure_FortStandard || type == Figure_FishGulls ||
		type == Figure_Creature || type == Figure_HippodromeMiniHorses) {
		return;
	}

	switch (type) {
		case Figure_TaxCollector:
			SET(taxCollector, 132, 32);
			break;
		case Figure_Engineer:
			SET(engineer, 165, 32);
			break;
		case Figure_Prefect:
		case Figure_TowerSentry:
			SET(prefect, 198, 32);
			break;
		case Figure_Actor:
			SET(actor, 330, 32);
			break;
		case Figure_Gladiator:
			SET(gladiator, 363, 32);
			break;
		case Figure_LionTamer:
			SET(lionTamer, 396, 16);
			break;
		case Figure_Charioteer:
			SET(charioteer, 413, 16);
			break;
		case Figure_TradeCaravan:
		case Figure_TradeCaravanDonkey:
			SET(trader, 562, 16);
			break;
		case Figure_TradeShip:
		case Figure_FishingBoat:
			SET(tradeShip, 579, 16);
			break;
		case Figure_MarketTrader:
		case Figure_MarketBuyer:
		case Figure_BathhouseWorker:
			SET(citizenFemale, 99, 32);
			break;
		case Figure_SchoolChild:
		case Figure_DeliveryBoy:
		case Figure_Barber:
		case Figure_35:
		default:
			SET(citizenMale, 1, 64);
			break;
		case Figure_Priest:
		case Figure_Teacher:
		case Figure_Missionary:
		case Figure_Librarian:
		case Figure_Doctor:
		case Figure_Surgeon:
		case Figure_Patrician:
			SET(patrician, 66, 32);
			break;
		case Figure_FortJavelin:
		case Figure_Enemy55_Javelin:
			SET(javelinThrower, 231, 32);
			break;
		case Figure_FortMounted:
		case Figure_Enemy56_Mounted:
			SET(cavalry, 264, 32);
			break;
		case Figure_FortLegionary:
		case Figure_EnemyCaesarLegionary:
			SET(legionary, 297, 32);
			break;
		case Figure_IndigenousNative:
		case Figure_NativeTrader:
			SET(barbarian, 430, 32);
			break;
		case Figure_Enemy43_Spear:
		case Figure_Enemy44_Sword:
		case Figure_Enemy45_Sword:
		case Figure_Enemy46_Camel:
		case Figure_Enemy47_Elephant:
		case Figure_Enemy48_Chariot:
		case Figure_Enemy49_FastSword:
		case Figure_Enemy50_Sword:
		case Figure_Enemy51_Spear:
		case Figure_Enemy52_MountedArcher:
		case Figure_Enemy53_Axe:
			switch (Data_Formations[(int)Data_Walkers[walkerId].formationId].enemyType) {
				case EnemyType_8_Greek:
					SET(enemyGreek, 463, 32);
					break;
				case EnemyType_9_Egyptian:
					SET(enemyEgyptian, 496, 32);
					break;
				case EnemyType_1_Numidian:
				case EnemyType_5_Pergamum:
				case EnemyType_10_Carthaginian:
					SET(enemyArabian, 529, 32);
					break;
				case EnemyType_7_Etruscan:
					SET(prefect, 198, 32);
					break;
				default:
					SET(barbarian, 430, 32);
					break;
			}
			break;
	}
}
