#ifndef DATA_RANDOM_H
#define DATA_RANDOM_H

extern struct _Data_Random {
	unsigned int iv1;
	unsigned int iv2;
	int random1_7bit;
	int random1_15bit;
	int random2_7bit;
	int random2_15bit;
	int pool[100];
	int poolIndex;
} Data_Random;

#endif
