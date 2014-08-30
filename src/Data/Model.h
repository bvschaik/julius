#ifndef DATA_MODEL_H
#define DATA_MODEL_H

extern struct Data_Model_House {
	int devolveDesirability;
	int evolveDesirability;
	int entertainment;
	int water;
	int religion;
	int education;
	int food;
	int barber;
	int bathhouse;
	int health;
	int foodTypes;
	int pottery;
	int oil;
	int furniture;
	int wine;
	int __unused1;
	int __unused2;
	int prosperity;
	int maxPeople;
	int taxMultiplier;
	int __unused3[5];
} Data_Model_Houses[20];

extern struct Data_Model_Building {
	int cost;
	int desirabilityValue;
	int desirabilityStep;
	int desirabilityStepSize;
	int desirabilityRange;
	int laborers;
	int __unused1;
	int __unused2;
} Data_Model_Buildings[130];

extern struct Data_Model_Difficulty {
	int moneyPercentage[5];
	int enemyPercentage[5];
	int startingFavor[5];
	int sentiment[5];
} Data_Model_Difficulty;

#endif
