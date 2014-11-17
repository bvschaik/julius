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
	int distanceX;
	int distanceY;
	if (x1 <= x2) {
		distanceX = x2 - x1;
	} else {
		distanceX = x1 - x2;
	}
	if (y1 <= y2) {
		distanceY = y2 - y1;
	} else {
		distanceY = y1 - y2;
	}
	return distanceX + distanceY;
}

int Calc_distanceMaximum(int x1, int y1, int x2, int y2)
{
	int distanceX;
	int distanceY;
	if (x1 <= x2) {
		distanceX = x2 - x1;
	} else {
		distanceX = x1 - x2;
	}
	if (y1 <= y2) {
		distanceY = y2 - y1;
	} else {
		distanceY = y1 - y2;
	}
	if (distanceX >= distanceY) {
		return distanceX;
	} else {
		return distanceY;
	}
}

int Calc_distanceMinimum(int x1, int y1, int x2, int y2)
{
	int distanceX;
	int distanceY;
	if (x1 <= x2) {
		distanceX = x2 - x1;
	} else {
		distanceX = x1 - x2;
	}
	if (y1 <= y2) {
		distanceY = y2 - y1;
	} else {
		distanceY = y1 - y2;
	}
	if (distanceX <= distanceY) {
		return distanceX;
	} else {
		return distanceY;
	}
}
