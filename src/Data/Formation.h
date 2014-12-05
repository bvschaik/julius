#ifndef DATA_FORMATION_H
#define DATA_FORMATION_H

#define MAX_FORMATIONS 50

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
	FormationLayout_10 = 10,
	FormationLayout_Enemy12 = 12,
};

struct Data_Formation {
	unsigned char inUse;
	unsigned char ciid;
	unsigned char legionId;
	unsigned char isAtFort;
	short walkerType;
	short buildingId;
	short walkerIds[16];
	unsigned char numWalkers;
	unsigned char maxWalkers;
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
	short standardWalkerId;
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
	int formationId[25];
	int homeX[25];
	int homeY[25];
	int layout[25];
	int destinationX[25];
	int destinationY[25];
	int destinationBuildingId[25];
	int numLegions[25];
	int ignoreRomanSoldiers[25];
} Data_Formation_Invasion;

#endif
