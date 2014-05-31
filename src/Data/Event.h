#ifndef DATA_EVENT_H
#define DATA_EVENT_H

extern struct Data_Event {
	int lastInternalInvasionId;
	int timeLimitMaxGameYear;
	struct {
		int gameYear;
		int month;
		int state;
		int duration;
		int delay;
		int maxDuration;
		int maxDelay;
		struct {
			int x;
			int y;
		} expand[4];
	} earthquake;
	struct {
		int gameYear;
		int month;
		int state;
	} emperorChange;
	struct {
		int gameYear;
		int month;
		int endMonth;
		int state;
	} gladiatorRevolt;
} Data_Event;

#endif
