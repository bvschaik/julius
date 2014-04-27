#ifndef DATA_EVENT_H
#define DATA_EVENT_H

extern struct Data_Event {
	int timeLimitMaxGameYear;
	struct {
		int gameYear;
		int month;
		int state;
		// etc
	} earthquake;
} Data_Event;

#endif
