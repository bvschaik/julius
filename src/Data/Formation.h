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
	FormationLayout_Enemy12 = 12,
};

struct Data_Formation {
	char inUse;
	char ciid;
	char legionId;
	char isAtFort;
	short walkerType;
	short buildingId;
	short walkerIds[16];
	char numWalkers;
	char maxWalkers;
	short layout;
	short morale;
	char xHome; //2e
	char yHome;
	unsigned char xStandard; //30
	unsigned char yStandard;
	unsigned char x;
	unsigned char y;
	unsigned char destinationX; // 34
	unsigned char destinationY;
	short destinationBuildingId;
	short standardWalkerId;
	char isLegion;
	char __padding;
	short attackType;
	short legionRecruitType;
	short hasMilitaryTraining; //40
	short totalDamage; //42
	short maxTotalDamage; //44
	short waitTicks; // 46
	short recentFight; //48
	short __unknown4a;
	short __unknown4c;
	short __unknown4e;
	short __unknown50;
	short isHalted;
	short missileFired; //54
	short missileAttackTimeout; //56
	short missileAttackFormationId; //58
	short layoutBeforeMopUp;
	short cursedByMars;
	char monthsLowMorale; //5e
	char empireService;
	char inDistantBattle; //60
	char isHerd;
	char enemyType;
	char direction; // 63
	unsigned char prevXHome;
	unsigned char prevYHome;
	char __unknown66;
	char orientation;
	char monthsFromHome; //68
	char monthsVeryLowMorale; //69
	char invasionId;
	char herdWolfSpawnDelay;
	char herdDirection;
	char __padding6d[17];
	short invasionSeq;
};

extern struct Data_Formation Data_Formations[MAX_FORMATIONS];

extern struct Data_Formation_Extra {
	int idLastInUse;
	int idLastLegion;
	int numLegions;
	int numEnemyFormations;
	int numEnemySoldierStrength;
	int numLegionFormations;
	int numLegionSoldierStrength;

	int daysSinceRomanSoldierConcentration;
} Data_Formation_Extra;

#endif
