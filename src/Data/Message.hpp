#ifndef DATA_MESSAGE_H
#define DATA_MESSAGE_H

#define MAX_MESSAGES 1000

#include "core/time.hpp"

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
	struct {
		unsigned char pop0;
		unsigned char pop500;
		unsigned char pop1000;
		unsigned char pop2000;
		unsigned char pop3000;
		unsigned char pop5000;
		unsigned char pop10000;
		unsigned char pop15000;
		unsigned char pop20000;
		unsigned char pop25000;
	} populationMessagesShown;

	int messageCategoryCount[20];
	int messageDelay[20];
	int popupMessageQueue[20];
	int consecutiveMessageDelay;

	// sound related
	int playSound;
	struct {
		time_millis fire;
		time_millis collapse;
		time_millis rioterGenerated;
		time_millis rioterCollapse;
	} lastSoundTime;

	// UI related
	int scrollPosition;
	int maxScrollPosition;
	int isDraggingScrollbar;
	int scrollPositionDrag;
	int hotspotCount;
	int hotspotIndex;
	time_millis hotspotLastClick;
} Data_Message;

enum {
	MessageDelay_Riot = 0,
	MessageDelay_Fire = 1,
	MessageDelay_Collapse = 2,
	MessageDelay_RiotCollapse = 3,
	MessageDelay_BlockedDock = 4,
	MessageDelay_WorkersNeeded = 8,
	MessageDelay_Tutorial3 = 9,
	MessageDelay_NoWorkingDock = 10,
	MessageDelay_FishingBlocked = 11,
};

#endif
