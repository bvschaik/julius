#ifndef DATA_MESSAGE_H
#define DATA_MESSAGE_H

extern struct Data_Message {
	struct {
		int param1;
		short year;
		short param2;
		short messageTypeId;
		short id;
		char readFlag;
		char month;
		char __filler[2];
	} messages[1000];

	int nextMessageId;
	int totalMessages;
	int currentMessageId;
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
} Data_Message;

#endif
