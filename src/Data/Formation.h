#ifndef DATA_FORMATION_H
#define DATA_FORMATION_H

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

extern struct _Data_Formation_Extra {
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
