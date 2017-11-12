#ifndef DATA_MESSAGE_H
#define DATA_MESSAGE_H

#include "core/time.h"


extern struct _Data_Message {

	int currentProblemAreaMessageId;

	int popupMessageQueue[20];

	int hotspotCount;
	int hotspotIndex;
	time_millis hotspotLastClick;
} Data_Message;

#endif
