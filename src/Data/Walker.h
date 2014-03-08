#ifndef DATA_WALKER_H
#define DATA_WALKER_H

#define MAX_WALKERS 1000

enum {
	Walker_Immigrant = 1,
	Walker_Emigrant = 2,
	Walker_Homeless = 3,
	Walker_CartPusher = 4,
	Walker_LaborSeeker = 5,
	Walker_Explosion = 6,
	Walker_TaxCollector = 7,
	Walker_Engineer = 8,
	Walker_Warehouseman = 9,
	Walker_Prefect = 10,
	Walker_FortJavelin = 11,
	Walker_FortMounted = 12,
	Walker_FortLegionary = 13,
	Walker_FortStandard = 14,
	Walker_Actor = 15,
	Walker_Gladiator = 16,
	Walker_LionTamer = 17,
	Walker_Charioteer = 18,
	Walker_TradeCaravan = 19,
	Walker_TradeShip = 20,
	Walker_TradeCaravanDonkey = 21,
	Walker_Protestor = 22,
	Walker_Criminal = 23,
	Walker_Rioter = 24,
	Walker_FishingBoat = 25,
	Walker_MarketTrader = 26,
	Walker_Priest = 27,
	Walker_SchoolChild = 28,
	Walker_Teacher = 29,
	Walker_Librarian = 30,
	Walker_Barber = 31,
	Walker_BathhouseWorker = 32,
	Walker_Doctor = 33,
	Walker_Surgeon = 34,
	Walker_35 = 35,
	Walker_MapFlag = 36,
	Walker_Flotsam = 37,
	Walker_Dockman = 38,
	Walker_MarketBuyer = 39,
	Walker_Patrician = 40,
	Walker_IndigenousNative = 41,
	Walker_TowerSentry = 42,
	Walker_Enemy43 = 43,
	Walker_Enemy44 = 44,
	Walker_Enemy45 = 45,
	Walker_Enemy46_Camel = 46,
	Walker_Enemy47_Elephant = 47,
	Walker_Enemy48_Horse = 48,
	Walker_Enemy49 = 49,
	Walker_Enemy50 = 50,
	Walker_Enemy51 = 51,
	Walker_Enemy52_Horse = 52,
	Walker_Enemy53 = 53,
	Walker_Enemy54 = 54,
	Walker_Enemy55_Javelin = 55,
	Walker_Enemy56_Mounted = 56,
	Walker_Enemy57_Legionary = 57,
	Walker_NativeTrader = 58,
	Walker_Arrow = 59,
	Walker_Javelin = 60,
	Walker_Bolt = 61,
	Walker_Ballista = 62,
	Walker_Creature = 63,
	Walker_Missionary = 64,
	Walker_FishGulls = 65,
	Walker_DeliveryBoy = 66,
	Walker_Shipwreck = 67,
	Walker_Sheep = 68,
	Walker_Wolf = 69,
	Walker_Zebra = 70,
	Walker_Spear = 71,
	Walker_HippodromeMiniHorses = 72
};

extern struct Data_Walker_NameSequence {
	int citizenMale;
	int patrician;
	int citizenFemale;
	int taxCollector;
	int engineer;
	int prefect;
	int javelinThrower;
	int cavalry;
	int legionary;
	int actor;
	int gladiator;
	int lionTamer;
	int charioteer;
	int barbarian;
	int enemyGreek;
	int enemyEgyptian;
	int enemyArabian;
	int trader;
	int tradeShip;
	int warShip;
	int enemyShip;
} Data_Walker_NameSequence;

extern struct Data_Walker {
	char type;
	char x;
	char y;
	int minMaxSeen;// TODO char
	short buildingId;
	char name;
	char formationId;
} Data_Walkers[MAX_WALKERS];

#endif
