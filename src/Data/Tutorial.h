#ifndef DATA_TUTORIAL_H
#define DATA_TUTORIAL_H
#include "Data.h"

extern struct Data_Tutorial {
	struct {
		int fire;
		int crime;
		int collapse;
		int senateBuilt;
	} tutorial1;
	struct {
		int granaryBuilt;
		int population250Reached;
		int population450Reached;
		int potteryMade;
		int potteryMadeYear;
	} tutorial2;
	struct {
		int disease;
	} tutorial3;
} Data_Tutorial;

#endif
