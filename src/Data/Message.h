#ifndef DATA_MESSAGE_H
#define DATA_MESSAGE_H

#define MAX_MESSAGES 1000

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
		int fire;
		int collapse;
		int rioterGenerated;
		int rioterCollapse;
	} lastSoundTime;

	// UI related
	int scrollPosition;
	int maxScrollPosition;
	int isDraggingScrollbar;
	int scrollPositionDrag;
} Data_Message;

// TODO message delay ticks
enum {
	MessageCount_NoWorkingDock = 5,
	MessageDelayCategory_WorkersNeeded = 8
};

#endif
