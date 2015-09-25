#include "String.h"

int String_toInt(const char *str)
{
	static const int multipliers[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000 };
	const char *ptr = str;
	int negative = 0;
	int numChars = 0;
	if (*ptr == '-') {
		negative = 1;
		ptr++;
	}
	while (*ptr >= '0' && *ptr <= '9') {
		numChars++;
		ptr++;
	}
	
	ptr = str;
	if (*ptr == '-') {
		ptr++;
	}
	int result = 0;
	while (numChars) {
		--numChars;
		result += multipliers[numChars] * (*ptr - '0');
		ptr++;
	}
	if (negative) {
		result = -result;
	}
	return result;
}

int String_fromInt(char *targetString, int value, int forcePlusSign)
{
	int totalChars = 0;
	if (value >= 0) {
		if (forcePlusSign) {
			targetString[0] = '+';
			targetString++;
			totalChars = 1;
		}
	} else {
		targetString[0] = '-';
		targetString++;
		value = -value;
		totalChars = 1;
	}
	int numDigits;
	if (value < 10) {
		numDigits = 1;
	} else if (value < 100) {
		numDigits = 2;
	} else if (value < 1000) {
		numDigits = 3;
	} else if (value < 10000) {
		numDigits = 4;
	} else if (value < 100000) {
		numDigits = 5;
	} else if (value < 1000000) {
		numDigits = 6;
	} else if (value < 10000000) {
		numDigits = 7;
	} else if (value < 100000000) {
		numDigits = 8;
	} else if (value < 1000000000) {
		numDigits = 9;
	} else {
		numDigits = 0;
	}

	totalChars += numDigits;

	while (--numDigits >= 0) {
		targetString[numDigits] = value % 10 + '0';
		value /= 10;
	}

	return totalChars;
}