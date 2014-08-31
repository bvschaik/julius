#ifndef DATA_MESSAGE_H
#define DATA_MESSAGE_H

#define MAX_MESSAGES 1000

#include "Types.h"

struct Data_PlayerMessage {
	int param1;
	short year;
	short param2;
	short messageType;
	short sequence;
	char readFlag;
	char month;
	char __filler[2];
};

extern struct Data_Message {
	struct Data_PlayerMessage messages[MAX_MESSAGES];

	int nextMessageSequence;
	int totalMessages;
	int currentMessageId;
	int currentProblemAreaMessageId;
	struct {
		char pop0;
		char pop500;
		char pop1000;
		char pop2000;
		char pop3000;
		char pop5000;
		char pop10000;
		char pop15000;
		char pop20000;
		char pop25000;
	} populationMessagesShown;

	int messageCategoryCount[20];
	int messageDelay[20];
	int popupMessageQueue[20];
	int consecutiveMessageDelay;

	// sound related
	int playSound;
	struct {
		TimeMillis fire;
		TimeMillis collapse;
		TimeMillis rioterGenerated;
		TimeMillis rioterCollapse;
	} lastSoundTime;

	// UI related
	int scrollPosition;
	int maxScrollPosition;
	int isDraggingScrollbar;
	int scrollPositionDrag;
	int hotspotCount;
} Data_Message;

// TODO message delay ticks
enum {
	MessageDelay_Riot = 0,
	MessageDelay_Fire = 1,
	MessageDelay_Collapse = 2,
	MessageDelay_RiotCollapse = 3,
	MessageDelay_BlockedDock = 4,
	MessageDelay_WorkersNeeded = 8,
	MessageDelay_NoWorkingDock = 10,
	MessageDelay_FishingBlocked = 11,
};

#endif
