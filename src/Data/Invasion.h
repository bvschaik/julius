#ifndef DATA_INVASION_H
#define DATA_INVASION_H

#define MAX_INVASION_WARNINGS 101

struct Data_InvasionWarning {
	char inUse;
	char handled;
	char empireInvasionPathId;
	char warningYears;
	short empireX;
	short empireY;
	short empireGraphicId;
	short empireObjectId;
	short gameMonthNotified;
	short gameYearNotified;
	int monthsToGo;
	char invasionId;
	char __filler[11];
};

extern struct Data_InvasionWarning Data_InvasionWarnings[MAX_INVASION_WARNINGS];

#endif
