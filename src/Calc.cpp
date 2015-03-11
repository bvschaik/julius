#include "Calc.h"

int Calc_adjustWithPercentage(int value, int percentage)
{
	return percentage * value / 100;
}

int Calc_getPercentage(int value, int total)
{
	if (total) {
		int valueTimes100 = 100 * value;
		return valueTimes100 / total;
	} else {
		return 0;
	}
}

static int getDelta(int value1, int value2)
{
	if (value1 <= value2) {
		return value2 - value1;
	} else {
		return value1 - value2;
	}
}

int Calc_distanceTotal(int x1, int y1, int x2, int y2)
{
	int distanceX = getDelta(x1, x2);
	int distanceY = getDelta(y1, y2);
	return distanceX + distanceY;
}

int Calc_distanceMaximum(int x1, int y1, int x2, int y2)
{
	int distanceX = getDelta(x1, x2);
	int distanceY = getDelta(y1, y2);
	if (distanceX >= distanceY) {
		return distanceX;
	} else {
		return distanceY;
	}
}

int Calc_distanceMinimum(int x1, int y1, int x2, int y2)
{
	int distanceX = getDelta(x1, x2);
	int distanceY = getDelta(y1, y2);
	if (distanceX <= distanceY) {
		return distanceX;
	} else {
		return distanceY;
	}
}
