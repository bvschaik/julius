#include "Random.h"
#include "Data/Random.h"

void Random_generateNext()
{
	Data_Random.pool[Data_Random.poolIndex++] = Data_Random.random1_7bit;
	if (Data_Random.poolIndex >= 100) {
		Data_Random.poolIndex = 0;
	}
	for (int i = 0; i < 31; i++) {
		unsigned int r1 = (((Data_Random.iv1 & 0x10) >> 4) ^ Data_Random.iv1) & 1;
		unsigned int r2 = (((Data_Random.iv2 & 0x10) >> 4) ^ Data_Random.iv2) & 1;
		Data_Random.iv1 = Data_Random.iv1 >> 1;
		Data_Random.iv2 = Data_Random.iv2 >> 1;
		if (r1) {
			Data_Random.iv1 |= 0x40000000;
		}
		if (r2) {
			Data_Random.iv2 |= 0x40000000;
		}
	}
	Data_Random.random1_7bit = Data_Random.iv1 & 0x7f;
	Data_Random.random1_15bit = Data_Random.iv1 & 0x7fff;
	Data_Random.random2_7bit = Data_Random.iv2 & 0x7f;
	Data_Random.random2_15bit = Data_Random.iv2 & 0x7fff;
}

void Random_generatePool()
{
	Data_Random.poolIndex = 0;
	for (int i = 0; i < 100; i++) {
		Random_generateNext();
	}
}
