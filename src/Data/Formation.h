#ifndef DATA_FORMATION_H
#define DATA_FORMATION_H

#define MAX_FORMATIONS 50
#define MAX_FORMATION_FIGURES 16
#define MAX_INVASION_FORMATIONS 25

enum {
	FormationLayout_Tortoise = 0,
	FormationLayout_DoubleLine1 = 1,
	FormationLayout_DoubleLine2 = 2,
	FormationLayout_SingleLine1 = 3,
	FormationLayout_SingleLine2 = 4,
	FormationLayout_Column = 5,
	FormationLayout_MopUp = 6,
	FormationLayout_AtRest = 7,
	FormationLayout_Enemy8 = 8,
	FormationLayout_Herd = 9,
	FormationLayout_Enemy10 = 10,
	FormationLayout_Enemy12 = 12,
};

enum {
	FormationAttackType_FoodChain = 0,
	FormationAttackType_GoldStores = 1,
	FormationAttackType_BestBuildings = 2,
	FormationAttackType_Troops = 3,
	FormationAttackType_Random = 4
};

struct Data_Formation {
	unsigned char inUse;
	unsigned char ciid;
	unsigned char legionId;
	unsigned char isAtFort;
	short figureType;
	short buildingId;
	short figureIds[MAX_FORMATION_FIGURES];
	unsigned char numFigures;
	unsigned char maxFigures;
	short layout;
	short morale;
	unsigned char xHome; //2e
	unsigned char yHome;
	unsigned char xStandard; //30
	unsigned char yStandard;
	unsigned char x;
	unsigned char y;
	unsigned char destinationX; // 34
	unsigned char destinationY;
	short destinationBuildingId;
	short standardFigureId;
	unsigned char isLegion;
	unsigned char __padding;
	short attackType;
	short legionRecruitType;
	short hasMilitaryTraining; //40
	short totalDamage; //42
	short maxTotalDamage; //44
	short waitTicks; // 46
	short recentFight; //48
	short durationAdvance;
	short durationRegroup;
	short durationHalt;
	short enemyLegionIndex;
	short isHalted;
	short missileFired; //54
	short missileAttackTimeout; //56
	short missileAttackFormationId; //58
	short layoutBeforeMopUp;
	short cursedByMars;
	unsigned char monthsLowMorale; //5e
	unsigned char empireService;
	unsigned char inDistantBattle; //60
	unsigned char isHerd;
	unsigned char enemyType;
	unsigned char direction; // 63
	unsigned char prevXHome;
	unsigned char prevYHome;
	unsigned char __unknown66;
	unsigned char orientation;
	unsigned char monthsFromHome; //68
	unsigned char monthsVeryLowMorale; //69
	unsigned char invasionId;
	unsigned char herdWolfSpawnDelay;
	unsigned char herdDirection;
	unsigned char __padding6d[17];
	short invasionSeq;
};

extern struct Data_Formation Data_Formations[MAX_FORMATIONS];

extern struct _Data_Formation_Extra {
	int idLastInUse;
	int idLastLegion;
	int numForts;
	int numEnemyFormations;
	int numEnemySoldierStrength;
	int numLegionFormations;
	int numLegionSoldierStrength;

	int daysSinceRomanSoldierConcentration;
} Data_Formation_Extra;

extern struct _Data_Formation_Invasion {
	int formationId[MAX_INVASION_FORMATIONS];
	int homeX[MAX_INVASION_FORMATIONS];
	int homeY[MAX_INVASION_FORMATIONS];
	int layout[MAX_INVASION_FORMATIONS];
	int destinationX[MAX_INVASION_FORMATIONS];
	int destinationY[MAX_INVASION_FORMATIONS];
	int destinationBuildingId[MAX_INVASION_FORMATIONS];
	int numLegions[MAX_INVASION_FORMATIONS];
	int ignoreRomanSoldiers[MAX_INVASION_FORMATIONS];
} Data_Formation_Invasion;

#endif
