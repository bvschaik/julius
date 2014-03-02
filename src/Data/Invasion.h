#ifndef DATA_INVASION_H
#define DATA_INVASION_H

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

extern struct Data_InvasionWarning Data_InvasionWarnings[101];

#endif
