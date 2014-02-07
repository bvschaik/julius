#include "Time.h"

static unsigned int currentMillis;

void Time_setMillis(unsigned int millis)
{
	currentMillis = millis;
}

unsigned int Time_getMillis()
{
	return currentMillis;
}

