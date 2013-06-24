#ifndef DATA_RANDOM_H
#define DATA_RANDOM_H

extern struct Data_Random {
	int iv1;
	int iv2;
	int random1_7bit;
	int random1_15bit;
	int random2_7bit;
	int random2_15bit;
	int pool[100];
	int poolIndex;
} Data_Random;

#endif
