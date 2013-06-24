#include "Calc.h"

int Calc_adjustWithPercentage(int value, int percentage)
{
	return percentage * value / 100;
}

int Calc_getPercentage(int value, int total)
{
	int valueTimes100 = 100 * value;
	if (total) {
		return valueTimes100 / total;
	}
	return 0;
}

int Calc_distanceTotal(int x1, int y1, int x2, int y2)
{
	int distance_x;
	int distance_y;
	if (x1 <= x2) {
		distance_x = x2 - x1;
	} else {
		distance_x = x1 - x2;
	}
	if (y1 <= y2) {
		distance_y = y2 - y1;
	} else {
		distance_y = y1 - y2;
	}
	return distance_x + distance_y;
}

int Calc_distanceMaximum(int x1, int y1, int x2, int y2)
{
	int distance_x;
	int distance_y;
	if (x1 <= x2) {
		distance_x = x2 - x1;
	} else {
		distance_x = x1 - x2;
	}
	if (y1 <= y2) {
		distance_y = y2 - y1;
	} else {
		distance_y = y1 - y2;
	}
	if (distance_x >= distance_y) {
		return distance_x;
	} else {
		return distance_y;
	}
}

int Calc_distanceMinimum(int x1, int y1, int x2, int y2)
{
	int distance_x;
	int distance_y;
	if (x1 <= x2) {
		distance_x = x2 - x1;
	} else {
		distance_x = x1 - x2;
	}
	if (y1 <= y2) {
		distance_y = y2 - y1;
	} else {
		distance_y = y1 - y2;
	}
	if (distance_x <= distance_y) {
		return distance_x;
	} else {
		return distance_y;
	}
}
