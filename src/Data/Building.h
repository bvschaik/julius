#ifndef DATA_BUILDING_H
#define DATA_BUILDING_H

enum {
	Building_Menu_Farms = 2,
	Building_Menu_RawMaterials = 3,
	Building_Menu_Workshops = 4,
	Building_Road = 5,
	Building_Wall = 6,
	Building_ReservoirSimon = 7,
	Building_Aqueduct = 8,
	Building_ClearLand = 9,
	Building_HouseVacantLot = 10,
	Building_HouseLargeTent = 11,
	Building_HouseSmallShack = 12,
	Building_HouseLargeShack = 13,
	Building_HouseSmallHovel = 14,
	Building_HouseLargeHovel = 15,
	Building_HouseSmallCasa = 16,
	Building_HouseLargeCasa = 17,
	Building_HouseSmallInsula = 18,
	Building_HouseMediumInsula = 19,
	Building_HouseLargeInsula = 20,
	Building_HouseGrandInsula = 21,
	Building_HouseSmallVilla = 22,
	Building_HouseMediumVilla = 23,
	Building_HouseLargeVilla = 24,
	Building_HouseGrandVilla = 25,
	Building_HouseSmallPalace = 26,
	Building_HouseMediumPalace = 27,
	Building_HouseLargePalace = 28,
	Building_HouseLuxuryPalace = 29,
	Building_Amphitheater = 30,
	Building_Theater = 31,
	Building_Hippodrome = 32,
	Building_Colosseum = 33,
	Building_GladiatorSchool = 34,
	Building_LionHouse = 35,
	Building_ActorColony = 36,
	Building_ChariotMaker = 37,
	Building_Plaza = 38,
	Building_Gardens = 39,
	Building_FortLegionaries = 40,
	Building_SmallStatue = 41,
	Building_MediumStatue = 42,
	Building_LargeStatue = 43,
	Building_FortJavelin = 44,
	Building_FortMounted = 45,
	Building_Doctor = 46,
	Building_Hospital = 47,
	Building_Bathhouse = 48,
	Building_Barber = 49,
	Building_DistributionCenter_Unused = 50,
	Building_School = 51,
	Building_Academy = 52,
	Building_Library = 53,
	Building_FortGround = 54,
	Building_Prefecture = 55,
	Building_TriumphalArch = 56,
	Building_FortGround__ = 57,
	Building_Gatehouse = 58,
	Building_Tower = 59,
	Building_SmallTempleCeres = 60,
	Building_SmallTempleNeptune = 61,
	Building_SmallTempleMercury = 62,
	Building_SmallTempleMars = 63,
	Building_SmallTempleVenus = 64,
	Building_LargeTempleCeres = 65,
	Building_LargeTempleNeptune = 66,
	Building_LargeTempleMercury = 67,
	Building_LargeTempleMars = 68,
	Building_LargeTempleVenus = 69,
	Building_Market = 70,
	Building_Granary = 71,
	Building_Warehouse = 72,
	Building_WarehouseSpace = 73,
	Building_Shipyard = 74,
	Building_Dock = 75,
	Building_Wharf = 76,
	Building_GovernorsHouse = 77,
	Building_GovernorsVilla = 78,
	Building_GovernorsPalace = 79,
	Building_MissionPost = 80,
	Building_EngineersPost = 81,
	Building_LowBridge = 82,
	Building_ShipBridge = 83,
	Building_Senate = 84,
	Building_SenateUpgraded = 85,
	Building_Forum = 86,
	Building_ForumUpgraded = 87,
	Building_NativeHut = 88,
	Building_NativeMeeting = 89,
	Building_Reservoir = 90,
	Building_Fountain = 91,
	Building_Well = 92,
	Building_NativeCrops = 93,
	Building_MilitaryAcademy = 94,
	Building_Barracks = 95,
	Building_Menu_SmallTemples = 96,
	Building_Menu_LargeTemples = 97,
	Building_Oracle = 98,
	Building_BurningRuin = 99,
	Building_WheatFarm = 100,
	Building_VegetableFarm = 101,
	Building_FruitFarm = 102,
	Building_OliveFarm = 103,
	Building_VinesFarm = 104,
	Building_PigFarm = 105,
	Building_MarbleQuarry = 106,
	Building_IronMine = 107,
	Building_TimberYard = 108,
	Building_ClayPit = 109,
	Building_WineWorkshop = 110,
	Building_OilWorkshop = 111,
	Building_WeaponsWorkshop = 112,
	Building_FurnitureWorkshop = 113,
	Building_PotteryWorkshop = 114
};

extern struct Data_Building {
	char inUse;
	char __unknown_01;
	char __unknown_02;
	char __unknown_03;
	char __unknown_04;
	char __unknown_05;
	char x;
	char y;
	short gridOffset;
	short type;
	char __unknown_0c;
	char __unknown_0d;
	char __unknown_0e;
	char __unknown_0f;
	char __unknown_10;
	char __unknown_11;
	char __unknown_12;
	char __unknown_13;
	char __unknown_14;
	char __unknown_15;
	char __unknown_16;
	char __unknown_17;
	char __unknown_18;
	char __unknown_19;
	char __unknown_1a;
	char __unknown_1b;
	char __unknown_1c;
	char __unknown_1d;
	char __unknown_1e;
	char __unknown_1f;
	char __unknown_20;
	char __unknown_21;
	char __unknown_22;
	char __unknown_23;
	char __unknown_24;
	char __unknown_25;
	char __unknown_26;
	char __unknown_27;
	char __unknown_28;
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
	char __unknown_4a;
	char __unknown_4b;
	char __unknown_4c;
	char __unknown_4d;
	char __unknown_4e;
	char __unknown_4f;
	char __unknown_50;
	char __unknown_51;
	char __unknown_52;
	char __unknown_53;
	char __unknown_54;
	char __unknown_55;
	char __unknown_56;
	char __unknown_57;
	char __unknown_58;
	char __unknown_59;
	char __unknown_5a;
	char __unknown_5b;
	char __unknown_5c;
	char __unknown_5d;
	char __unknown_5e;
	char __unknown_5f;
	char __unknown_60;
	char __unknown_61;
	char __unknown_62;
	char __unknown_63;
	char __unknown_64;
	char __unknown_65;
	char __unknown_66;
	char __unknown_67;
	char __unknown_68;
	char __unknown_69;
	char __unknown_6a;
	char __unknown_6b;
	char __unknown_6c;
	char __unknown_6d;
	char __unknown_6e;
	char __unknown_6f;
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
} Data_Buildings[2000];

#endif
