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
	Message_91_CeresIsUpset = 91,
	Message_92_NeptuneIsUpset = 92,
	Message_93_MercuryIsUpset = 93,
	Message_94_MarsIsUpset = 94,
	Message_95_VenusIsUpset = 95,
	Message_96_BlessingFromCeres = 96,
	Message_97_BlessingFromNeptune = 97,
	Message_98_BlessingFromMercury = 98,
	Message_99_BlessingFromMars = 99,
	Message_100_BlessingFromVenus = 100,
	Message_102_HealthIllness = 102,
	Message_103_HealthDisease = 103,
	Message_104_HealthPestilence = 104,
	Message_105_SpiritOfMars = 105,
	Message_109_WorkingHippodrome = 109,
	Message_110_WorkingColosseum = 110,
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
