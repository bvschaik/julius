#ifndef PLAYERMESSAGE_H
#define PLAYERMESSAGE_H

enum {
	MessageAdvisor_None = 0,
	MessageAdvisor_Labor = 1,
	MessageAdvisor_Trade = 2,
	MessageAdvisor_Population = 3,
	MessageAdvisor_Imperial = 4,
	MessageAdvisor_Military = 5,
	MessageAdvisor_Health = 6,
	MessageAdvisor_Religion = 7,
};

enum {
	Message_12_FireInTheCity = 12,
	Message_80_RoadToRomeBlocked = 80,
	Message_116_RoadToRomeObstructed = 116,
	Message_117_NoWorkingDock = 117,
};

void PlayerMessage_disableSoundForNextMessage();
void PlayerMessage_post(int usePopup, int messageType, int param1, short param2);
void PlayerMessage_postWithPopupDelay(int type, int messageType, int param1, short param2);

void PlayerMessage_initList();
void PlayerMessage_initProblemArea();
void PlayerMessage_sortMessages();
void PlayerMessage_updateMessageDelays();
void PlayerMessage_processQueue();
void PlayerMessage_goToProblem();

int PlayerMessage_getAdvisorForMessageType(int messageType);
int PlayerMessage_getMessageTextId(int messageType);

#endif
