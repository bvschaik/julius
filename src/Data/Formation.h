#ifndef DATA_FORMATION_H
#define DATA_FORMATION_H

#define MAX_FORMATIONS 50

struct Data_Formation {
	char inUse;
	char ciid;
	char fortId;
	char __unknown03;
	short walkerType;
	short buildingId;
	short walkerIds[16];
	char numWalkers;
	char maxWalkers;
	short layout;
	short morale;
	char xFort;
	char yFort;
	char xStandard;
	char yStandard;
	char x;
	char y;
	char __unknown34;
	char __unknown35;
	short __unknown36;
	short bannerWalkerId;
	char isFort;
	char __padding;
	short attackType;
	short __unknown3e;
	short hasMilitaryTraining;
	short __unknown42;
	short __unknown44;
	short __unknown46;
	short __unknown48;
	short __unknown4a;
	short __unknown4c;
	short __unknown4e;
	short __unknown50;
	short __unknown52;
	short __unknown54;
	short __unknown56;
	short __unknown58;
	short __unknown5a;
	short cursedByMars;
	char __unknown5e;
	char empireService;
	char inDistantBattle;
	char __unknown61;
	char enemyType;
	char __unknown63;
	char __unknown64;
	short __unknown65;
	char orientation;
	char __unknown68;
	char __unknown69;
	char invasionId;
	char __unknown6b;
	char __unknown6c;
	char __padding6d[17];
	short invasionInternalId;
};

extern struct Data_Formation Data_Formations[MAX_FORMATIONS];

#endif
