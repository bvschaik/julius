#ifndef DATA_MESSAGE_H
#define DATA_MESSAGE_H

#define MAX_MESSAGES 1000

#include "core/time.h"

struct Data_PlayerMessage {
	int param1;
	short year;
	short param2;
	short messageType;
	short sequence;
	unsigned char readFlag;
	unsigned char month;
	unsigned char __filler[2];
};

extern struct _Data_Message {
	struct Data_PlayerMessage messages[MAX_MESSAGES];

	int nextMessageSequence;
	int totalMessages;
	int currentMessageId;
	int currentProblemAreaMessageId;

	int popupMessageQueue[20];
	int consecutiveMessageDelay;

	// sound related
	int playSound;

	// UI related
	int scrollPosition;
	int maxScrollPosition;
	int isDraggingScrollbar;
	int scrollPositionDrag;
	int hotspotCount;
	int hotspotIndex;
	time_millis hotspotLastClick;
} Data_Message;

#endif
