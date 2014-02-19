#include "Time.h"

static TimeMillis currentMillis;

void Time_setMillis(TimeMillis millis)
{
	currentMillis = millis;
}

TimeMillis Time_getMillis()
{
	return currentMillis;
}

