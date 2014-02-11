#ifndef CALC_H
#define CALC_H

/**
Adjusts the value with the percentage
@param value Value to adjust
@param percentage Percentage, example: 80
@return value * percentage/100
*/
int Calc_adjustWithPercentage(int value, int percentage);

/**
Calculates which percentage 'value' of 'total' is
@param value Value
@param total Total amount
@return 100 * value / total
*/
int Calc_getPercentage(int value, int total);

int Calc_distanceTotal(int x1, int y1, int x2, int y2);

int Calc_distanceMaximum(int x1, int y1, int x2, int y2);

int Calc_distanceMinimum(int x1, int y1, int x2, int y2);


#endif
