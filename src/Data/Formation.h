#ifndef DATA_FORMATION_H
#define DATA_FORMATION_H

#define MAX_FORMATIONS 50

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
	char xFort;
	char yFort;
	char xStandard; //30
	char yStandard;
	char x;
	char y;
	char __unknown34;
	char __unknown35;
	short __unknown36;
	short bannerWalkerId;
	char isLegion;
	char __padding;
	short attackType;
	short legionRoomForRecruits;
	short hasMilitaryTraining; //40
	short totalDamage; //42
	short maxTotalDamage; //44
	short __unknown46;
	short __unknown48;
	short __unknown4a;
	short __unknown4c;
	short __unknown4e;
	short __unknown50;
	short isHalted;
	short __unknown54;
	short __unknown56;
	short __unknown58;
	short __unknown5a;
	short cursedByMars;
	char __unknown5e;
	char empireService;
	char inDistantBattle; //60
	char isHerd;
	char enemyType;
	char __unknown63;
	char __unknown64;
	char __unknown65;
	char __unknown66;
	char orientation;
	char __unknown68;
	char __unknown69;
	char invasionId;
	char __unknown6b;
	char __unknown6c;
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
} Data_Formation_Extra;

#endif
