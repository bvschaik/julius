#ifndef DATA_WALKER_H
#define DATA_WALKER_H

#define MAX_WALKERS 1000
#define MAX_TRADERS 100

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
	Walker_Protester = 22,
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

#define WalkerIsEnemyOrNative(t) ((t) >= Walker_IndigenousNative && (t) <= Walker_NativeTrader)

enum {
	WalkerActionState_20_CartPusher = 20,
	WalkerActionState_21_CartPusher = 21,
	WalkerActionState_22_CartPusher = 22,
	WalkerActionState_23_CartPusher = 23,
	WalkerActionState_51_Warehouseman = 51,
	WalkerActionState_74_Prefect = 74,
	WalkerActionState_75_Prefect = 75,
	WalkerActionState_102_TraderAtWarehouse = 102,
	WalkerActionState_103_TraderLeaving = 103,
	WalkerActionState_112_TradeShipMoored = 112,
	WalkerActionState_115_TradeShipLeaving = 115,
	WalkerActionState_126_MarketTrader = 126,
	WalkerActionState_133_Dockman = 133,
	WalkerActionState_134_Dockman = 134,
	WalkerActionState_135_Dockman = 135,
	WalkerActionState_136_Dockman = 136,
	WalkerActionState_137_Dockman = 137,
	WalkerActionState_138_Dockman = 138,
	WalkerActionState_139_Dockman = 139,
	WalkerActionState_140_Dockman = 140,
	WalkerActionState_145_MarketBuyer = 145,
	WalkerActionState_146_MarketBuyer = 146,
	WalkerActionState_150 = 150,
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
	char inUse;
	char __unknown_01;
	char __unknown_02;
	char __unknown_03;
	char __unknown_04;
	char __unknown_05;
	short resourceGraphicId;
	char __unknown_08;
	char __unknown_09;
	unsigned char type;
	char resourceId;
	char __unknown_0c;
	char __unknown_0d;
	char state;
	char __unknown_0f;
	char __unknown_10;
	char __unknown_11;
	char __unknown_12;
	char __unknown_13;
	char x;
	char y;
	char __unknown_16;
	char __unknown_17;
	char __unknown_18;
	char __unknown_19;
	char __unknown_1a;
	char __unknown_1b;
	char destinationX;
	char destinationY;
	char __unknown_1e;
	char __unknown_1f;
	char sourceX;
	char sourceY;
	char __unknown_22;
	char __unknown_23;
	char __unknown_24;
	char __unknown_25;
	char __unknown_26;
	char __unknown_27;
	char actionState;
	char __unknown_29;
	char __unknown_2a;
	char __unknown_2b;
	char __unknown_2c;
	char __unknown_2d;
	char __unknown_2e;
	char __unknown_2f;
	char __unknown_30;
	char __unknown_31;
	char __unknown_32;
	char __unknown_33;
	char __unknown_34;
	char __unknown_35;
	char __unknown_36;
	char __unknown_37;
	char __unknown_38;
	char __unknown_39;
	char __unknown_3a;
	char __unknown_3b;
	char __unknown_3c;
	char __unknown_3d;
	char __unknown_3e;
	char __unknown_3f;
	char __unknown_40;
	char __unknown_41;
	char __unknown_42;
	char __unknown_43;
	char __unknown_44;
	char __unknown_45;
	char __unknown_46;
	char __unknown_47;
	char __unknown_48;
	char __unknown_49;
	short buildingId;
	char __unknown_4c;
	char __unknown_4d;
	short destinationBuildingId;
	short formationId;
	char __unknown_52;
	char __unknown_53;
	char __unknown_54;
	char __unknown_55;
	char minMaxSeen;
	char __unknown_57;
	char __unknown_58;
	char __unknown_59;
	char __unknown_5a;
	char __unknown_5b;
	char __unknown_5c;
	char __unknown_5d;
	char empireCityId;
	char __unknown_5f;
	short name; // 60
	char __unknown_62;
	char __unknown_63;
	char __unknown_64;
	char __unknown_65;
	char __unknown_66;
	char __unknown_67;
	char __unknown_68;
	char __unknown_69;
	unsigned char phraseSequenceExact;
	unsigned char phraseId;
	unsigned char phraseSequenceCity;
	unsigned char traderId;
	char __unknown_6e;
	char __unused_6f;
	char __unknown_70;
	char __unknown_71;
	char __unknown_72;
	char __unknown_73;
	char __unknown_74;
	char __unknown_75;
	char __unknown_76;
	char __unknown_77;
	char __unknown_78;
	char __unknown_79;
	char __unknown_7a;
	char __unknown_7b;
	char __unknown_7c;
	char __unknown_7d;
	char __unknown_7e;
	char __unknown_7f;
} Data_Walkers[MAX_WALKERS];

extern struct Data_Walker_Trader {
	int totalBought;
	int totalSold;
	char boughtResources[16];
	char soldResources[16];
	int moneyBoughtGoods;
	int moneySoldGoods;
} Data_Walker_Traders[MAX_TRADERS];

#endif
